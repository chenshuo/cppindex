#include "leveldb/db.h"
#include "llvm/Support/MD5.h"
#include "build/record.pb.h"
#include <memory>

#include <boost/noncopyable.hpp>

#include <stdio.h>

using std::string;
#include "sink.h"

template<typename MSG>
void parseAndPrint(const std::string& content)
{
  MSG msg;
  if (msg.ParseFromString(content))
  {
    printf("%s", msg.DebugString().c_str());
  }
}

void print(leveldb::Slice key, const std::string& content)
{
  if (key.starts_with("src:"))
  {
    printf("%s", content.c_str());
  }
  else if (key.starts_with("main:"))
  {
    parseAndPrint<indexer::proto::CompilationUnit>(content);
  }
  else if (key.starts_with("digests:"))
  {
    parseAndPrint<indexer::proto::Digests>(content);
  }
  else if (key.starts_with("functions:"))
  {
    parseAndPrint<indexer::proto::Functions>(content);
  }
  else if (key.starts_with("prep:"))
  {
    parseAndPrint<indexer::proto::Preprocess>(content);
  }
  else
  {
    printf("don't know how to print %s\n", key.data());
  }
}

void dumpdb(const char* key)
{
  leveldb::DB* db;
  leveldb::Options options;
  leveldb::Status status = leveldb::DB::Open(options, "testdb", &db);
  if (status.ok())
  {
    std::unique_ptr<leveldb::DB> own(db);
    std::unique_ptr<leveldb::Iterator> it(db->NewIterator(leveldb::ReadOptions()));
    if (key == NULL)
    {
      size_t total = 0;
      for (it->SeekToFirst(); it->Valid(); it->Next())
      {
        printf("%s %zu\n", it->key().ToString().c_str(), it->value().size());
        total += it->value().size();
      }
      assert(it->status().ok());
      printf("total: %zu\n", total);
    }
    else
    {
      std::string content;
      leveldb::Status s = db->Get(leveldb::ReadOptions(), key, &content);
      if (s.ok())
      {
        print(key, content);
      }
      // TODO: scan prefix
    }
  }
  else
  {
    printf("%s\n", status.ToString().c_str());
  }
}

int main(int argc, char* argv[])
{
  if (argc == 1)
  {
    dumpdb(NULL);
  }
  else
  {
    if (strchr(argv[1], ':'))
    {
      dumpdb(argv[1]);
    }
    else
    {
      Reader reader(argv[1]);
      string key, value;
      while (reader.read(&key, &value))
      {
        printf("KEY %s %zd\n", key.c_str(), value.size());
        print(key, value); printf("\n");
      }
    }
  }
}
