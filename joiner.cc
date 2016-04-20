#include "leveldb/db.h"
#include "llvm/Support/MD5.h"
#include "build/record.pb.h"
#include <memory>

#include <boost/noncopyable.hpp>

#include "muduo/base/Logging.h"

#include <stdio.h>

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
    for (char** file = argv; *file; ++file)
    {
      add(*file);
    }

    resolve();
  }

 private:
  // key is uri
  typedef std::map<string, string> Entries;
  // key is function name
  typedef std::map<string, proto::Function> FunctionMap;

  void add(const char* file)
  {
    printf("add %s ", file);
    Entries entries;

    Reader reader(file);
    string key, value;
    while (reader.read(&key, &value))
    {
      assert(entries.find(key) == entries.end());
      entries[key] = value;
    }
    printf("%zd\n", entries.size());
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
    LOG_INFO << "global functions " << functions.size();
    resolveFunctions(functions);
    LOG_INFO << "undefined functions " << undefinedFunctions_.size();
    for (auto& func : functions)
    {
      if (func.second.ref_file_size() == 1)
      {
        // FIXME: update link of define to the only usage.
        LOG_INFO << "global function " << func.second.DebugString();
      }
    }
    merge();
  }

  FunctionMap getGlobalFunctions()
  {
    FunctionMap functions;
    for (const auto& input : inputs_)
    {
      const Entries& entries = input.second;
      proto::CompilationUnit cu = getCompilationUnit(entries);
      for (const proto::Function& func : cu.functions())
      {
        if (func.storage_class() != proto::kStatic)
        {
          assert(functions.find(func.name()) == functions.end());
          functions[func.name()] = func;
        }
      }
    }
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
    for (auto& input : inputs_)
    {
      Entries& entries = input.second;
      proto::CompilationUnit cu = getCompilationUnit(entries);
      FunctionMap staticFunctions = getStaticFunctions(cu, globalFunctions);

      // for each "functions:" in this CU
      for (auto file = entries.lower_bound("functions:"); file != entries.end(); ++file)
      {
        if (!leveldb::Slice(file->first).starts_with("functions:"))
          break;

        proto::Functions functions;
        CHECK(functions.ParseFromString(file->second));
        // for each function in file
        for (proto::Function& func : *functions.mutable_functions())
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
              assert(globalFunctions.find(func.name()) == globalFunctions.end());
              auto it = staticFunctions.find(func.name());
              assert(it != staticFunctions.end());
              proto::Function& define = it->second;
              foundDefine(&func, &define);
            }
            else
            {
              // some functions are declared as extern but defined as static
              auto it = staticFunctions.find(func.name());
              if (it != staticFunctions.end())
              {
                proto::Function& define = it->second;
                foundDefine(&func, &define);
                LOG_TRACE << func.name() << " was defined as static, but used as " << func.DebugString();
              }
              else
              {
                it = globalFunctions.find(func.name());
                if (it != globalFunctions.end())
                {
                  proto::Function& define = it->second;
                  foundDefine(&func, &define);
                }
                else
                {
                  LOG_TRACE << "Undefined function " << func.name() << " used in " << input.first;
                  undefinedFunctions_[func.name()] = func.signature();
                }
              }
            }
          }
        }
        // printf("resolved functions for %s\n%s\n", cu->first.c_str(),
        //        functions.DebugString().c_str());
        file->second = functions.SerializeAsString();
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
  }

  void foundDefine(proto::Function* func, proto::Function* define)
  {
    assert(func->ref_file_size() == 0);
    assert(func->ref_lineno_size() == 0);
    func->add_ref_file(define->name_range().filename());
    func->add_ref_lineno(define->name_range().begin().lineno());
  }

  void merge()
  {
    Entries sources;
    Entries functions;
    Entries preprocess;
    Entries mains;
    // key is file name
    std::map<std::string, MD5String> md5s;
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
        else if (key.starts_with("functions:"))
        {
          update(&functions, entry);
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
    Sink sink(db_.get());
    for (const auto& it : sources)
    {
      sink.writeOrDie(it.first, it.second);
    }
    for (const auto& it : functions)
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
  }

  static void update(Entries* entries, const Entries::value_type& entry)
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
        if (entry.first.find("<built-in>") == string::npos)
          LOG_INFO << "changed " << entry.first;
      }
    }
  }

  void CHECK(bool ok)
  {
    if (!ok)
      abort();
  }

  void saveSourcesDb(const std::string& mainFile)
  {
    std::string content;

    // read md5 from db
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), kSrcmd5, &content);
    proto::Digests digests;
    std::map<std::string, MD5String> md5s;
    if (s.ok() && digests.ParseFromString(content))
    {
      for (auto it : digests.digests())
      {
        md5s[it.filename()] = it.md5();
      }
    }

    std::map<std::string, MD5String> mymd5s;
    // update/save source file when md5 is different
    std::map<std::string, std::string> files;
    for (const auto& src : files)
    {
      MD5String md5 = md5String(src.second);
      mymd5s[src.first] = md5;
      MD5String& origmd5 = md5s[src.first];
      if (origmd5 != md5)
      {
        std::string uri = "src:" + src.first;
        if (!origmd5.empty())
        {
          LOG_WARN << "Different content for " << uri;
        }
        else
        {
          LOG_INFO << "Add " << uri;
        }
        origmd5 = md5;
        s = db_->Put(leveldb::WriteOptions(), uri, src.second);
        assert(s.ok());
      }
    }

    // update digests
    digests.Clear();
    for (const auto& d : md5s)
    {
      auto* digest = digests.add_digests();
      digest->set_filename(d.first);
      llvm::StringRef str = d.second.str();
      digest->set_md5(str.data(), str.size());
    }
    if (!digests.SerializeToString(&content))
    {
      assert(false && "Digests::Serialize");
    }
    s = db_->Put(leveldb::WriteOptions(), kSrcmd5, content);
    assert(s.ok());

    std::string uri = "main:" + mainFile;

    // update main:
    digests.Clear();
    for (const auto& d : mymd5s)
    {
      auto* digest = digests.add_digests();
      digest->set_filename(d.first);
      llvm::StringRef str = d.second.str();
      digest->set_md5(str.data(), str.size());
    }
    if (!digests.SerializeToString(&content))
    {
      assert(false && "Digests::Serialize");
    }
    s = db_->Put(leveldb::WriteOptions(), uri, content);
    assert(s.ok());
  }

  std::unique_ptr<leveldb::DB> db_;
  // key is compilation unit name
  std::map<string, Entries> inputs_;
  // key is function name
  std::map<string, int> allStaticFunctions_;
  std::map<string, string> undefinedFunctions_;
  static constexpr const char* kSrcmd5 = "srcmd5:";
};
}

int main(int argc, char* argv[])
{
  indexer::Joiner j(true);
  j.join(argv+1);
}
