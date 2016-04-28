#include "leveldb/db.h"
#include "llvm/Support/MD5.h"
#include "build/record.pb.h"

#include <boost/noncopyable.hpp>

#include "muduo/base/Logging.h"
#include "muduo/base/Timestamp.h"

//#include <stdio.h>
#include <iostream>
#include <memory>
#include <unordered_set>

namespace indexer
{
using std::string;
#include "sink.h"

class Joiner
{
 public:
  explicit Joiner(bool save)
  {
    if (save)
    {
      leveldb::DB* db;
      leveldb::Options options;
      options.create_if_missing = true;
      leveldb::Status status = leveldb::DB::Open(options, "testdb", &db);
      if (status.ok())
      {
        db_.reset(db);
      }
      else
      {
        LOG_ERROR << "Unable to open leveldb";
      }
    }
  }

  void join(char* argv[])
  {
    LOG_INFO << "reading";
    muduo::Timestamp start(muduo::Timestamp::now());
    for (char** file = argv; *file; ++file)
    {
      add(*file);
    }
    LOG_INFO << inputs_.size() << " inputs, "
             << timeDifference(muduo::Timestamp::now(), start) << " sec";

    resolve();
    merge();
    LOG_INFO << "done "
             << timeDifference(muduo::Timestamp::now(), start) << " sec";
  }

 private:
  // key is uri
  typedef std::map<string, string> Entries;
  // key is function name
  typedef std::map<string, proto::Function> FunctionMap;

  void add(const char* file)
  {
    Entries entries;

    Reader reader(file);
    string key, value;
    while (reader.read(&key, &value))
    {
      assert(entries.find(key) == entries.end());
      entries[key] = value;
    }
    std::cout << "add " << file
              << " " << entries.size() << " entries\n";
    proto::CompilationUnit cu = getCompilationUnit(entries);
    string main = cu.main_file();
    assert(inputs_.find(main) == inputs_.end());
    inputs_[main] = std::move(entries);
  }

  proto::CompilationUnit getCompilationUnit(const Entries& entries)
  {
    proto::CompilationUnit cu;
    auto it = entries.lower_bound("main:");
    assert(it != entries.end());
    assert(leveldb::Slice(it->first).starts_with("main:"));
    CHECK(cu.ParseFromString(it->second));
    return cu;
  }

  void resolve()
  {
    FunctionMap functions = getGlobalFunctions();
    resolveFunctions(functions);
    LOG_INFO << "undefined functions " << undefinedFunctions_.size();
    if (muduo::Logger::logLevel() <= muduo::Logger::DEBUG)
    {
      for (auto& func : undefinedFunctions_)
      {
        std::cout << "undefined " << func.first << " of " << func.second << "\n";
      }
    }
    for (auto& func : functions)
    {
      if (func.second.ref_file_size() == 1)
      {
        // FIXME: update link of define to the only usage.
        LOG_INFO << "global function used once " << func.second.DebugString();
      }
    }

    resolveStructs();
  }

  FunctionMap getGlobalFunctions()
  {
    LOG_INFO << "getGlobalFunctions";
    muduo::Timestamp start(muduo::Timestamp::now());
    FunctionMap functions;
    for (const auto& input : inputs_)
    {
      const Entries& entries = input.second;
      proto::CompilationUnit cu = getCompilationUnit(entries);
      for (const proto::Function& func : cu.functions())
      {
        if (func.storage_class() != proto::kStatic)
        {
          auto it = functions.find(func.name());
          if (it != functions.end())
          {
            std::cout << "duplicate global function: " << func.name() << "\n"
                     << "    THIS " << func.ShortDebugString() << "\n"
                     << "    PREV " << it->second.ShortDebugString() << "\n";
          }
          functions[func.name()] = func;
        }
      }
    }
    LOG_INFO << "global functions " << functions.size() << ", "
             << timeDifference(muduo::Timestamp::now(), start) << " sec";
    return functions;
  }

