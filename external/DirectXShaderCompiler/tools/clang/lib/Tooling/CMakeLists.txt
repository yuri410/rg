set(LLVM_LINK_COMPONENTS support)

add_subdirectory(Core)

add_clang_library(clangTooling
  ArgumentsAdjusters.cpp
  CommonOptionsParser.cpp
  CompilationDatabase.cpp
  FileMatchTrie.cpp
  JSONCompilationDatabase.cpp
  Refactoring.cpp
  RefactoringCallbacks.cpp
  Tooling.cpp

  LINK_LIBS
  clangAST
  clangASTMatchers
  clangBasic
  clangDriver
  clangFrontend
  clangLex
  clangRewrite
  clangToolingCore
  )
