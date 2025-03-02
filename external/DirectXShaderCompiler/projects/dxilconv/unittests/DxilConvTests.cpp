///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilConvTest.cpp                                                          //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides tests for the dxilconv.dll API.                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef UNICODE
#define UNICODE
#endif

#include <memory>
#include <vector>
#include <string>
#include <map>
#include <cassert>
#include <sstream>
#include <algorithm>
#include <cfloat>
#include "dxc/DxilContainer/DxilContainer.h"
#include "dxc/Support/WinIncludes.h"
#include "dxc/dxcapi.h"
#include <atlfile.h>

#include "dxc/test/HLSLTestData.h"

#define HLSLDATAFILEPARAM L"DxilConvDataDir"
#include "dxc/test/HlslTestUtils.h"

#include "dxc/test/DxcTestUtils.h"

#include "llvm/Support/raw_os_ostream.h"
#include "dxc/Support/Global.h"
#include "dxc/Support/dxcapi.use.h"
#include "dxc/Support/microcom.h"
#include "dxc/Support/HLSLOptions.h"
#include "dxc/Support/Unicode.h"

#include <fstream>
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MSFileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringSwitch.h"

using namespace std;

class DxilConvTest {
public:
  BEGIN_TEST_CLASS(DxilConvTest)
    TEST_CLASS_PROPERTY(L"Parallel", L"true")
    TEST_METHOD_PROPERTY(L"Priority", L"0")
    END_TEST_CLASS()

    TEST_CLASS_SETUP(InitSupport);

  TEST_METHOD(BatchDxbc2dxil);
  TEST_METHOD(BatchDxbc2dxilAsm);
  TEST_METHOD(BatchDxilCleanup);
  TEST_METHOD(BatchNormalizeDxil);
  TEST_METHOD(BatchScopeNestIterator);
  TEST_METHOD(RegressionTests);

  BEGIN_TEST_METHOD(ManualFileCheckTest)
    TEST_METHOD_PROPERTY(L"Ignore", L"true")
    END_TEST_METHOD()

private:
  dxc::DxcDllSupport m_dllSupport;
  PluginToolsPaths m_TestToolPaths;

  void DxilConvTestCheckFile(LPCWSTR path) {
    FileRunTestResult t = FileRunTestResult::RunFromFileCommands(path, m_dllSupport, &m_TestToolPaths);
    if (t.RunResult != 0) {
      CA2W commentWide(t.ErrorMessage.c_str(), CP_UTF8);
      WEX::Logging::Log::Comment(commentWide);
      WEX::Logging::Log::Error(L"Run result is not zero");
    }
  }

  void DxilConvTestCheckBatchDir(std::wstring suitePath, std::string fileExt = ".hlsl", bool useRelativeFilename = false) {
    using namespace llvm;
    using namespace WEX::TestExecution;

    ::llvm::sys::fs::MSFileSystem *msfPtr;
    VERIFY_SUCCEEDED(CreateMSFileSystemForDisk(&msfPtr));
    std::unique_ptr<::llvm::sys::fs::MSFileSystem> msf(msfPtr);
    ::llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
    IFTLLVM(pts.error_code());

    if (!useRelativeFilename) {
      CW2A pUtf8Filename(suitePath.c_str());
      if (!llvm::sys::path::is_absolute(pUtf8Filename.m_psz)) {
        suitePath = hlsl_test::GetPathToHlslDataFile(suitePath.c_str());
      }
    }
    CW2A utf8SuitePath(suitePath.c_str());

    unsigned numTestsRun = 0;

    std::error_code EC;
    llvm::SmallString<128> DirNative;
    llvm::StringRef filterExt(fileExt);
    llvm::sys::path::native(utf8SuitePath.m_psz, DirNative);
    for (llvm::sys::fs::recursive_directory_iterator Dir(DirNative, EC), DirEnd;
      Dir != DirEnd && !EC; Dir.increment(EC)) {
      if (!llvm::sys::path::extension(Dir->path()).equals(filterExt)) {
        continue;
      }
      StringRef filename = Dir->path();
      CA2W wRelPath(filename.data());

      WEX::Logging::Log::StartGroup(wRelPath);
      DxilConvTestCheckFile(wRelPath);
      WEX::Logging::Log::EndGroup(wRelPath);

      numTestsRun++;
    }

    VERIFY_IS_GREATER_THAN(numTestsRun, (unsigned)0, L"No test files found in batch directory.");
  }