  FunctionMap getStaticFunctions(const proto::CompilationUnit& cu,
                                 const FunctionMap& globalFunctions)
  {
    FunctionMap staticFunctions;
    for (const proto::Function& func : cu.functions())
    {
      if (func.storage_class() == proto::kStatic)
      {
        assert(staticFunctions.find(func.name()) == staticFunctions.end());
        staticFunctions[func.name()] = func;
        allStaticFunctions_[func.name()]++;
        if (allStaticFunctions_[func.name()] > 1)
        {
          // if a header defines a static inline function, we will see a lot:
          // LOG_INFO << "duplicate static function: " << func.name();
        }
      }
      else
      {
        assert(staticFunctions.find(func.name()) == staticFunctions.end());
        assert(globalFunctions.find(func.name()) != globalFunctions.end());
      }
    }
    return staticFunctions;
  }

  void resolveFunctions(FunctionMap& globalFunctions)
  {
    muduo::Timestamp start(muduo::Timestamp::now());
    // FIXME: resolve function by sharing declare
    for (auto& input : inputs_)
    {
      Entries& entries = input.second;
      proto::CompilationUnit cu = getCompilationUnit(entries);
      assert(cu.main_file() == input.first);
      FunctionMap staticFunctions = getStaticFunctions(cu, globalFunctions);

      // for each "file:" in this CU
      for (auto file = entries.lower_bound("file:"); file != entries.end(); ++file)
      {
        if (!leveldb::Slice(file->first).starts_with("file:"))
          break;

        crossReferenceFunctions(globalFunctions, cu.main_file(), staticFunctions, file);
      }
      for (auto& func : staticFunctions)
      {
        // FIXME: update static function defines
        if (func.second.ref_file_size() == 1)
        {
          // FIXME: update link of define to the only usage.
          // LOG_INFO << "static function " << func.second.DebugString();
        }
      }
    }
    LOG_INFO << "resolveFunctions "
             << timeDifference(muduo::Timestamp::now(), start) << " sec";
  }

  void crossReferenceFunctions(FunctionMap& globalFunctions, const string cu,
                               FunctionMap& staticFunctions, Entries::iterator file)
  {
    proto::SourceFile sourceFile;
    CHECK(sourceFile.ParseFromString(file->second));
    assert(file->first.substr(strlen("file:")) == sourceFile.filename());
    // for each function in file
    for (proto::Function& func : *sourceFile.mutable_functions())
    {
      if (func.usage() == proto::kDefine)
      {
        if (func.storage_class() == proto::kStatic)
        {
          auto it = staticFunctions.find(func.name());
          assert(it != staticFunctions.end());
          // FIXME more asserts
        }
      }
      else
      {
        // use or declare
        if (func.storage_class() == proto::kStatic)
        {
          auto it = staticFunctions.find(func.name());
          if (it != staticFunctions.end())
          {
            proto::Function& define = it->second;
            foundDefine(&func, &define);
          }
          else
          {
            std::cout << "undefined static function " << func.ShortDebugString()
                      << " IN " << sourceFile.filename()
                      << " CU " << cu << "\n";
          }
          it = globalFunctions.find(func.name());
          if (it != globalFunctions.end())
          {
            std::cout << "global function hidden by static: "<< func.name()
                      << " IN " << sourceFile.filename()
                      << " CU " << cu << "\n"
                      << "    DEF " << it->second.ShortDebugString() << "\n"
                      << "    USE " << func.ShortDebugString() << "\n";
          }
        }
        else
        {
          // use of global function
          auto it = staticFunctions.find(func.name());
          if (it != staticFunctions.end())
          {
            // some functions are declared as extern but defined as static
            proto::Function& define = it->second;
            foundDefine(&func, &define);
            LOG_TRACE << func.name() << " was defined as static, but used as " << func.DebugString();
            assert(globalFunctions.find(func.name()) == globalFunctions.end());
          }
          else
          {
            it = globalFunctions.find(func.name());
            if (it != globalFunctions.end())
            {
              proto::Function& define = it->second;
              foundDefine(&func, &define);
            }
            else if (func.usage() == proto::kUse &&
                     !leveldb::Slice(func.name()).starts_with("__compiletime_assert_"))  // KERNEL HACK
            {
              LOG_TRACE << "Undefined function " << func.name() << " used in " << cu;
              undefinedFunctions_[func.name()] = func.signature();
            }
          }
        }
      }
    }
    // printf("resolved functions for %s\n%s\n", cu->first.c_str(),
    //        functions.DebugString().c_str());
    file->second = sourceFile.SerializeAsString();
  }

