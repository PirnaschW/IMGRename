#ifndef REGISTRY
#define REGISTRY

/////////////////////////////////////////////////////////////////////////////////////////
//
// Modern C++ Wrappers for Win32 Registry Access APIs
// 
// Copyright (C) 2017 by Giovanni Dicanio <giovanni.dicanio AT gmail.com>
// 
// Compiler: Visual Studio 2015
// Compiles cleanly at /W4 in both 32-bit and 64-bit builds.
// 
// -----------------------------------------------------------------------------------
// 
// The MIT License(MIT)
//
// Copyright(c) 2017 Giovanni Dicanio
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
/////////////////////////////////////////////////////////////////////////////////////////


#include <Windows.h>        // Win32 Platform SDK
#include <string.h>         // For wcslen()
#include <memory>           // For std::unique_ptr
#include <stdexcept>        // For std::runtime_error
#include <string>           // For std::wstring
#include <utility>          // For std::pair, std::swap
#include <vector>           // For std::vector

namespace Registry_Helper
{

  // Exception class representing an error from Windows Registry API calls
  class RegistryError;

  // Wrapper around a raw HKEY handle
  class RegKey;


  //
  // Wrappers for getting registry values
  // 

  class Registry
  {
  public:
    static DWORD                       RegGetDword(HKEY hKey, const std::wstring& subKey, const std::wstring& value);
    static ULONGLONG                   RegGetQword(HKEY hKey, const std::wstring& subKey, const std::wstring& value);
    static std::wstring                RegGetString(HKEY hKey, const std::wstring& subKey, const std::wstring& value, const std::wstring& def);
    static std::vector<std::wstring>   RegGetMultiString(HKEY hKey, const std::wstring& subKey, const std::wstring& value);
    static std::vector<BYTE>           RegGetBinary(HKEY hKey, const std::wstring& subKey, const std::wstring& value);
    static void                        RegSetString(HKEY hKey, const std::wstring& subKey, const std::wstring& value, const std::wstring& content);


  //
  // Enumerates all the values under the given (open) hKey registry key.
  // The information for each value is stored in a pair<wstring, DWORD>,
  // representing the value's name and value's type respectively.
  //
    static std::vector<std::pair<std::wstring, DWORD>> RegEnumValues(HKEY hKey);

    //
    // Enumerate sub-keys of the given (open) hKey registry key.
    // 
    static std::vector<std::wstring> RegEnumSubKeys(HKEY hKey);


    //
    // Wrappers for creating and opening registry keys
    //

    static RegKey RegCreateKey(
      HKEY hKeyParent,
      const std::wstring& keyName,
      LPTSTR keyClass = REG_NONE,
      DWORD options = REG_OPTION_NON_VOLATILE,
      REGSAM access = KEY_READ | KEY_WRITE,
      SECURITY_ATTRIBUTES* securityAttributes = nullptr,
      DWORD* disposition = nullptr
    );

    static RegKey RegOpenKey(
      HKEY hKeyParent,
      const std::wstring& keyName,
      REGSAM access = KEY_READ | KEY_WRITE
    );


  };
}

#endif // REGISTRY

