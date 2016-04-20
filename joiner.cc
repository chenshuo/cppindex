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
  typedef std::map<string, string> Entries;

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
    proto::CompilationUnit cu;
    CHECK(cu.ParseFromString(getOrDie(entries, "main:")));
    string main = cu.main_file();
    assert(input.find(main) == input.end());
    input[main] = std::move(entries);
  }

  void resolve()
  {
    std::map<string, proto::Function> functions = getGlobalFunctions();
    LOG_INFO << "global functions " << functions.size();
    resolveFunctions(functions);
    merge();
  }

  std::map<string, proto::Function> getGlobalFunctions()
  {
    std::map<string, proto::Function> functions;
    for (const auto& file : input)
    {
      const Entries& entries = file.second;
      proto::CompilationUnit cu;
      CHECK(cu.ParseFromString(getOrDie(entries, "main:")));
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

  std::map<string, proto::Function> getStaticFunctions(const proto::CompilationUnit& cu,
                                                       const std::map<string, proto::Function>& globalFunctions)
  {
      std::map<string, proto::Function> staticFunctions;
      for (const proto::Function& func : cu.functions())
      {
        if (func.storage_class() == proto::kStatic)
        {
          assert(staticFunctions.find(func.name()) == staticFunctions.end());
          staticFunctions[func.name()] = func;
        }
        else
        {
          assert(staticFunctions.find(func.name()) == staticFunctions.end());
          assert(globalFunctions.find(func.name()) != globalFunctions.end());
        }
      }
      return staticFunctions;
  }

  void resolveFunctions(const std::map<string, proto::Function>& globalFunctions)
  {
    for (const auto& file : input)
    {
      const Entries& entries = file.second;
      proto::CompilationUnit cu;
      CHECK(cu.ParseFromString(getOrDie(entries, "main:")));
      const std::map<string, proto::Function> staticFunctions = getStaticFunctions(cu, globalFunctions);

      // for each "functions:" in this CU
      for (auto it = entries.lower_bound("functions:"); it != entries.end(); ++it)
      {
        if (!leveldb::Slice(it->first).starts_with("functions:"))
          break;

        proto::Functions functions;
        CHECK(functions.ParseFromString(it->second));
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
              const proto::Function& define = it->second;
              func.set_def_file(define.name_range().filename());
              func.set_def_lineno(define.name_range().begin().lineno());
            }
            else
            {
              assert(staticFunctions.find(func.name()) == staticFunctions.end());
              auto it = globalFunctions.find(func.name());
              if (it != globalFunctions.end())
              {
                const proto::Function& define = it->second;
                func.set_def_file(define.name_range().filename());
                func.set_def_lineno(define.name_range().begin().lineno());
              }
              else
              {
                LOG_WARN << "Undefined function " << func.name() << " used in " << file.first;
              }
            }
          }
        }
        // printf("resolved functions for %s\n%s\n", it->first.c_str(),
        //        functions.DebugString().c_str());
        // FIXME: save functions
      }
    }
  }

  void merge()
  {
  }

  static const string& getOrDie(const Entries& entries, const string& uri)
  {
    auto it = entries.find(uri);
    assert(it != entries.end());
    return it->second;
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
  std::map<string, Entries> input;
  static constexpr const char* kSrcmd5 = "srcmd5:";
};
}

int main(int argc, char* argv[])
{
  indexer::Joiner j(false);
  j.join(argv+1);
}
