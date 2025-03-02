///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Unicode.cpp                                                               //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides utitlity functions to work with Unicode and other encodings.     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include <specstrings.h>
#else
#include <clocale>
#endif
#include <string>
#include "dxc/Support/Global.h"
#include "dxc/Support/Unicode.h"
#include "dxc/Support/WinIncludes.h"

#ifndef _WIN32
// MultiByteToWideChar which is a Windows-specific method.
// This is a very simplistic implementation for non-Windows platforms. This
// implementation completely ignores CodePage and dwFlags.
int MultiByteToWideChar(uint32_t CodePage, uint32_t /*dwFlags*/,
                        const char *lpMultiByteStr, int cbMultiByte,
                        wchar_t *lpWideCharStr, int cchWideChar) {

  if (cbMultiByte == 0) {
    SetLastError(ERROR_INVALID_PARAMETER);
    return 0;
  }

  // if cbMultiByte is -1, it indicates that lpMultiByteStr is null-terminated
  // and the entire string should be processed.
  if (cbMultiByte == -1) {
    for (cbMultiByte = 0; lpMultiByteStr[cbMultiByte] != '\0'; ++cbMultiByte)
      ;
    // Add 1 for the null-terminating character.
    ++cbMultiByte;
  }
  // If zero is given as the destination size, this function should
  // return the required size (including the null-terminating character).
  // This is the behavior of mbstowcs when the target is null.
  if (cchWideChar == 0) {
    lpWideCharStr = nullptr;
  } else if (cchWideChar < cbMultiByte) {
    SetLastError(ERROR_INSUFFICIENT_BUFFER);
    return 0;
  }

  size_t rv;
  const char *locale = CPToLocale(CodePage);
  locale = setlocale(LC_ALL, locale);
  if (lpMultiByteStr[cbMultiByte - 1] != '\0') {
    char *srcStr = (char *)malloc((cbMultiByte +1) * sizeof(char));
    strncpy(srcStr, lpMultiByteStr, cbMultiByte);
    srcStr[cbMultiByte]='\0';
    rv = mbstowcs(lpWideCharStr, srcStr, cchWideChar);
    free(srcStr);
  } else {
    rv = mbstowcs(lpWideCharStr, lpMultiByteStr, cchWideChar);
  }
  setlocale(LC_ALL, locale);
  if (rv == (size_t)cbMultiByte) return rv;
  return rv + 1; // mbstowcs excludes the terminating character
}

// WideCharToMultiByte is a Windows-specific method.
// This is a very simplistic implementation for non-Windows platforms. This
// implementation completely ignores CodePage and dwFlags.
int WideCharToMultiByte(uint32_t CodePage, uint32_t /*dwFlags*/,
                        const wchar_t *lpWideCharStr, int cchWideChar,
                        char *lpMultiByteStr, int cbMultiByte,
                        const char * /*lpDefaultChar*/,
                        bool * /*lpUsedDefaultChar*/) {

  if (cchWideChar == 0) {
    SetLastError(ERROR_INVALID_PARAMETER);
    return 0;
  }

  // if cchWideChar is -1, it indicates that lpWideCharStr is null-terminated
  // and the entire string should be processed.
  if (cchWideChar == -1) {
    for (cchWideChar = 0; lpWideCharStr[cchWideChar] != '\0'; ++cchWideChar)
      ;
    // Add 1 for the null-terminating character.
    ++cchWideChar;
  }
  // If zero is given as the destination size, this function should
  // return the required size (including the null-terminating character).
  // This is the behavior of wcstombs when the target is null.
  if (cbMultiByte == 0) {
    lpMultiByteStr = nullptr;
  } else if (cbMultiByte < cchWideChar) {
    SetLastError(ERROR_INSUFFICIENT_BUFFER);
    return 0;
  }

  size_t rv;
  const char *locale = CPToLocale(CodePage);
  locale = setlocale(LC_ALL, locale);
  if (lpWideCharStr[cchWideChar - 1] != L'\0') {
    wchar_t *srcStr = (wchar_t *)malloc((cchWideChar+1) * sizeof(wchar_t));
    wcsncpy(srcStr, lpWideCharStr, cchWideChar);
    srcStr[cchWideChar] = L'\0';
    rv = wcstombs(lpMultiByteStr, srcStr, cbMultiByte);
    free(srcStr);
  } else {
    rv = wcstombs(lpMultiByteStr, lpWideCharStr, cbMultiByte);
  }
  setlocale(LC_ALL, locale);
  if (rv == (size_t)cchWideChar) return rv;
  return rv + 1; // mbstowcs excludes the terminating character
}
#endif // _WIN32

