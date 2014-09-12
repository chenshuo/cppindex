#include "indexer.h"

#include "clang/Basic/Version.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"

#include "muduo/base/FileUtil.h"

std::map<std::string, std::string> getBuiltinHeaders(const char* path)
{
  std::map<std::string, std::string> headers;
  // see Linux::AddClangSystemIncludeArgs() in clang/lib/Driver/ToolChains.cpp
  // SmallString<128> P("/usr/lib/clang");
  // llvm::sys::path::append(P, CLANG_VERSION_STRING, "include/");
  std::string inc = "/usr/lib/clang/";
  inc += CLANG_VERSION_STRING;
  inc += "/include/";

  llvm::error_code ec;
  llvm::sys::fs::directory_iterator it(path, ec);
  if (ec)
    return headers;
  for (llvm::sys::fs::directory_iterator end; it != end; it.increment(ec))
  {
    if (ec)
      break;
    if (llvm::sys::path::extension(it->path()) != ".h")
      continue;
    std::string header = (inc + llvm::sys::path::filename(it->path())).str();
    std::string content;
    if (muduo::FileUtil::readFile(it->path(), 10*1024*1024, &content) == 0)
    {
      // LOG_TRACE << "Add " << header << " " << content.size();
      headers[header] = content;
    }
  }
  return headers;
}

int main(int argc, char* argv[])
{
  std::vector<std::string> commands;
  for (int i = 0; i < argc; ++i)
    commands.push_back(argv[i]);
  llvm::IntrusiveRefCntPtr<clang::FileManager> files(
      new clang::FileManager(clang::FileSystemOptions()));
  clang::tooling::ToolInvocation tool(commands, new IndexAction(true), files.getPtr());
  auto headers = getBuiltinHeaders("/home/schen"
                                   "/llvm-3.4.2.src/build/lib/clang/3.4.2/include");
  for (const auto& it : headers)
  {
    tool.mapVirtualFile(it.first, it.second);
  }
  tool.run();
}
