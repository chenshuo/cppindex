
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

  void setNameRange(const string& name, clang::SourceLocation start, proto::Range* range) const
  {
    assert(start.isValid());
    if (start.isFileID())
    {
      string spelling = getSpelling(start);
      assert(spelling == name);

      clang::FileID fileId = sourceManager_.getFileID(start);
      if (const clang::FileEntry* fileEntry = sourceManager_.getFileEntryForID(fileId))
        range->set_filename(fileEntry->getName());
      else
        assert(0 && "Cannot get file entry.");

      clang::SourceLocation end = clang::Lexer::getLocForEndOfToken(
          start, 0, sourceManager_, langOpts_);
      sourceLocationToLocation(start, range->mutable_begin());
      sourceLocationToLocation(end, range->mutable_end());
      // assert [start, end] == name FIXME
    }
    else
    {
      clang::SourceLocation fileLoc = sourceManager_.getFileLoc(start);
      string spelling = getSpelling(fileLoc);
      if (spelling == name)
      {
        setNameRange(name, fileLoc, range);
      }
      else
      {
        // FIXME
        LOG_WARN << "Spelling " << spelling << " != " << name;
        fflush(stdout);
        start.dump(sourceManager_);
        llvm::errs() << " fileLoc ";
        fileLoc.dump(sourceManager_);
        llvm::errs() << "\n";
      }
    }
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

