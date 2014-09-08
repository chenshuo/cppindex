#include "indexer.h"
#include "clang/Tooling/Tooling.h"

int main(int argc, char* argv[])
{
  std::vector<std::string> commands;
  for (int i = 0; i < argc; ++i)
    commands.push_back(argv[i]);
  llvm::IntrusiveRefCntPtr<clang::FileManager> files(
      new clang::FileManager(clang::FileSystemOptions()));
  clang::tooling::ToolInvocation tool(commands, new IndexAction, files.getPtr());
  tool.run();
}
