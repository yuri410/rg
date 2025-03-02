set(HLSL_IGNORE_SOURCES CGObjCGNU.cpp CGObjCMac.cpp CGObjCRuntime.cpp CGOpenCLRuntime.cpp CGOpenMPRuntime.cpp)

set(LLVM_LINK_COMPONENTS
  Analysis
  BitReader
  BitWriter
  Core
  DXIL
  DxilRootSignature
  IPO
  IRReader
  InstCombine
#  Instrumentation # HLSL Change
  Linker
#  MC # HLSL Change
#  ObjCARCOpts # HLSL Change
#  Object # HLSL Change
  ProfileData
  ScalarOpts
  Support
  Target
  TransformUtils
  )

# In a standard Clang+LLVM build, we need to generate intrinsics before
# building codegen. In a standalone build, LLVM is already built and we don't
# need this dependency. Furthermore, LLVM doesn't export it so we can't have
# this dependency.
set(codegen_deps intrinsics_gen)
if (CLANG_BUILT_STANDALONE)
  set(codegen_deps)
endif()

add_clang_library(clangCodeGen
  BackendUtil.cpp
  CGAtomic.cpp
  CGBlocks.cpp
  CGBuiltin.cpp
  CGCUDANV.cpp
  CGCUDARuntime.cpp
  CGCXX.cpp
  CGCXXABI.cpp
  CGCall.cpp
  CGClass.cpp
  CGCleanup.cpp
  CGDebugInfo.cpp
  CGDecl.cpp
  CGDeclCXX.cpp
  CGException.cpp
  CGExpr.cpp
  CGExprAgg.cpp
  CGExprCXX.cpp
  CGExprComplex.cpp
  CGExprConstant.cpp
  CGExprScalar.cpp
  CGHLSLRuntime.cpp
  CGHLSLMS.cpp
  CGHLSLMSFinishCodeGen.cpp
  CGHLSLRootSignature.cpp
  CGLoopInfo.cpp
  CGObjC.cpp
  # CGObjCGNU.cpp # HLSL Change - no definitions used
  # CGObjCMac.cpp # HLSL Change - no definitions used
  # CGObjCRuntime.cpp # HLSL Change - no definitions used
  # CGOpenCLRuntime.cpp # HLSL Change - no definitions used
  # CGOpenMPRuntime.cpp # HLSL Change - no definitions used
  CGRecordLayoutBuilder.cpp
  CGStmt.cpp
  CGStmtOpenMP.cpp
  CGVTT.cpp
  CGVTables.cpp
  CodeGenABITypes.cpp
  CodeGenAction.cpp
  CodeGenFunction.cpp
  CodeGenModule.cpp
  CodeGenPGO.cpp
  CodeGenTBAA.cpp
  CodeGenTypes.cpp
  CoverageMappingGen.cpp
  ItaniumCXXABI.cpp
  MicrosoftCXXABI.cpp
  ModuleBuilder.cpp
  ObjectFilePCHContainerOperations.cpp
  SanitizerMetadata.cpp
  TargetInfo.cpp

  DEPENDS
  ${codegen_deps}

  LINK_LIBS
  clangAST
  clangBasic
  clangFrontend
  clangLex
  )
