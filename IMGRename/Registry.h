#ifndef REGISTRY
#define REGISTRY

namespace Registry
{

  class Reg
  {
  public:
    static std::wstring GetString(HKEY hKey, const std::wstring& key, const std::wstring& value, const std::wstring& content);    // read a string from Registry, return default if missing. do not create if missing
    static bool         SetString(HKEY hKey, const std::wstring& key, const std::wstring& value, const std::wstring& content);    // write a string to Registry; if missing, create and return true
    static int          GetInt(HKEY hKey, const std::wstring& key, const std::wstring& value, int content);                       // read an int from Registry, return def if missing. do not create if missing
    static bool         SetInt(HKEY hKey, const std::wstring& key, const std::wstring& value, int content);                       // write an int to Registry; if missing, create and return true
  };
}
#endif // REGISTRY
