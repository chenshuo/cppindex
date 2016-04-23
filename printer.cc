#include "build/record.pb.h"

#include "llvm/Support/MD5.h"
#include "clang/Rewrite/Core/Rewriter.h"

#include "leveldb/db.h"

#include "muduo/base/Logging.h"

#include <unordered_set>

#include <stdio.h>

namespace indexer
{

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

    srcuri.remove_prefix(4); // "src:"
    std::string filename = srcuri.ToString();
    formatPreprocess(filename, &rb);
    formatFunctions(filename, &rb);

    int numlines = escapeHtml(text, &rb);
    std::vector<std::string> headers;
    headers.push_back("<html><head><title>");
    headers.push_back(filename);
    headers.push_back(R"(</title><link rel="stylesheet" type="text/css" href="../source.css">)");
    headers.push_back("</head>\n<body><table style=\"border-spacing:10px 0px\"><tr><td>\n");
    for (int line = 1; line <= numlines; ++line)
    {
      char buf[256];
      snprintf(buf, sizeof buf,
               R"(<span class="lineno"><a href="#L%d" name="L%d" tabindex="-1">%d</a></span>)",
               line, line, line);
      headers.push_back(buf);
    }
    headers.push_back("</td>\n<td><pre>");
    for (size_t i = 0; i < headers.size(); ++i)
      rb.InsertTextBefore(0, headers[headers.size()-1-i]);
    rb.InsertTextAfter(text.size(), "</pre></td></tr></table></body></html>\n");
    *html = std::string(rb.begin(), rb.end());
    return getHtmlFilename(filename);;
  }

 private:
  void formatPreprocess(const std::string& filename, clang::RewriteBuffer* rb)
  {
    std::string content;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), "prep:" + filename, &content);
    if (!s.ok())
      return;
    proto::Preprocess pp;
    if (!pp.ParseFromString(content))
      assert(0);
    assert(filename == pp.filename());
    for (const auto& inc : pp.includes())
    {
      if (!inc.changed())
      {
        rb->InsertTextBefore(inc.range().begin().offset(), makeHref(inc.included_file()));
        rb->InsertTextAfter(inc.range().end().offset(), "</a>");
      }
    }
    for (const auto& macro : pp.macros())
    {
      if (macro.define())
      {
        rb->InsertTextBefore(macro.range().begin().offset(), R"(<span class="macro-def">)");
        rb->InsertTextAfter(macro.range().end().offset(), "</span>");
      }
      else if (macro.reference() && macro.ref_lineno() > 0)
      {
        rb->InsertTextBefore(macro.range().begin().offset(),
                             makeHref(macro.ref_file(), macro.ref_lineno()));
        rb->InsertTextAfter(macro.range().end().offset(), "</a>");
      }
      else
      {
        rb->InsertTextBefore(macro.range().begin().offset(), R"(<span class="macro-use">)");
        rb->InsertTextAfter(macro.range().end().offset(), "</span>");
      }
    }
  }

  void formatFunctions(const std::string& filename, clang::RewriteBuffer* rb)
  {
    std::string content;
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), "functions:" + filename, &content);
    if (!s.ok())
      return;
    proto::Functions functions;
    if (!functions.ParseFromString(content))
      assert(0);
    assert(filename == functions.filename());
    for (const auto& func : functions.functions())
    {
      if (func.name_range().anchor())
        continue;
      if (func.ref_file_size() == 1 && func.ref_lineno_size() == 1)
      {
        rb->InsertTextBefore(func.name_range().begin().offset(),
                             makeHref(func.ref_file().Get(0), func.ref_lineno().Get(0)));
        rb->InsertTextAfter(func.name_range().end().offset(), "</a>");
      }
      else if (func.usage() == proto::kDefine)
      {
        rb->InsertTextBefore(func.name_range().begin().offset(), R"(<span class="func-def">)");
        rb->InsertTextAfter(func.name_range().end().offset(), "</span>");
      }
    }

  }

  static std::string makeHref(const std::string& filename, int lineno = 0)
  {
    std::string result = "<a href=\"" + getHtmlFilename(filename);
    if (lineno > 0)
    {
      char buf[32];
      snprintf(buf, sizeof buf, "#L%d", lineno);
      result += buf;
    }
    return result + "\">";
  }

  static std::string getHtmlFilename(const std::string& srcfile)
  {
    std::string result = srcfile;
    if (!result.empty())
    {
      for (auto& ch : result)
      {
        if (ch == '/' || ch == '.' || ch == '<' || ch == '>' || ch == '+')
          ch = '_';
      }
      result += ".html";
    }
    else
    {
      result = "#";
    }
    return result;
  }

  static int escapeHtml(leveldb::Slice text, clang::RewriteBuffer* rb)
  {
    int lines = 0;
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

        case '\n':
          ++lines;
          break;
      }
    }
    // FIXME: last line without EOL
    return lines;
  }
};

}  // namespace indexer

std::string escapeHtml(const std::string& text)
{
  std::string html;
  for (char ch : text)
  {
    switch (ch)
    {
      case '<': html += "&lt;"; break;
      case '>': html += "&gt;"; break;
      case '&': html += "&amp;"; break;
      default: html += ch;
    }
  }
  return html;
}

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
    indexer::Formatter fmt(db);

    if (argc > 1)
    {
      std::string html;
      std::string file = fmt.format(argv[1], &html);
      printf("%s\n%s\n", file.c_str(), html.c_str());
    }
    else
    {
      std::unique_ptr<leveldb::Iterator> it(db->NewIterator(leveldb::ReadOptions()));
      std::string html;
      std::unordered_set<std::string> files;
      std::string index_page = "<html><body><ul>";
      for (it->Seek("src:");
           it->Valid() && it->key().ToString() < "src:\xff";  // FIXME signed char?
           it->Next())
      {
        leveldb::Slice srcuri = it->key();

        std::string file = fmt.format(srcuri, it->value(), &html);
        if (!files.insert(file).second)
          LOG_WARN << "Rewrite " << file << " for " << srcuri.ToString();
        LOG_DEBUG << file;
        if (!file.empty())
        {
          save(file, html);
        }
        srcuri.remove_prefix(4); // "src:"
        index_page += R"(<li><a href=")" + file + R"(">)" + escapeHtml(srcuri.ToString()) + "</a></li>\n";
      }
      index_page += "</ul></body></html>";
      save("index.html", index_page);
    }
  }
  google::protobuf::ShutdownProtobufLibrary();
}
