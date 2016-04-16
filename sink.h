class Sink : boost::noncopyable
{
 public:
  Sink()
    : out_(::fopen("sink", "wb"))
  {
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
    }
  }
 private:
  FILE* out_;
};