  bool GetCurrentBinDir(std::string &binDir) {
    // get the test dll directory
    HMODULE hModule;
    if ((hModule = ::GetModuleHandleA("dxilconv-tests.dll")) == 0) {
      hlsl_test::LogErrorFmt(L"GetModuleHandle failed.");
      return false;
    }
    // get the path
    char buffer[MAX_PATH];
    DWORD size = sizeof(buffer);
    if ((size = ::GetModuleFileNameA(hModule, buffer, size)) == 0) {
      hlsl_test::LogErrorFmt(L"GetModuleFileName failed.");
      return false;
    }
    std::string str = std::string(buffer, size);
    DWORD pos;
    if ((pos = str.find_last_of("\\")) != std::string::npos) {
      str = str.substr(0, pos + 1);
    }
    binDir.assign(str);
    return true;
  }

  // Find fxc.exe in current bin dir or via WIN10_SDK_PATH environment variable or in the currently installed Windows SDK.
  // Add it to m_TestToolPaths so it can be invoked from RUN: lines by the ref name %fxc
  bool FindFxc() {
    // Find fxc.exe
    // 1. in current bin dir
    std::string binDir;
    if (!GetCurrentBinDir(binDir)) {
      return false;
    }
    std::string fxcLoc = binDir + "fxc.exe";
    if (::PathFileExistsA(fxcLoc.c_str())) {
      m_TestToolPaths.emplace("%fxc", fxcLoc);
    }
    else {
      std::string sdkPath;
      // 2. based on WIN10_SDK_PATH environment variable
      char win10SdkPath[MAX_PATH];
      size_t win10SdkPathLen = ::GetEnvironmentVariableA("WIN10_SDK_PATH", win10SdkPath, MAX_PATH);
      if (win10SdkPathLen > 0) {
        sdkPath = std::string(win10SdkPath, win10SdkPathLen);
      }
      else {
        // 3. get current SDK version from registry
        LPCSTR szRegSdkVerLoc = "SOFTWARE\\WOW6432Node\\Microsoft\\Microsoft SDKs\\Windows\\v10.0";
        std::string szInstallFolder, szVersion;
        char buffer[512];
        DWORD size = sizeof(buffer);
        if (SUCCEEDED(::RegGetValueA(HKEY_LOCAL_MACHINE, szRegSdkVerLoc, "InstallationFolder", RRF_RT_REG_SZ, nullptr, (PVOID)buffer, &size))) {
          szInstallFolder = std::string(buffer, size - 1); // skip trailing 0
          if (SUCCEEDED(::RegGetValueA(HKEY_LOCAL_MACHINE, szRegSdkVerLoc, "ProductVersion", RRF_RT_REG_SZ, nullptr, (PVOID)buffer, &size))) {
            szVersion = std::string(buffer, size - 1); // skip trailing 0
            sdkPath = szInstallFolder + "bin\\" + szVersion;
            // ProductVersion will be something like 10.0.18362 or 10.0.18362.0; we need the one that matches the directory name
            std::string sdkPathDot0 = sdkPath + ".0";
            if (::PathFileExistsA(sdkPathDot0.c_str())) {
              sdkPath = sdkPathDot0;
            }
          }
        }
      }

      if (sdkPath.size() > 0) {
        fxcLoc = sdkPath + std::string("\\x64\\fxc.exe");
      }

      if (::PathFileExistsA(fxcLoc.c_str())) {
        m_TestToolPaths.emplace("%fxc", fxcLoc);
      }
      else {
        CA2W fxcLocW(fxcLoc.c_str(), CP_UTF8);
        hlsl_test::LogErrorFmt(L"Cannot find %s.", fxcLocW.m_psz);
        return false;
      }
    }
    return true;
  }

