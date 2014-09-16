#include "record.pb.h"

#include "llvm/Support/MD5.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "leveldb/db.h"

#include "muduo/base/Logging.h"

#include <unordered_set>

#include <stdio.h>

class Formatter
{
  std::unique_ptr<leveldb::DB> db_;
 public:

  explicit Formatter(leveldb::DB* db)
    : db_(db)
  {
  }

  std::string format(const std::string& srcuri, std::string* html)
  {
    std::string text;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), srcuri, &text);
    if (s.ok())
    {
      return format(srcuri, text, html);
    }
    return "";
  }

  std::string format(leveldb::Slice srcuri, leveldb::Slice text, std::string* html)
  {
    clang::RewriteBuffer rb;
    rb.Initialize(text.data(), text.data() + text.size());

    srcuri.remove_prefix(4);
    std::string filename = srcuri.ToString();
    formatPreprocess(filename, &rb);

    escapeHtml(text, &rb);
    rb.InsertTextBefore(0, "<html><head><title>" + filename + "</title></head><body><pre>");
    rb.InsertTextAfter(text.size(), "</pre></body></html>");
    *html = rb.ToString();
    return getHtmlFilename(filename);;
  }

 private:
  void formatPreprocess(const std::string& filename, clang::RewriteBuffer* rb)
  {
    std::string content;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), "prep:" + filename, &content);
    if (!s.ok())
      return;
    indexer::proto::Preprocess pp;
    if (!pp.ParseFromString(content))
      return;
    assert(filename == pp.filename());
    for (const auto& inc : pp.includes())
    {
      if (!inc.changed())
      {
        rb->InsertTextBefore(inc.range().begin().offset(), makeHref(inc.included_file()));
        rb->InsertTextAfter(inc.range().end().offset(), "</a>");
      }
    }
  }

  std::string makeHref(const std::string& filename)
  {
    std::string result = "<a href=\"" + getHtmlFilename(filename);
    return result + "\">";
  }

  std::string getHtmlFilename(const std::string& srcfile)
  {
    std::string result = srcfile;
    for (auto& ch : result)
    {
      if (ch == '/' || ch == '.')
        ch = '_';
    }
    result += ".html";
    return result;
  }

  static void escapeHtml(leveldb::Slice text, clang::RewriteBuffer* rb)
  {
    for (size_t i = 0; i < text.size(); ++i)
    {
      switch (text[i])
      {
        case '<':
          rb->ReplaceText(i, 1, "&lt;");
          break;

        case '>':
          rb->ReplaceText(i, 1, "&gt;");
          break;

        case '&':
          rb->ReplaceText(i, 1, "&amp;");
          break;
      }
    }
  }
};

void save(const std::string& file, const std::string& content)
{
  FILE* fp = fopen(("html/" + file).c_str(), "w");
  assert(fp && "Failed to open output file");
  if (fp)
  {
    size_t nw = fwrite(content.data(), 1, content.size(), fp);
    assert(nw == content.size());
    fclose(fp);
  }
}

int main(int argc, char* argv[])
{
  leveldb::DB* db;
  leveldb::Options options;
  leveldb::Status status = leveldb::DB::Open(options, "testdb", &db);
  if (status.ok())
  {
    Formatter fmt(db);

    if (argc > 1)
    {
      std::string html;
      std::string file = fmt.format(argv[1], &html);
      printf("%s\n", file.c_str());
      if (!file.empty())
        save(file, html);
    }
    else
    {
      std::unique_ptr<leveldb::Iterator> it(db->NewIterator(leveldb::ReadOptions()));
      std::string html;
      std::unordered_set<std::string> files;
      for (it->Seek("src:");
           it->Valid() && it->key().ToString() < "src:\xff";
           it->Next())
      {
        std::string file = fmt.format(it->key(), it->value(), &html);
        if (!files.insert(file).second)
          LOG_WARN << "Rewrite " << file << " for " << it->key().ToString();
        LOG_DEBUG << file;
        if (!file.empty())
          save(file, html);
      }
    }
  }
  google::protobuf::ShutdownProtobufLibrary();
}
