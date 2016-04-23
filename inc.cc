#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Tooling/Tooling.h"

#include "muduo/base/Logging.h"
#include <iostream>
#include <iomanip>
#include <set>

namespace indexer
{
using namespace clang;
using std::string;

/// \brief This interface provides a way to observe the actions of the
/// preprocessor as it does its thing.
///
/// Clients can define their hooks here to implement preprocessor level tools.
class IncludePPCallbacks : public clang::PPCallbacks
{
 public:
  IncludePPCallbacks(clang::CompilerInstance& compiler)
    : compiler_(compiler),
      preprocessor_(compiler.getPreprocessor()),
      sourceManager_(compiler.getSourceManager())
  {
    std::cout << "IncludePPCallbacks " << filePath(sourceManager_.getMainFileID()) << "\n";
  }

  ~IncludePPCallbacks() override
  {
    std::cout << "~IncludePPCallbacks\n\n";
  }

  string filePath(clang::FileID fileId) const
  {
    if (fileId.isInvalid())
      return "<invalid location>";

    if (const clang::FileEntry* fileEntry = sourceManager_.getFileEntryForID(fileId))
      return fileEntry->getName();

    return kBuiltInFileName;
  }

  std::string filePath(clang::SourceLocation location) const
  {
    if (location.isInvalid())
      return "<invalid location>";

    if (location.isFileID())
      return filePath(sourceManager_.getFileID(location));

    return "<unknown>";
  }

  static constexpr const char* kBuiltInFileName = "<built-in>";

  static const char* reasonString(clang::PPCallbacks::FileChangeReason reason)
  {
    switch (reason)
    {
      case clang::PPCallbacks::EnterFile:
        return "Enter";
      case clang::PPCallbacks::ExitFile:
        return "Exit";
      case clang::PPCallbacks::SystemHeaderPragma:
        return "SystemHeaderPragma";
      case clang::PPCallbacks::RenameFile:
        return "Rename";
      default:
        return "Unknown";
    }
  }

  /*
  enum FileChangeReason {
    EnterFile, ExitFile, SystemHeaderPragma, RenameFile
  };
  */

  /// \brief Callback invoked whenever a source file is entered or exited.
  ///
  /// \param Loc Indicates the new location.
  /// \param PrevFID the file that was exited if \p Reason is ExitFile.
  void FileChanged(SourceLocation loc, FileChangeReason reason,
                   SrcMgr::CharacteristicKind fileType,
                   FileID prevFID) override {
    const std::string file_changed = filePath(loc);
    bool reenter = false;
    if (reason == EnterFile)
    {
      ++indent_;
      if (!seen_files_.insert(file_changed).second)
      {
        reenter = true;
        reenter_files_.insert(file_changed);
      }
    }
    else if (reason == ExitFile)
      --indent_;
    std::cout << "FileChanged: " << std::setw(3) << indent_ << string(2*indent_, ' ')
              << reasonString(reason) << " " << file_changed
              << " FROM " << filePath(prevFID)
              << (reenter ? " REENTER" : "")
              << "\n";
  }

  /// \brief Callback invoked whenever a source file is skipped as the result
  /// of header guard optimization.
  ///
  /// \param ParentFile The file that \#included the skipped file.
  ///
  /// \param FilenameTok The token in ParentFile that indicates the
  /// skipped file.
  void FileSkipped(const FileEntry &parentFile,
                   const Token &filenameTok,
                   SrcMgr::CharacteristicKind FileType) override {
    std::cout << "FileSkipped:    " << string(2*indent_, ' ') << "Skip "
              << preprocessor_.getSpelling(filenameTok)
              << " OF " << parentFile.getName()
              << "\n";
  }

  /// \brief Callback invoked whenever an inclusion directive results in a
  /// file-not-found error.
  ///
  /// \param FileName The name of the file being included, as written in the
  /// source code.
  ///
  /// \param RecoveryPath If this client indicates that it can recover from
  /// this missing file, the client should set this as an additional header
  /// search patch.
  ///
  /// \returns true to indicate that the preprocessor should attempt to recover
  /// by adding \p RecoveryPath as a header search path.
  bool FileNotFound(StringRef FileName,
                    SmallVectorImpl<char> &RecoveryPath) override {
    return false;
  }