namespace Unicode {

_Success_(return != false)
bool UTF16ToEncodedString(_In_z_ const wchar_t* text, size_t cUTF16, DWORD cp, DWORD flags, _Inout_ std::string* pValue, _Out_opt_ bool* lossy) {
  BOOL usedDefaultChar;
  LPBOOL pUsedDefaultChar = (lossy == nullptr) ? nullptr : &usedDefaultChar;
  if (lossy != nullptr) *lossy = false;

  // Handle zero-length as a special case; it's a special value to indicate errors in WideCharToMultiByte.
  if (cUTF16 == 0) {
    pValue->resize(0);
    DXASSERT(lossy == nullptr || *lossy == false, "otherwise earlier initialization in this function was updated");
    return true;
  }

  int cbUTF8 = ::WideCharToMultiByte(cp, flags, text, cUTF16, nullptr, 0, nullptr, pUsedDefaultChar);
  if (cbUTF8 == 0)
    return false;

  pValue->resize(cbUTF8);

  cbUTF8 = ::WideCharToMultiByte(cp, flags, text, cUTF16, &(*pValue)[0], pValue->size(), nullptr, pUsedDefaultChar);
  DXASSERT(cbUTF8 > 0, "otherwise contents have changed");
  DXASSERT((*pValue)[pValue->size()] == '\0', "otherwise string didn't null-terminate after resize() call");

  if (lossy != nullptr) *lossy = usedDefaultChar;
  return true;
}

_Use_decl_annotations_
bool UTF8ToUTF16String(const char *pUTF8, std::wstring *pUTF16) {
  size_t cbUTF8 = (pUTF8 == nullptr) ? 0 : strlen(pUTF8);
  return UTF8ToUTF16String(pUTF8, cbUTF8, pUTF16);
}

_Use_decl_annotations_
bool UTF8ToUTF16String(const char *pUTF8, size_t cbUTF8, std::wstring *pUTF16) {
  DXASSERT_NOMSG(pUTF16 != nullptr);

  // Handle zero-length as a special case; it's a special value to indicate
  // errors in MultiByteToWideChar.
  if (cbUTF8 == 0) {
    pUTF16->resize(0);
    return true;
  }

  int cUTF16 = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pUTF8,
                                     cbUTF8, nullptr, 0);
  if (cUTF16 == 0)
    return false;

  pUTF16->resize(cUTF16);

  cUTF16 = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pUTF8, cbUTF8,
                                 &(*pUTF16)[0], pUTF16->size());
  DXASSERT(cUTF16 > 0, "otherwise contents changed");
  DXASSERT((*pUTF16)[pUTF16->size()] == L'\0',
           "otherwise wstring didn't null-terminate after resize() call");
  return true;
}

std::wstring UTF8ToUTF16StringOrThrow(_In_z_ const char *pUTF8) {
  std::wstring result;
  if (!UTF8ToUTF16String(pUTF8, &result)) {
    throw hlsl::Exception(DXC_E_STRING_ENCODING_FAILED);
  }
  return result;
}

_Use_decl_annotations_
bool UTF8ToConsoleString(_In_z_ const char* text, _In_ size_t textLen, _Inout_ std::string* pValue, _Out_opt_ bool* lossy) {
  DXASSERT_NOMSG(text != nullptr);
  DXASSERT_NOMSG(pValue != nullptr);
  std::wstring text16;
  if (lossy != nullptr) *lossy = false;
  if (!UTF8ToUTF16String(text, textLen, &text16)) {
    return false;
  }
  return UTF16ToConsoleString(text16.c_str(), text16.length(), pValue, lossy);
}

_Use_decl_annotations_
bool UTF8ToConsoleString(_In_z_ const char* text, _Inout_ std::string* pValue, _Out_opt_ bool* lossy) {
  return UTF8ToConsoleString(text, strlen(text), pValue, lossy);
}

_Use_decl_annotations_
bool UTF16ToConsoleString(const wchar_t* text, _In_ size_t textLen, std::string* pValue, bool* lossy) {
  DXASSERT_NOMSG(text != nullptr);
  DXASSERT_NOMSG(pValue != nullptr);
  UINT cp = GetConsoleOutputCP();
  return UTF16ToEncodedString(text, textLen, cp, 0, pValue, lossy);
}

_Use_decl_annotations_
bool UTF16ToConsoleString(const wchar_t* text, std::string* pValue, bool* lossy) {
  return UTF16ToConsoleString(text, wcslen(text), pValue, lossy);
}

_Use_decl_annotations_
bool UTF16ToUTF8String(const wchar_t *pUTF16, size_t cUTF16, std::string *pUTF8) {
  DXASSERT_NOMSG(pUTF16 != nullptr);
  DXASSERT_NOMSG(pUTF8 != nullptr);
  return UTF16ToEncodedString(pUTF16, cUTF16, CP_UTF8, 0, pUTF8, nullptr);
}

_Use_decl_annotations_
bool UTF16ToUTF8String(const wchar_t *pUTF16, std::string *pUTF8) {
  DXASSERT_NOMSG(pUTF16 != nullptr);
  DXASSERT_NOMSG(pUTF8 != nullptr);
  return UTF16ToEncodedString(pUTF16, wcslen(pUTF16), CP_UTF8, 0, pUTF8, nullptr);
}