  void foundDefine(proto::Function* func, proto::Function* define)
  {
    assert(func->ref_file_size() == 0);
    assert(func->ref_lineno_size() == 0);
    func->add_ref_file(define->range().filename());
    func->add_ref_lineno(define->range().begin().lineno());
  }

  void resolveStructs()
  {
    // FIXME
  }

  void merge()
  {
    Entries sources;
    Entries files;
    Entries preprocess;
    Entries mains;
    // key is file name
    std::map<std::string, MD5String> md5s;
    LOG_INFO << "merging";
    muduo::Timestamp start(muduo::Timestamp::now());
    for (const auto& input : inputs_)
    {
      const Entries& entries = input.second;
      for (const auto& entry : entries)
      {
        leveldb::Slice key(entry.first);
        if (key.starts_with("src:"))
        {
          update(&sources, entry);
          key.remove_prefix(4); // "src:"
          md5s[key.ToString()] = md5String(entry.second);
        }
        else if (key.starts_with("file:"))
        {
          update(&files, entry);
        }
        else if (key.starts_with("prep:"))
        {
          update(&preprocess, entry);
        }
        else if (key.starts_with("main:"))
        {
          update(&mains, entry);
        }
        else if (key.starts_with("digests:"))
        {
          proto::Digests digests;
          CHECK(digests.ParseFromString(entry.second));
          // FIXME: verify digests
        }
        else
        {
          LOG_WARN << "Unknown uri " << entry.first;
        }
      }
    }
    LOG_INFO << "merge took  "
             << timeDifference(muduo::Timestamp::now(), start) << " sec";

    LOG_INFO << "writing";
    start = muduo::Timestamp::now();
    Sink sink(db_.get());
    // Sink sink("output");
    for (const auto& it : sources)
    {
      sink.writeOrDie(it.first, it.second);
    }
    for (const auto& it : files)
    {
      sink.writeOrDie(it.first, it.second);
    }
    for (const auto& it : preprocess)
    {
      sink.writeOrDie(it.first, it.second);
    }
    for (const auto& it : mains)
    {
      sink.writeOrDie(it.first, it.second);
    }
    LOG_INFO << "write took "
             << timeDifference(muduo::Timestamp::now(), start) << " sec";
  }

  void update(Entries* entries, const Entries::value_type& entry)
  {
    auto it = entries->find(entry.first);
    if (it == entries->end())
    {
      entries->insert(entry);
    }
    else
    {
      if (it->second != entry.second)
      {
        if (!changed_.count(entry.first))
        {
          std::cout << "changed " << entry.first << "\n";
          if (muduo::Logger::logLevel() <= muduo::Logger::DEBUG ||
              entry.first == printChanged_)
          {
            printf("OLD ========== {\n");
            print(entry.first, it->second);
            printf("NEW } ========== {\n");
            print(entry.first, entry.second);
            printf("END }\n");
          }
          changed_.insert(entry.first);
        }
      }
    }
  }

  void CHECK(bool ok)
  {
    if (!ok)
      abort();
  }

  std::unique_ptr<leveldb::DB> db_;
  // key is compilation unit name
  std::map<string, Entries> inputs_;
  // key is function name
  std::map<string, int> allStaticFunctions_;
  std::map<string, string> undefinedFunctions_;
  std::unordered_set<string> changed_;
  static constexpr const char* kSrcmd5 = "srcmd5:";
  static const string printChanged_;
};

const string Joiner::printChanged_ = ::getenv("PRINT_CHANGED") ?: "";
}

int main(int argc, char* argv[])
{
  indexer::Joiner j(true);
  j.join(argv+1);
}
