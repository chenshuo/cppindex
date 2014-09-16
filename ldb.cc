#include "leveldb/db.h"
#include "record.pb.h"
#include <memory>

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
  if (key.starts_with("srcmd5:") || key.starts_with("main:"))
  {
    parseAndPrint<indexer::proto::Digests>(content);
  }
  else if (key.starts_with("src:"))
  {
    printf("%s", content.c_str());
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

int main(int argc, char* argv[])
{
  leveldb::DB* db;
  leveldb::Options options;
  leveldb::Status status = leveldb::DB::Open(options, "testdb", &db);
  if (status.ok())
  {
    std::unique_ptr<leveldb::DB> own(db);
    std::unique_ptr<leveldb::Iterator> it(db->NewIterator(leveldb::ReadOptions()));
    if (argc == 1)
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
      const char* key = argv[1];
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
    printf("%s \n", status.ToString().c_str());
  }
}
