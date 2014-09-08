#include "indexer.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include <stdio.h>

static clang::FrontendPluginRegistry::Add<IndexAction>
X("index", "index");

