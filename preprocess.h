class IndexPP : public clang::PPCallbacks
{
 public:
  IndexPP(clang::CompilerInstance& compiler, Sink* sink)
    : compiler_(compiler),
      preprocessor_(compiler.getPreprocessor()),
      sourceManager_(compiler.getSourceManager()),
      sink_(sink)
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
          // a new file
          files_[file_changed] = content;
          clang::FileID fileId = sourceManager_.getFileID(location);
          findComments(fileId);
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

  void FileSkipped(const clang::FileEntry &ParentFile,
                   const clang::Token &FilenameTok,
                   clang::SrcMgr::CharacteristicKind FileType) override {
    // printf("FileSkipped %s\n", preprocessor_.getSpelling(FilenameTok).c_str());
  }

  void InclusionDirective(clang::SourceLocation hashLoc,
                          const clang::Token &includeTok,
                          clang::StringRef filename,
                          bool isAngled,
                          clang::CharSourceRange filenameRange,
                          const clang::FileEntry *file,
                          clang::StringRef searchPath,
                          clang::StringRef relativePath,
                          const clang::Module *imported) override
  {
    if (file == nullptr)
      return;

    std::string currentFile = filePath(hashLoc);

    bool invalid = true;
    unsigned lineno = sourceManager_.getSpellingLineNumber(hashLoc, &invalid);
    if (invalid)
      return;

    std::string includedFile = file ? file->getName() : filename.str();
    LOG_TRACE << currentFile << ":" << lineno << " -> " << includedFile;
    Inclusions& inc = inclusions_[currentFile];

    proto::Range range;
    bool isMacro = filenameRange.getBegin().isMacroID();
    if (isMacro)
    {
      LOG_WARN << currentFile << ":" << lineno << "#include " << filename.str() << " was a macro";
      auto start = includeTok.getLocation();
      auto end = start.getLocWithOffset(includeTok.getLength());
      const clang::SourceRange srcRange(start, end);
      sourceRangeToRange(srcRange, &range);
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
        const clang::SourceRange srcRange(filenameStart, filenameEnd);
        sourceRangeToRange(srcRange, &range);
      }
      else
      {
        LOG_WARN << filename.str() << " != " << clang::StringRef(filenameStr, filenameLength).str();
      }
    }

    if (!inc.add(lineno, includedFile, isMacro, &range))
    {
      LOG_WARN << "#include changed at " << currentFile << ":" << lineno << " -> " << includedFile;
    }
  }

  void EndOfMainFile() override
  {
    std::string mainFile = filePath(sourceManager_.getMainFileID());
    LOG_INFO << __FUNCTION__ << " " << mainFile;
    if (sink_ == nullptr)
      return;

    saveSources(mainFile);

    std::string content;
    proto::Preprocess pp;
    for (const auto& it : files_)
    {
      const std::string& filename = it.first;
      std::string uri = "prep:" + filename;
      // leveldb::Status s = db_->Get(leveldb::ReadOptions(), uri, &content);

      // LOG_INFO << it.first << ":\n" << pp.DebugString();
      pp.Clear();
      pp.set_filename(filename);
      bool hasContent = false;

      auto inc = inclusions_.find(filename);
      if (inc != inclusions_.end())
      {
        hasContent = true;
        inc->second.fill(&pp);
      }

      auto macro = macros_.find(filename);
      if (macro != macros_.end())
      {
        hasContent = true;
        macro->second.fill(&pp);
      }

      if (!hasContent)
      {
        continue;
      }
      if (pp.SerializeToString(&content))
      {
        sink_->writeOrDie(uri, content);
      }
      else
      {
        assert(false && "Preprocess::Serialize");
      }
    }
    sink_ = nullptr;
  }

  /// \brief Hook called whenever a macro definition is seen.
  void MacroDefined(const clang::Token &MacroNameTok,
                    const clang::MacroDirective *MD) override
  {
    auto start = MacroNameTok.getLocation();
    if (start.isMacroID())
    {
      printf("don't know %s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
      abort();
      return;
    }

    string filename = filePath(start);
    clang::SourceRange range(start, start.getLocWithOffset(MacroNameTok.getLength()));
    macros_[filename].addMacro(this, MacroNameTok, filename, range);
  }

  /// \brief Called by Preprocessor::HandleMacroExpandedIdentifier when a
  /// macro invocation is found.
  void MacroExpands(const clang::Token& MacroNameTok,
                    const clang::MacroDirective *MD,
                    clang::SourceRange,
                    const clang::MacroArgs *Args) override
  {
    macroUsed(MacroNameTok, MD);
  }

  /// \brief Hook called whenever a macro \#undef is seen.
  ///
  /// MD is released immediately following this callback.
  void MacroUndefined(const clang::Token &MacroNameTok,
                      const clang::MacroDirective *MD) override {
    macroUsed(MacroNameTok, MD);
  }

  /// \brief Hook called whenever the 'defined' operator is seen.
  /// \param MD The MacroDirective if the name was a macro, null otherwise.
  void Defined(const clang::Token &MacroNameTok,
               const clang::MacroDirective *MD,
               clang::SourceRange Range) override
  {
    macroUsed(MacroNameTok, MD);
  }

  // Called for #ifdef, checks for use of an existing macro definition.
  // Implements PPCallbacks::Ifdef.
  void Ifdef(clang::SourceLocation location,
             const clang::Token& MacroNameTok,
             const clang::MacroDirective* MD) override
  {
    macroUsed(MacroNameTok, MD);
  }

  // Called for #ifndef, checks for use of an existing macro definition.
  // Implements PPCallbacks::Ifndef.
  void Ifndef(clang::SourceLocation location,
              const clang::Token& MacroNameTok,
              const clang::MacroDirective* MD) override
  {
    macroUsed(MacroNameTok, MD);
  }


  /// \brief Hook called when a source range is skipped.
  /// \param Range The SourceRange that was skipped. The range begins at the
  /// \#if/\#else directive and ends after the \#endif/\#else directive.
  void SourceRangeSkipped(clang::SourceRange Range) override {
    // printf("%s\n", __FUNCTION__);
    // FIXME
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

  void sourceRangeToRange(const clang::SourceRange& sourceRange, proto::Range* range) const
  {
    sourceLocationToLocation(sourceRange.getBegin(), range->mutable_begin());
    sourceLocationToLocation(sourceRange.getEnd(), range->mutable_end());
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

  void macroUsed(const clang::Token &MacroNameTok,
                 const clang::MacroDirective *MD)
  {
    auto start = MacroNameTok.getLocation();
    if (start.isMacroID())
    {
      // printf("don't know %s %s\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str());
      return;
    }

    std::string filename = filePath(start);
    clang::SourceRange range(start, start.getLocWithOffset(MacroNameTok.getLength()));
    // printf("%s %s %p\n", __FUNCTION__, MacroNameTok.getIdentifierInfo()->getName().str().c_str(), MD);
    macros_[filename].addReference(this, MacroNameTok, filename, range, MD);
  }

  void findComments(clang::FileID fid)
  {
    const llvm::MemoryBuffer *FromFile = sourceManager_.getBuffer(fid);
    clang::Lexer L(fid, FromFile, sourceManager_, compiler_.getLangOpts());
    L.SetCommentRetentionState(true);
    clang::Token rawTok;
    do {
      L.LexFromRawLexer(rawTok);
      //unsigned TokOffs = sourceManager_.getFileOffset(rawTok.getLocation());
      //unsigned TokLen = rawTok.getLength();
      // comments
      //if (rawTok.is(clang::tok::comment))
      //  printf("off=%d len=%d kind=%s\n",
      //         TokOffs, TokLen, clang::tok::getTokenName(rawTok.getKind()));
      // string literals
      // preprocess directives
    } while (rawTok.isNot(clang::tok::eof));
  }

  void saveSources(const std::string& mainFile) const
  {
    std::map<std::string, MD5String> md5s;
    for (const auto& src : files_)
    {
      MD5String md5 = md5String(src.second);
      md5s[src.first] = md5;
      std::string uri = "src:" + src.first;
      // LOG_INFO << "Add " << uri;
      sink_->writeOrDie(uri, src.second);
    }

    std::string uri = "main:" + mainFile;

    // update main:
    proto::Digests digests;
    for (const auto& d : md5s)
    {
      auto* digest = digests.add_digests();
      digest->set_filename(d.first);
      llvm::StringRef str = d.second.str();
      digest->set_md5(str.data(), str.size());
    }
    std::string content;
    if (!digests.SerializeToString(&content))
    {
      assert(false && "Digests::Serialize");
    }
    sink_->writeOrDie(uri, content);
  }

  void saveSourcesDb(const std::string& mainFile)
  {
    std::string content;
    leveldb::DB* db_ = nullptr;

    // read md5 from db
    leveldb::Status s = db_->Get(leveldb::ReadOptions(), kSrcmd5, &content);
    proto::Digests digests;
    std::map<std::string, MD5String> md5s;
    if (s.ok() && digests.ParseFromString(content))
    {
      for (auto it : digests.digests())
      {
        md5s[it.filename()] = it.md5();
      }
    }

    std::map<std::string, MD5String> mymd5s;
    // update/save source file when md5 is different
    for (const auto& src : files_)
    {
      MD5String md5 = md5String(src.second);
      mymd5s[src.first] = md5;
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

    std::string uri = "main:" + mainFile;

    // update main:
    digests.Clear();
    for (const auto& d : mymd5s)
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
    s = db_->Put(leveldb::WriteOptions(), uri, content);
    assert(s.ok());
  }

  // Record include's for a source file
  struct Inclusions
  {
    // return true if added a new item or same item.
    bool add(unsigned lineno, std::string& includedFile, bool isMacro, proto::Range* range)
    {
      std::unique_ptr<proto::Inclusion>& inc = includes_[lineno];
      if (inc)
      {
        if (inc->included_file() != includedFile)
        {
          // TODO: check other
          // FIXME: update ?
          inc->set_changed(true);
          return false;
        }
      }
      else
      {
        inc.reset(new proto::Inclusion);
        inc->set_included_file(includedFile);
        inc->set_lineno(lineno);
        if (isMacro)
          inc->set_macro(isMacro);
        inc->mutable_range()->Swap(range);
      }
      return true;
    }

    void fill(proto::Preprocess* pp)
    {
      for (auto& it : includes_)
      {
        pp->mutable_includes()->AddAllocated(it.second.release());
      }
    }

    // lineno to included_file
    std::map<unsigned, std::unique_ptr<proto::Inclusion>> includes_;
  };

  struct Macros
  {
    proto::Macro* addMacro(IndexPP* pp,
                           const clang::Token& macroNameTok,
                           const std::string& filename,
                           const clang::SourceRange& srcRange,
                           bool define = true)
    {
      unsigned offset = pp->sourceManager_.getDecomposedSpellingLoc(srcRange.getBegin()).second;
      auto& macro = macros_[offset];
      if (macro)
      {
        // FIXME: check identical
      }
      else
      {
        macro.reset(new proto::Macro);
        macro->set_name(macroNameTok.getIdentifierInfo()->getName());
        if (define)
          macro->set_define(define);
        pp->sourceRangeToRange(srcRange, macro->mutable_range());
      }
      return macro.get();
    }

    void addReference(IndexPP* pp,
                      const clang::Token& macroNameTok,
                      const std::string& filename,
                      const clang::SourceRange& srcRange,
                      const clang::MacroDirective *MD)
    {
      proto::Macro* macro = addMacro(pp, macroNameTok, filename, srcRange, false);
      macro->set_reference(true);
      if (MD)
      {
        auto loc = MD->getLocation();
        if (!loc.isInvalid() && loc.isFileID())
        {
          macro->set_ref_file(pp->filePath(loc));
          macro->set_ref_lineno(pp->sourceManager_.getSpellingLineNumber(loc));
        }
      }
    }

    void fill(proto::Preprocess* pp)
    {
      for (auto& it : macros_)
      {
        pp->mutable_macros()->AddAllocated(it.second.release());
      }
    }

    // offset to macro
    std::map<unsigned, std::unique_ptr<proto::Macro>> macros_;
  };

  clang::CompilerInstance& compiler_;
  const clang::Preprocessor& preprocessor_;
  clang::SourceManager& sourceManager_;
  Sink* sink_;

  // map from filename to file content
  std::map<std::string, std::string> files_;
  // map from filename to inclusions
  std::unordered_map<std::string, Inclusions> inclusions_;
  // map from filename to macros
  std::unordered_map<std::string, Macros> macros_;

  static constexpr const char* kSrcmd5 = "srcmd5:";
};

