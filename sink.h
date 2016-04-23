typedef llvm::SmallString<32> MD5String;
inline MD5String md5String(const std::string& text)
{
  MD5String str;
  llvm::MD5 md5;
  md5.update(text);
  llvm::MD5::MD5Result result;
  md5.final(result);
  llvm::MD5::stringifyResult(result, str);
  return str;
}

class Sink : boost::noncopyable
{
 public:
  explicit Sink(leveldb::DB* db)
    : db_(db)
  {
    assert(db_);
  }

  explicit Sink(const char* output)
    : out_(::fopen(output, "wb"))  // FIXME: CHECK_NOTNULL
  {
    assert(out_);
    printf("Sink %s\n", output);
  }

  ~Sink()
  {
    if (out_)
      ::fclose(out_);
    printf("~Sink count %d max_key %s value_len %d\n", count_, max_key_.c_str(), max_value_);
  }

  int count() const { return count_; }

  void writeOrDie(const string& key, const string& value)
  {
    if (db_)
    {
      leveldb::Status s = db_->Put(leveldb::WriteOptions(), key, value);
      assert(s.ok());
      printf("write %s %zd\n", key.c_str(), value.size());
    }
    else
    {
      {
      int key_len = key.size();
      fwrite(&key_len, 1, sizeof key_len, out_);
      fwrite(key.data(), 1, key.size(), out_);
      }
      {
      int value_len = value.size();
      fwrite(&value_len, 1, sizeof value_len, out_);
      fwrite(value.data(), 1, value.size(), out_);
      }
    }
    ++count_;
    if (value.size() > max_value_)
    {
      max_key_ = key;
      max_value_ = value.size();
    }
  }

 private:
  leveldb::DB* db_ = nullptr;  // not owned
  FILE* out_ = nullptr;
  int count_ = 0;
  string max_key_;
  int max_value_ = 0;
};

class Reader : boost::noncopyable
{
 public:
  Reader(const char* file)
    : fp_(::fopen(file, "rb")),
      size_(size())
  {
    assert(fp_);
  }

  ~Reader()
  {
    ::fclose(fp_);
  }

  bool valid() const { return fp_; }

  bool read(string* key, string* value)
  {
    if (curr() < size_)
    {
      int key_len = readInt32();
      *key = readBytes(key_len);
      int value_len = readInt32();
      *value = readBytes(value_len);
      return true;
    }
    else
      return false;
  }

private:
  string readBytes(int n)
  {
    char buf[n];
    ssize_t nr = ::fread(buf, 1, n, fp_);
    assert(nr == n);
    return string(buf, n); 
  }

  int32_t readInt32()
  {
    int32_t x = 0;
    ssize_t nr = ::fread(&x, 1, sizeof(x), fp_);
    assert (nr == sizeof(x));
    return x;
  }

  long curr()
  {
    return ::ftell(fp_);
  }

  long size()
  {
    ::fseek(fp_, 0, SEEK_END);
    long s = curr();
    ::fseek(fp_, 0, SEEK_SET);
    return s;
  }
 private:
  FILE* fp_;
  long size_;
};

template<typename MSG>
inline void parseAndPrint(const string& content)
{
  MSG msg;
  if (msg.ParseFromString(content))
  {
    printf("%s", msg.DebugString().c_str());
  }
}

inline void print(leveldb::Slice key, const string& content)
{
  if (key.starts_with("src:"))
  {
    printf("%s", content.c_str());
  }
  else if (key.starts_with("main:") || key == "inc:")
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

