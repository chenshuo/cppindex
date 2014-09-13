class IndexPP : public clang::PPCallbacks
{
 public:
  IndexPP(clang::CompilerInstance& compiler, leveldb::DB* db)
    : preprocessor_(compiler.getPreprocessor()),
      sourceManager_(compiler.getSourceManager()),
      db_(db)
  {
    // printf("predefines:\n%s\n", preprocessor_.getPredefines().c_str());
    LOG_DEBUG;
  }

  ~IndexPP()
  {
    LOG_DEBUG;
  }

  void FileChanged(clang::SourceLocation location,
                   PPCallbacks::FileChangeReason reason,
                   clang::SrcMgr::CharacteristicKind fileType,
                   clang::FileID previousFileID) override
  {
    LOG_TRACE << reasonString(reason) << " " << filePath(location);

    const std::string file_changed = filePath(location);
    if (reason == clang::PPCallbacks::EnterFile)
    {
      std::string content;
      if (getFileContent(location, &content))
      {
        auto it = files_.find(file_changed);
        if (it == files_.end())
        {
          files_[file_changed] = content;
        }
        else if (it->second != content)
        {
          LOG_WARN << "File content changed " << file_changed;
        }
      }
      else
      {
        LOG_ERROR << "unable to get file content for " << file_changed;
      }
    }
  }

  virtual void FileSkipped(const clang::FileEntry &ParentFile,
                           const clang::Token &FilenameTok,
                           clang::SrcMgr::CharacteristicKind FileType) {
    // printf("FileSkipped %s\n", preprocessor_.getSpelling(FilenameTok).c_str());
  }

  virtual void InclusionDirective(clang::SourceLocation hashLoc,
                                  const clang::Token &includeTok,
                                  clang::StringRef filename,
                                  bool isAngled,
                                  clang::CharSourceRange filenameRange,
                                  const clang::FileEntry *file,
                                  clang::StringRef searchPath,
                                  clang::StringRef relativePath,
                                  const clang::Module *imported)
  {
    if (file == nullptr)
      return;

    std::string currentFile = filePath(hashLoc);
    // FIXME: skip if currentFile has been indexed.

    bool invalid = true;
    unsigned lineno = sourceManager_.getSpellingLineNumber(hashLoc, &invalid);
    if (invalid)
      return;

    std::string includedFile = file ? file->getName() : filename.str();
    // LOG_DEBUG << currentFile << ":" << lineno << " -> " << includedFile;
    Preprocess& pp = records_[currentFile];

    auto* inc = pp.add_includes();
    inc->set_included_file(file->getName());
    inc->set_lineno(lineno);

    if (filenameRange.getBegin().isMacroID())
    {
      // LOG_WARN << currentFile << ":" << lineno << "#include " << filename.str() << " was a macro";
      inc->set_macro(true);
        const clang::SourceRange range(includeTok.getLocation(),
                                       includeTok.getLocation().getLocWithOffset(includeTok.getLength()));
        sourceRangeToRange(range, inc->mutable_range());
    }
    else
    {
      // skip '"' or '<'
      clang::SourceLocation filenameStart = filenameRange.getBegin().getLocWithOffset(1);
      clang::SourceLocation filenameEnd = filenameRange.getEnd().getLocWithOffset(-1);
      int filenameLength = 0;
      invalid = true;
      const char* filenameStr = sourceManager_.getCharacterData(filenameStart, &invalid);
      if (!invalid
          && sourceManager_.isInSameSLocAddrSpace(filenameStart, filenameEnd, &filenameLength)
          && static_cast<size_t>(filenameLength) == filename.size()
          && filename == clang::StringRef(filenameStr, filenameLength))
      {
        // LOG_INFO << "good";
        const clang::SourceRange range(filenameStart, filenameEnd);
        sourceRangeToRange(range, inc->mutable_range());
      }
      else
      {
        LOG_WARN << filename.str() << " != " << clang::StringRef(filenameStr, filenameLength).str();
      }
    }
  }

  virtual void EndOfMainFile()
  {
    LOG_DEBUG << filePath(sourceManager_.getMainFileID());
    if (db_ == nullptr)
      return;

    saveSources();

    // TODO: dedup
    // FIXME: save only when different
    std::string content;
    for (const auto& it : records_)
    {
      std::string uri = "prep:" + it.first;
      // LOG_INFO << it.first << ":\n" << it.second.DebugString();
      if (it.second.SerializeToString(&content))
      {
        leveldb::Status s = db_->Put(leveldb::WriteOptions(), uri, content);
        assert(s.ok());
      }
      else
      {
        assert(false && "Preprocess::Serialize");
      }
    }
    db_ = nullptr;
  }

  /// \brief Called by Preprocessor::HandleMacroExpandedIdentifier when a
  /// macro invocation is found.
  virtual void MacroExpands(const clang::Token &MacroNameTok, const clang::MacroDirective *MD,
                            clang::SourceRange Range, const clang::MacroArgs *Args) {
    // printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
  }

  /// \brief Hook called whenever a macro definition is seen.
  virtual void MacroDefined(const clang::Token &MacroNameTok,
                            const clang::MacroDirective *MD) {
    // printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
    auto start = MacroNameTok.getLocation();
    clang::SourceRange range(start, start.getLocWithOffset(MacroNameTok.getLength()));
    if (range.getBegin().isMacroID())
    {
      printf("don't know\n");
    }
    else
    {
      //const auto first = sourceManager_.getDecomposedSpellingLoc(start);
      //const auto last = sourceManager_.getDecomposedSpellingLoc(range.getEnd());
      //printf("%u %u\n", first, last);
    }
  }

  /// \brief Hook called whenever a macro \#undef is seen.
  ///
  /// MD is released immediately following this callback.
  virtual void MacroUndefined(const clang::Token &MacroNameTok,
                              const clang::MacroDirective *MD) {
    // printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
  }

  /// \brief Hook called whenever the 'defined' operator is seen.
  /// \param MD The MacroDirective if the name was a macro, null otherwise.
  virtual void Defined(const clang::Token &MacroNameTok, const clang::MacroDirective *MD,
                       clang::SourceRange Range) {
    // printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());

  }

  // Called for #ifdef, checks for use of an existing macro definition.
  // Implements PPCallbacks::Ifdef.
  void Ifdef(clang::SourceLocation location,
             const clang::Token& MacroNameTok,
             const clang::MacroDirective* macro) override {
    // printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
  }

  // Called for #ifndef, checks for use of an existing macro definition.
  // Implements PPCallbacks::Ifndef.
  void Ifndef(clang::SourceLocation location,
              const clang::Token& MacroNameTok,
              const clang::MacroDirective* macro) override {
    // printf("%s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
  }


  /// \brief Hook called when a source range is skipped.
  /// \param Range The SourceRange that was skipped. The range begins at the
  /// \#if/\#else directive and ends after the \#endif/\#else directive.
  virtual void SourceRangeSkipped(clang::SourceRange Range) {
    // printf("%s\n", __FUNCTION__);
  }

  std::string filePath(clang::FileID fileId) const
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

  void sourceRangeToRange(const clang::SourceRange& sourceRange, Range* range) const
  {
    sourceLocationToLocation(sourceRange.getBegin(), range->mutable_begin());
    sourceLocationToLocation(sourceRange.getEnd(), range->mutable_end());
  }

  void sourceLocationToLocation(clang::SourceLocation sloc, Location* loc) const
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

  typedef llvm::SmallString<32> MD5String;
  static MD5String md5String(const std::string& text)
  {
    MD5String str;
    llvm::MD5 md5;
    md5.update(text);
    llvm::MD5::MD5Result result;
    md5.final(result);
    llvm::MD5::stringifyResult(result, str);
    return str;
  }

 private:
  bool getFileContent(clang::SourceLocation location, std::string* content)
  {
    assert(location.isFileID());
    clang::FileID fileId = sourceManager_.getFileID(location);
    if (const clang::FileEntry* fileEntry = sourceManager_.getFileEntryForID(fileId))
    {
      bool isInvalid = false;
      const llvm::MemoryBuffer* buffer = sourceManager_.getMemoryBufferForFile(fileEntry, &isInvalid);
      if (buffer && ! isInvalid)
      {
        *content = buffer->getBuffer().str();
        return true;
      }
    }
    else if (fileId == preprocessor_.getPredefinesFileID())
    {
      *content = preprocessor_.getPredefines();
      return true;
    }
    return false;
  }

  void saveSources()
  {
    std::string content;

    // read md5 from db
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), kSrcmd5, &content);
    Digests digests;
    std::map<std::string, MD5String> md5s;
    if (s.ok() && digests.ParseFromString(content))
    {
      for (auto it : digests.digests())
      {
        md5s[it.filename()] = it.md5();
      }
    }

    // update/save source file when md5 is different
    for (const auto& src : files_)
    {
      MD5String md5 = md5String(src.second);
      MD5String& origmd5 = md5s[src.first];
      if (origmd5 != md5)
      {
        std::string uri = "src:" + src.first;
        if (!origmd5.empty())
        {
          LOG_WARN << "Different content for " << uri;
        }
        else
        {
          LOG_INFO << "Add " << uri;
        }
        origmd5 = md5;
        s = db_->Put(leveldb::WriteOptions(), uri, src.second);
        assert(s.ok());
      }
    }

    // update digests
    digests.Clear();
    for (const auto& d : md5s)
    {
      auto* digest = digests.add_digests();
      digest->set_filename(d.first);
      llvm::StringRef str = d.second.str();
      digest->set_md5(str.data(), str.size());
    }
    if (!digests.SerializeToString(&content))
    {
      assert(false && "Digests::Serialize");
    }
    s = db_->Put(leveldb::WriteOptions(), kSrcmd5, content);
    assert(s.ok());
  }

  const clang::Preprocessor& preprocessor_;
  clang::SourceManager& sourceManager_;
  std::map<std::string, std::string> files_;
  std::map<std::string, Preprocess> records_;
  leveldb::DB* db_;

  static constexpr const char* kSrcmd5 = "srcmd5:";
};