  // Find the binary in current bin dir and add it to m_TestToolPaths
  // so that it can be invoked from RUN: lines by the refName
  bool FindToolInBinDir(std::string refName, std::string binaryName) {
    std::string binDir;
    if (!GetCurrentBinDir(binDir)) {
      return false;
    }
    std::string loc = binDir + binaryName;
    if (::PathFileExistsA(loc.c_str())) {
      m_TestToolPaths.emplace(refName, loc);
    }
    else {
      CA2W locW(loc.c_str(), CP_UTF8);
      hlsl_test::LogErrorFmt(L"Cannot find %s.", locW.m_psz);
      return false;
    }
    return true;
  }
};

bool DxilConvTest::InitSupport() {
  if (!m_dllSupport.IsEnabled()) {
    VERIFY_SUCCEEDED(m_dllSupport.Initialize());
  }

  if (!FindFxc()) {
    return false;
  }
  if (!FindToolInBinDir("%dxbc2dxil", "dxbc2dxil.exe")) {
    return false;
  }
  if (!FindToolInBinDir("%opt-exe", "opt.exe")) {
    return false;
  }
  return true;
}

TEST_F(DxilConvTest, ManualFileCheckTest) {
  using namespace llvm;
  using namespace WEX::TestExecution;

  WEX::Common::String value;
  VERIFY_SUCCEEDED(RuntimeParameters::TryGetValue(L"InputPath", value));

  std::wstring path = value;
  if (!llvm::sys::path::is_absolute(CW2A(path.c_str()).m_psz)) {
    path = hlsl_test::GetPathToHlslDataFile(path.c_str());
  }

  bool isDirectory;
  {
    // Temporarily setup the filesystem for testing whether the path is a directory.
    // If it is, CodeGenTestCheckBatchDir will create its own instance.
    llvm::sys::fs::MSFileSystem *msfPtr;
    VERIFY_SUCCEEDED(CreateMSFileSystemForDisk(&msfPtr));
    std::unique_ptr<llvm::sys::fs::MSFileSystem> msf(msfPtr);
    llvm::sys::fs::AutoPerThreadSystem pts(msf.get());
    IFTLLVM(pts.error_code());
    isDirectory = llvm::sys::fs::is_directory(CW2A(path.c_str()).m_psz);
  }

  if (isDirectory) {
    DxilConvTestCheckBatchDir(path);
  } else {
    DxilConvTestCheckFile(path.c_str() );
  }
}

TEST_F(DxilConvTest, BatchDxbc2dxil) {
  DxilConvTestCheckBatchDir(L"dxbc2dxil", ".hlsl");
}

TEST_F(DxilConvTest, BatchDxbc2dxilAsm) {
  DxilConvTestCheckBatchDir(L"dxbc2dxil-asm", ".asm");
}

TEST_F(DxilConvTest, BatchDxilCleanup) {
  // switch current directory to directory with test files and use relative paths
  // because the reference files contain file path as ModuleID
  wchar_t curDir[MAX_PATH];
  IFT(GetCurrentDirectoryW(sizeof(curDir), curDir) == 0);

  wstring testFilesPath = hlsl_test::GetPathToHlslDataFile(L"");
  IFT(::SetCurrentDirectory(testFilesPath.c_str()));

  DxilConvTestCheckBatchDir(L"dxil_cleanup", ".ll", true);

  IFT(::SetCurrentDirectory(curDir));
}
 
TEST_F(DxilConvTest, BatchNormalizeDxil) {
  DxilConvTestCheckBatchDir(L"normalize_dxil", ".ll");
}

TEST_F(DxilConvTest, BatchScopeNestIterator) {  
  DxilConvTestCheckBatchDir(L"scope_nest_iterator", ".ll");
}

TEST_F(DxilConvTest, RegressionTests) {
  DxilConvTestCheckBatchDir(L"regression_tests", ".hlsl");
}