std::string UTF16ToUTF8StringOrThrow(_In_z_ const wchar_t *pUTF16) {
  std::string result;
  if (!UTF16ToUTF8String(pUTF16, &result)) {
    throw hlsl::Exception(DXC_E_STRING_ENCODING_FAILED);
  }
  return result;
}

_Use_decl_annotations_
bool UTF8BufferToUTF16ComHeap(const char *pUTF8, wchar_t **ppUTF16) throw() {
  *ppUTF16 = nullptr;
  int c = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pUTF8, -1,
                                nullptr, 0);
  if (c == 0)
    return false;
  CComHeapPtr<wchar_t> p;
  if (!p.Allocate(c))
    return false;
  DXVERIFY_NOMSG(0 < ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pUTF8,
                                           -1, p.m_pData, c));
  *ppUTF16 = p.Detach();
  return true;
}

_Use_decl_annotations_
bool UTF8BufferToUTF16Buffer(const char *pUTF8, int cbUTF8, wchar_t **ppUTF16, size_t *pcUTF16) throw() {
  *ppUTF16 = nullptr;
  *pcUTF16 = 0;

  if (cbUTF8 == 0 || (cbUTF8 == -1 && *pUTF8 == '\0')) {
    *ppUTF16 = new (std::nothrow) wchar_t[1];
    if (*ppUTF16 == nullptr)
      return false;
    (*ppUTF16)[0] = L'\0';
    *pcUTF16 = 1;
    return true;
  }

  int c = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pUTF8, cbUTF8, nullptr, 0);
  if (c == 0)
    return false;

  // add space for null-terminator if we're not accounting for it
  if (cbUTF8 != -1)
    c += 1;

  wchar_t *p = new (std::nothrow) wchar_t[c];

  if (p == nullptr)
    return false;

  int converted = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                            pUTF8, cbUTF8,
                            p, c);
  (void)converted;
  DXASSERT(converted > 0, "otherwise contents have changed");
  p[c - 1] = L'\0';

  *ppUTF16 = p;
  *pcUTF16 = c;

  return true;
}

_Use_decl_annotations_
bool UTF16BufferToUTF8Buffer(const wchar_t *pUTF16, int cUTF16, char **ppUTF8, size_t *pcUTF8) throw() {
  *ppUTF8 = nullptr;
  *pcUTF8 = 0;

  if (cUTF16 == 0 || (cUTF16 == -1 && *pUTF16 == '\0')) {
    *ppUTF8 = new (std::nothrow) char[1];
    if (*ppUTF8 == nullptr)
      return false;
    (*ppUTF8)[0] = '\0';
    *pcUTF8 = 1;
    return true;
  }

  int c1 = ::WideCharToMultiByte(CP_UTF8, // code page
                                 0,       // flags
                                 pUTF16,  // string to convert
                                 cUTF16,  // size, in chars, of string to convert
                                 nullptr, // output buffer
                                 0,       // size of output buffer
                                 nullptr, nullptr);
  if (c1 == 0)
    return false;

  // add space for null-terminator if we're not accounting for it
  if (cUTF16 != -1)
    c1 += 1;

  char *p = new (std::nothrow) char[c1];
  if (p == nullptr)
    return false;

  int converted = ::WideCharToMultiByte(CP_UTF8, 0,
                            pUTF16, cUTF16,
                            p, c1,
                            nullptr, nullptr);
  (void)converted;
  DXASSERT(converted > 0, "otherwise contents have changed");
  p[c1 - 1] = '\0';

  *ppUTF8 = p;
  *pcUTF8 = c1;

  return true;
}

template<typename TChar>
static
bool IsStarMatchT(const TChar *pMask, size_t maskLen, const TChar *pName, size_t nameLen, TChar star) {
  if (maskLen == 0 && nameLen == 0) {
    return true;
  }
  if (maskLen == 0 || nameLen == 0) {
    return false;
  }

  if (pMask[maskLen - 1] == star) {
    // Prefix match.
    if (maskLen == 1) { // For just '*', everything is a match.
      return true;
    }
    --maskLen;
    if (maskLen > nameLen) { // Mask is longer than name, can't be a match.
      return false;
    }
    return 0 == memcmp(pMask, pName, sizeof(TChar) * maskLen);
  }
  else {
    // Exact match.
    if (nameLen != maskLen) {
      return false;
    }
    return 0 == memcmp(pMask, pName, sizeof(TChar) * nameLen);
  }
}

_Use_decl_annotations_
bool IsStarMatchUTF8(const char *pMask, size_t maskLen, const char *pName, size_t nameLen) {
  return IsStarMatchT<char>(pMask, maskLen, pName, nameLen, '*');
}

_Use_decl_annotations_
bool IsStarMatchUTF16(const wchar_t *pMask, size_t maskLen, const wchar_t *pName, size_t nameLen) {
  return IsStarMatchT<wchar_t>(pMask, maskLen, pName, nameLen, L'*');
}


}  // namespace Unicode
