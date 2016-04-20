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
  explicit Sink(const char* output)
    : out_(::fopen(output, "wb"))  // FIXME: CHECK_NOTNULL
  {
    printf("Sink %s\n", output);
  }
  ~Sink()
  {
    ::fclose(out_);
  }

  void writeOrDie(const string& key, const string& value)
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
      printf("write %s %d\n", key.c_str(), value_len);
    }
  }

 private:
  FILE* out_;
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