  /// \brief Callback invoked whenever an inclusion directive of
  /// any kind (\c \#include, \c \#import, etc.) has been processed, regardless
  /// of whether the inclusion will actually result in an inclusion.
  ///
  /// \param HashLoc The location of the '#' that starts the inclusion
  /// directive.
  ///
  /// \param IncludeTok The token that indicates the kind of inclusion
  /// directive, e.g., 'include' or 'import'.
  ///
  /// \param FileName The name of the file being included, as written in the
  /// source code.
  ///
  /// \param IsAngled Whether the file name was enclosed in angle brackets;
  /// otherwise, it was enclosed in quotes.
  ///
  /// \param FilenameRange The character range of the quotes or angle brackets
  /// for the written file name.
  ///
  /// \param File The actual file that may be included by this inclusion
  /// directive.
  ///
  /// \param SearchPath Contains the search path which was used to find the file
  /// in the file system. If the file was found via an absolute include path,
  /// SearchPath will be empty. For framework includes, the SearchPath and
  /// RelativePath will be split up. For example, if an include of "Some/Some.h"
  /// is found via the framework path
  /// "path/to/Frameworks/Some.framework/Headers/Some.h", SearchPath will be
  /// "path/to/Frameworks/Some.framework/Headers" and RelativePath will be
  /// "Some.h".
  ///
  /// \param RelativePath The path relative to SearchPath, at which the include
  /// file was found. This is equal to FileName except for framework includes.
  ///
  /// \param Imported The module, whenever an inclusion directive was
  /// automatically turned into a module import or null otherwise.
  ///
  void InclusionDirective(SourceLocation HashLoc,
                          const Token &IncludeTok,
                          StringRef FileName,
                          bool IsAngled,
                          CharSourceRange FilenameRange,
                          const FileEntry *File,
                          StringRef SearchPath,
                          StringRef RelativePath,
                          const Module *Imported) override {
  }

  /// \brief Callback invoked when the end of the main file is reached.
  ///
  /// No subsequent callbacks will be made.
  void EndOfMainFile() override {
    std::cout << "EndOfMainFile: ";
    clang::FileID fileId = sourceManager_.getMainFileID();
    if (const clang::FileEntry* fileEntry = sourceManager_.getFileEntryForID(fileId))
    {
      std::cout << fileEntry->getName();
    }
    std::cout << "\n\n";
    std::cout << "Seen:\n";
    for (const string& file : seen_files_)
    {
      std::cout << file << "\n";
    }
    std::cout << "\nReenter:\n";
    for (const string& file : reenter_files_)
    {
      std::cout << file << "\n";
    }
    std::cout << "\n";
  }

#if 0
{
  /// \brief Callback invoked whenever there was an explicit module-import
  /// syntax.
  ///
  /// \param ImportLoc The location of import directive token.
  ///
  /// \param Path The identifiers (and their locations) of the module
  /// "path", e.g., "std.vector" would be split into "std" and "vector".
  ///
  /// \param Imported The imported module; can be null if importing failed.
  ///
  virtual void moduleImport(SourceLocation ImportLoc,
                            ModuleIdPath Path,
                            const Module *Imported) {
  }

  /// \brief Callback invoked when a \#ident or \#sccs directive is read.
  /// \param Loc The location of the directive.
  /// \param str The text of the directive.
  ///
  virtual void Ident(SourceLocation Loc, const std::string &str) {
  }

  /// \brief Callback invoked when start reading any pragma directive.
  virtual void PragmaDirective(SourceLocation Loc,
                               PragmaIntroducerKind Introducer) {
  }

  /// \brief Callback invoked when a \#pragma comment directive is read.
  virtual void PragmaComment(SourceLocation Loc, const IdentifierInfo *Kind,
                             const std::string &Str) {
  }

  /// \brief Callback invoked when a \#pragma detect_mismatch directive is
  /// read.
  virtual void PragmaDetectMismatch(SourceLocation Loc,
                                    const std::string &Name,
                                    const std::string &Value) {
  }

  /// \brief Callback invoked when a \#pragma clang __debug directive is read.
  /// \param Loc The location of the debug directive.
  /// \param DebugType The identifier following __debug.
  virtual void PragmaDebug(SourceLocation Loc, StringRef DebugType) {
  }

  /// \brief Determines the kind of \#pragma invoking a call to PragmaMessage.
  enum PragmaMessageKind {
    /// \brief \#pragma message has been invoked.
    PMK_Message,

    /// \brief \#pragma GCC warning has been invoked.
    PMK_Warning,

    /// \brief \#pragma GCC error has been invoked.
    PMK_Error
  };

  /// \brief Callback invoked when a \#pragma message directive is read.
  /// \param Loc The location of the message directive.
  /// \param Namespace The namespace of the message directive.
  /// \param Kind The type of the message directive.
  /// \param Str The text of the message directive.
  virtual void PragmaMessage(SourceLocation Loc, StringRef Namespace,
                             PragmaMessageKind Kind, StringRef Str) {
  }

  /// \brief Callback invoked when a \#pragma gcc dianostic push directive
  /// is read.
  virtual void PragmaDiagnosticPush(SourceLocation Loc,
                                    StringRef Namespace) {
  }

  /// \brief Callback invoked when a \#pragma gcc dianostic pop directive
  /// is read.
  virtual void PragmaDiagnosticPop(SourceLocation Loc,
                                   StringRef Namespace) {
  }

  /// \brief Callback invoked when a \#pragma gcc dianostic directive is read.
  virtual void PragmaDiagnostic(SourceLocation Loc, StringRef Namespace,
                                diag::Severity mapping, StringRef Str) {}

  /// \brief Called when an OpenCL extension is either disabled or
  /// enabled with a pragma.
  virtual void PragmaOpenCLExtension(SourceLocation NameLoc,
                                     const IdentifierInfo *Name,
                                     SourceLocation StateLoc, unsigned State) {
  }

  /// \brief Callback invoked when a \#pragma warning directive is read.
  virtual void PragmaWarning(SourceLocation Loc, StringRef WarningSpec,
                             ArrayRef<int> Ids) {
  }

  /// \brief Callback invoked when a \#pragma warning(push) directive is read.
  virtual void PragmaWarningPush(SourceLocation Loc, int Level) {
  }

  /// \brief Callback invoked when a \#pragma warning(pop) directive is read.
  virtual void PragmaWarningPop(SourceLocation Loc) {
  }
}
#endif

