#include "stdafx.h"
#include "Registry.h"

using std::wstring;

namespace Registry
{

  // read a string from Registry, return def if missing. do not create if missing
  std::wstring Reg::GetString(HKEY hKey, const std::wstring& key, const std::wstring& value, const std::wstring& content)
  {
    DWORD size{};
    LONG retCode = ::RegGetValue(hKey, key.c_str(), value.c_str(), RRF_RT_REG_SZ, nullptr, nullptr, &size);  // Request the size of the string, in bytes
    switch (retCode)
    {
      case ERROR_SUCCESS:  // continue with the next step
        break;
      default:             // key or value not found or not readable -> return the provided default
        return content;
    }

    std::wstring data;
    data.resize(size / sizeof(wchar_t));
    retCode = ::RegGetValue(hKey, key.c_str(), value.c_str(), RRF_RT_REG_SZ, nullptr, &data[0], &size);
    DWORD stringLengthInWchars = size / sizeof(wchar_t);
    stringLengthInWchars--; // exclude the NUL written by the Win32 API
    data.resize(stringLengthInWchars);
    return data;
  }

  // write a string to Registry; create if missing
  bool Reg::SetString(HKEY hKey, const std::wstring& key, const std::wstring& value, const std::wstring& content)
  {
    // create (in case if not existing)
    HKEY sKey{};
    DWORD status{};
    ::RegCreateKeyEx(hKey, key.c_str(), 0, 0, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, 0, &sKey, &status);
    ::RegSetValueEx(sKey, value.c_str(), 0, REG_SZ, (LPBYTE)content.c_str(), wcslen(content.c_str()) * sizeof(std::wstring));
    ::RegCloseKey(sKey);
    return status == REG_CREATED_NEW_KEY;
  }

  
  // read an int from Registry, return def if missing. do not create if missing
  int Reg::GetInt(HKEY hKey, const std::wstring& key, const std::wstring& value, int content)
  {
    DWORD data{};
    DWORD size{};
    ::RegGetValue(hKey, key.c_str(), value.c_str(), RRF_RT_DWORD, nullptr, &data, &size);
    return data;
  }

  // write an int to Registry; create if missing
  bool Reg::SetInt(HKEY hKey, const std::wstring& key, const std::wstring& value, int content)
  {
    // create (in case if not existing)
    HKEY sKey{};
    DWORD status{};
    ::RegCreateKeyEx(hKey, key.c_str(), 0, 0, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, 0, &sKey, &status);
    ::RegSetValueEx(sKey, value.c_str(), 0, REG_DWORD, (LPBYTE)&content, sizeof(int));
    ::RegCloseKey(sKey);
    return status == REG_CREATED_NEW_KEY;

  }
}
