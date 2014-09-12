#include "leveldb/db.h"
#include <memory>

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
      // TODO: show value of keys
    }
  }
  else
  {
    printf("%s \n", status.ToString().c_str());
  }
}