  /// \brief Called by Preprocessor::HandleMacroExpandedIdentifier when a
  /// macro invocation is found.
  virtual void MacroExpands(const Token &MacroNameTok, const MacroDirective *MD,
                            SourceRange Range, const MacroArgs *Args) {
  }

  /// \brief Hook called whenever a macro definition is seen.
  virtual void MacroDefined(const Token &MacroNameTok,
                            const MacroDirective *MD) {
  }

  /// \brief Hook called whenever a macro \#undef is seen.
  ///
  /// MD is released immediately following this callback.
  virtual void MacroUndefined(const Token &MacroNameTok,
                              const MacroDirective *MD) {
  }

  /// \brief Hook called whenever the 'defined' operator is seen.
  /// \param MD The MacroDirective if the name was a macro, null otherwise.
  virtual void Defined(const Token &MacroNameTok, const MacroDirective *MD,
                       SourceRange Range) {
  }

  /// \brief Hook called when a source range is skipped.
  /// \param Range The SourceRange that was skipped. The range begins at the
  /// \#if/\#else directive and ends after the \#endif/\#else directive.
  virtual void SourceRangeSkipped(SourceRange Range) {
  }

  /*
  enum ConditionValueKind {
    CVK_NotEvaluated, CVK_False, CVK_True
  };
  */

  /// \brief Hook called whenever an \#if is seen.
  /// \param Loc the source location of the directive.
  /// \param ConditionRange The SourceRange of the expression being tested.
  /// \param ConditionValue The evaluated value of the condition.
  ///
  // FIXME: better to pass in a list (or tree!) of Tokens.
  virtual void If(SourceLocation Loc, SourceRange ConditionRange,
                  ConditionValueKind ConditionValue) {
  }

  /// \brief Hook called whenever an \#elif is seen.
  /// \param Loc the source location of the directive.
  /// \param ConditionRange The SourceRange of the expression being tested.
  /// \param ConditionValue The evaluated value of the condition.
  /// \param IfLoc the source location of the \#if/\#ifdef/\#ifndef directive.
  // FIXME: better to pass in a list (or tree!) of Tokens.
  virtual void Elif(SourceLocation Loc, SourceRange ConditionRange,
                    ConditionValueKind ConditionValue, SourceLocation IfLoc) {
  }

  /// \brief Hook called whenever an \#ifdef is seen.
  /// \param Loc the source location of the directive.
  /// \param MacroNameTok Information on the token being tested.
  /// \param MD The MacroDirective if the name was a macro, null otherwise.
  virtual void Ifdef(SourceLocation Loc, const Token &MacroNameTok,
                     const MacroDirective *MD) {
  }

  /// \brief Hook called whenever an \#ifndef is seen.
  /// \param Loc the source location of the directive.
  /// \param MacroNameTok Information on the token being tested.
  /// \param MD The MacroDirective if the name was a macro, null otherwise.
  virtual void Ifndef(SourceLocation Loc, const Token &MacroNameTok,
                      const MacroDirective *MD) {
  }

  /// \brief Hook called whenever an \#else is seen.
  /// \param Loc the source location of the directive.
  /// \param IfLoc the source location of the \#if/\#ifdef/\#ifndef directive.
  virtual void Else(SourceLocation Loc, SourceLocation IfLoc) {
  }

  /// \brief Hook called whenever an \#endif is seen.
  /// \param Loc the source location of the directive.
  /// \param IfLoc the source location of the \#if/\#ifdef/\#ifndef directive.
  virtual void Endif(SourceLocation Loc, SourceLocation IfLoc) {
  }

  clang::CompilerInstance& compiler_;
  const clang::Preprocessor& preprocessor_;
  clang::SourceManager& sourceManager_;
  int indent_ = 0;
  std::set<string> seen_files_;
  std::set<string> reenter_files_;
};
}  // namespace indexer

class PreprocessAction : public clang::PreprocessOnlyAction
{
 protected:
  void ExecuteAction() override
  {
    auto& compiler = getCompilerInstance();
    auto* pp = new indexer::IncludePPCallbacks(compiler);
    compiler.getPreprocessor().addPPCallbacks(pp);
    clang::PreprocessOnlyAction::ExecuteAction();
  }
};

int main(int argc, char* argv[])
{
  std::vector<std::string> commands;
  for (int i = 0; i < argc; ++i)
    commands.push_back(argv[i]);
  commands.push_back("-fno-spell-checking");
  llvm::IntrusiveRefCntPtr<clang::FileManager> files(
      new clang::FileManager(clang::FileSystemOptions()));
  clang::tooling::ToolInvocation tool(commands, new PreprocessAction, files.get());

  bool succeed = tool.run();
  return succeed ? 0 : -1;
}

