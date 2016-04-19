
struct Util
{
  clang::SourceManager& sourceManager_;
  const clang::LangOptions& langOpts_;

  Util(clang::SourceManager& sourceManager, const clang::LangOptions& langOpts)
    : sourceManager_(sourceManager), langOpts_(langOpts)
  {
  }
  
  void sourceLocationToLocation(clang::SourceLocation sloc, proto::Location* loc) const
  {
    assert(sloc.isFileID());
    auto decomposed = sourceManager_.getDecomposedLoc(sloc);
    loc->set_offset(decomposed.second);
    bool invalid = true;
    loc->set_lineno(sourceManager_.getLineNumber(decomposed.first, decomposed.second, &invalid));
    assert(!invalid && "getLineNumber");
    loc->set_column(sourceManager_.getColumnNumber(decomposed.first, decomposed.second, &invalid));
    assert(!invalid && "getColumnNumber");
  }

  string getSpelling(clang::SourceLocation start) const
  {
      llvm::SmallVector<char, 32> buffer;
      llvm::StringRef tokenSpelling = clang::Lexer::getSpelling(start, buffer, sourceManager_, langOpts_);
      return tokenSpelling.str();
  }

  static proto::StorageClass toProto(clang::StorageClass sc)
  {
    switch (sc)
    {
      case clang::SC_None:
        return proto::kNone;
      case clang::SC_Extern:
        return proto::kExtern;
      case clang::SC_Static:
        return proto::kStatic;
      default:
        assert(0 && "Unknown StorageClass");
    }
  }
      
  template<typename T>
  static void setStorageClass(clang::StorageClass sc, T* msg)
  {
    if (sc != clang::SC_None)
      msg->set_storage_class(Util::toProto(sc));
  }
};

