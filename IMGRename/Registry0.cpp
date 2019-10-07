#include "stdafx.h"
#include "Registry.h"

namespace Registry_Helper_old
{

  //-----------------------------------------------------------------------------
//                              Implementations
//-----------------------------------------------------------------------------


//
// Safe, efficient and convenient C++ wrapper around raw HKEY handles.
//
// This class is movable but not copyable.
//
// This class is designed to be very *efficient*, for example non-throwing 
// operations are carefully marked as noexcept, so the C++ compiler can emit 
// optimized code for them.
//
// Moreover, this class just wraps a raw HKEY handle, so it's very efficient
// (there's no shared-ownership overhead like in std::shared_ptr; you can 
// think of this class kind of like a std::unique_ptr for HKEYs).
//
// The class is also swappable (defines a custom non-throwing swap function); 
// comparison and relational operators (==, !=, <, <=, >, >=) are properly 
// overloaded as well.
// 
class RegKey
{
public:

  //
  // Initialize as an empty key handle
  //
  RegKey() noexcept = default;

  //
  // Take ownership of the input raw key handle
  //
  explicit RegKey(HKEY hKey) noexcept
    : m_hKey{ hKey }
  {}

  //
  // Take ownership of the input key handle.
  // The input key handle wrapper is reset to an empty state
  //
  RegKey(RegKey&& other) noexcept
    : m_hKey{ other.m_hKey }
  {
    //
    // Other doesn't own the raw handle anymore
    //
    other.m_hKey = nullptr;
  }

  //
  // Move-assign from the input key handle.
  // Properly check against self-move-assign (which is safe and does nothing).
  //
  RegKey& operator=(RegKey&& other) noexcept
  {
    //
    // Prevent self-move-assign
    //
    if ((this != &other) && (m_hKey != other.m_hKey))
    {
      Close();

      //
      // Move from other
      //
      m_hKey = other.m_hKey;
      other.m_hKey = nullptr;
    }
    return *this;
  }

  //
  // Ban copy
  //
  RegKey(const RegKey&) = delete;
  RegKey& operator=(const RegKey&) = delete;

  //
  // Safely close the wrapped key handle (if any)
  //
  ~RegKey() noexcept
  {
    Close();
  }

  //
  // Give read-only access to the wrapped HKEY handle
  //
  HKEY Get() const noexcept
  {
    return m_hKey;
  }

  //
  // Close current HKEY handle.
  // If key wasn't open, does nothing.
  //
  void Close() noexcept
  {
    if (IsValid())
    {
      ::RegCloseKey(m_hKey);
      m_hKey = nullptr;
    }
  }

  //
  // Is the wrapped HKEY handle valid?
  //
  bool IsValid() const noexcept
  {
    return m_hKey != nullptr;
  }

  //
  // Has the same semantic of IsValid, but allows a short "if (regKey)" syntax
  //
  explicit operator bool() const noexcept
  {
    return IsValid();
  }

  //
  // Transfer ownership of current HKEY to the caller.
  // Note that the caller is responsible for closing the key raw handle!
  //
  HKEY Detach() noexcept
  {
    HKEY hKey = m_hKey;

    //
    // We don't own the HKEY handle anymore
    //
    m_hKey = nullptr;

    //
    // Transfer ownership of the raw handle to the caller
    //
    return hKey;
  }

  //
  // Take ownership of the HKEY handle.
  // Safely close any previously open handle.
  // Input key handle can be nullptr; in this case this wrapper wraps an empty handle.
  //
  void Attach(HKEY hKey) noexcept
  {
    //
    // Prevent self-attach
    // 
    if (m_hKey != hKey)
    {
      Close();

      //
      // Take ownership of hKey
      //
      m_hKey = hKey;
    }
  }

  //
  // Non-throwing swap
  //
  friend void swap(RegKey& a, RegKey& b) noexcept
  {
    //
    // Enable ADL (not necessary in this case, but good practice)
    //
    using std::swap;

    //
    // Swap the raw handle members
    //
    swap(a.m_hKey, b.m_hKey);
  }


  //
  // Overload comparison and relational operators
  // 

  friend bool operator==(const RegKey& a, const RegKey& b) noexcept
  {
    return a.m_hKey == b.m_hKey;
  }

  friend bool operator!=(const RegKey& a, const RegKey& b) noexcept
  {
    return a.m_hKey != b.m_hKey;
  }

  friend bool operator<(const RegKey& a, const RegKey& b) noexcept
  {
    return a.m_hKey < b.m_hKey;
  }

  friend bool operator<=(const RegKey& a, const RegKey& b) noexcept
  {
    return a.m_hKey <= b.m_hKey;
  }

  friend bool operator>(const RegKey& a, const RegKey& b) noexcept
  {
    return a.m_hKey > b.m_hKey;
  }

  friend bool operator>=(const RegKey& a, const RegKey& b) noexcept
  {
    return a.m_hKey >= b.m_hKey;
  }


private:
  //
  // The wrapped registry key handle
  //
  HKEY m_hKey = nullptr;
};


//
// Exception class representing an error of a Windows Registry API call
//
class RegistryError : public std::runtime_error
{
public:
  RegistryError(const char* message, LONG errorCode)
    : std::runtime_error{ message }
    , m_errorCode{ errorCode }
  {}

  LONG ErrorCode() const noexcept
  {
    return m_errorCode;
  }

private:
  LONG m_errorCode;
};


//
// Inline Function Implementations
// 

void Registry::RegSetString(HKEY hKey, const std::wstring& subKey, const std::wstring& value, const std::wstring& content)
{
// create if not existing
  RegCreateKeyEx(hKey, subKey.c_str(), 0, 0, REG_SZ, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, nullptr, nullptr);

  REGSAM regsam{ STANDARD_RIGHTS_REQUIRED | STANDARD_RIGHTS_ALL };
  RegKey regkey = Registry::RegOpenKey(hKey, subKey, regsam);
  switch (RegSetValueEx(hKey, value.c_str(), 0, REG_SZ, (LPBYTE)content.c_str(), wcslen(content.c_str()) * sizeof(char)))
  {
    case 2:
      RegCreateKeyEx(hKey, subKey, REG_SZ, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, nullptr, nullptr);
      RegSetValueEx(hKey, value.c_str(), 0, REG_SZ, (LPBYTE)content.c_str(), wcslen(content.c_str()) * sizeof(char));
      break;
    case ERROR_SUCCESS: return;
  }
}


inline DWORD Registry::RegGetDword(HKEY hKey, const std::wstring& subKey, const std::wstring& value)
{
  //
  // Read a 32-bit DWORD value from the registry
  // 
  DWORD data{};
  DWORD dataSize = sizeof(data);
  LONG retCode = ::RegGetValue(
    hKey,
    subKey.c_str(),
    value.c_str(),
    RRF_RT_REG_DWORD,
    nullptr,
    &data,
    &dataSize);
  if (retCode != ERROR_SUCCESS)
  {
    throw RegistryError{ "Cannot read DWORD from registry.", retCode };
  }

  return data;
}


inline ULONGLONG Registry::RegGetQword(HKEY hKey, const std::wstring& subKey, const std::wstring& value)
{
  //
  // Read a 64-bit QWORD value from the registry
  // 
  ULONGLONG data{};
  DWORD dataSize = sizeof(data);
  LONG retCode = ::RegGetValue(
    hKey,
    subKey.c_str(),
    value.c_str(),
    RRF_RT_REG_QWORD,
    nullptr,
    &data,
    &dataSize);
  if (retCode != ERROR_SUCCESS)
  {
    throw RegistryError{ "Cannot read QWORD from registry.", retCode };
  }

  return data;
}


std::wstring Registry::RegGetString(HKEY hKey, const std::wstring& subKey, const std::wstring& value, const std::wstring& def)
{
  //
  // Request the size of the string, in bytes
  //
  DWORD dataSize{};
  LONG retCode = ::RegGetValue(
    hKey,
    subKey.c_str(),
    value.c_str(),
    RRF_RT_REG_SZ,
    nullptr,
    nullptr,
    &dataSize);
  switch (retCode)
  {
    case 2:
      return def; // not found -> return the provided default
      break;
    case ERROR_SUCCESS:
      break;
    default:
      throw RegistryError{ "Cannot read string from registry", retCode };
  }
  //
  // Allocate room for the result string.
  // 
  // Note that dataSize is in bytes, but wstring::resize() expects length in wchar_ts,
  // so must convert from bytes to wchar_t count first.
  // 
  std::wstring data;
  data.resize(dataSize / sizeof(wchar_t));

  //
  // Read the string from the registry into local wstring object
  //
  retCode = ::RegGetValue(
    hKey,
    subKey.c_str(),
    value.c_str(),
    RRF_RT_REG_SZ,
    nullptr,
    &data[0],
    &dataSize);
  if (retCode != ERROR_SUCCESS)
  {
    throw RegistryError{ "Cannot read string from registry", retCode };
  }

  //
  // On return, RegGetValue() writes in dataSize the actual size of the string, in bytes.
  // We must resize the wstring object to the proper string size.
  // Note that dataSize is expressed in bytes, and includes the terminating NUL; so we have to 
  // subtract the NUL from the total string length, as wstring objects are already NUL-terminated.
  //
  DWORD stringLengthInWchars = dataSize / sizeof(wchar_t);
  stringLengthInWchars--; // exclude the NUL written by the Win32 API
  data.resize(stringLengthInWchars);

  return data;
}


inline std::vector<std::wstring> Registry::RegGetMultiString(HKEY hKey, const std::wstring& subKey, const std::wstring& value)
{
  //
  // Request the size of the multi-string, in bytes
  //
  DWORD dataSize{};
  LONG retCode = ::RegGetValue(
    hKey,
    subKey.c_str(),
    value.c_str(),
    RRF_RT_REG_MULTI_SZ,
    nullptr,
    nullptr,
    &dataSize);
  if (retCode != ERROR_SUCCESS)
  {
    throw RegistryError{ "Cannot read multi-string from registry", retCode };
  }

  //
  // Allocate room for the result multi-string.
  // 
  // Note that dataSize is in bytes, but our vector<wchar_t> object has size expressed in wchar_ts.
  // 
  std::vector<wchar_t> data;
  data.resize(dataSize / sizeof(wchar_t));

  //
  // Read the multi-string from the registry into the vector object
  //
  retCode = ::RegGetValue(
    hKey,
    subKey.c_str(),
    value.c_str(),
    RRF_RT_REG_MULTI_SZ,
    nullptr,
    &data[0],
    &dataSize);
  if (retCode != ERROR_SUCCESS)
  {
    throw RegistryError{ "Cannot read multi-string from registry", retCode };
  }

  //
  // Resize vector to the actual size returned by GetRegValue().
  // Note that the vector is a vector of wchar_ts, instead the size returned by GetRegValue()
  // is in bytes, so we have to scale from bytes to wchar_t count.
  // 
  data.resize(dataSize / sizeof(wchar_t));

  //
  // Parse the double-NUL-terminated string into a vector<wstring>
  //
  std::vector<std::wstring> result;
  const wchar_t* currStringPtr = &data[0];
  while (*currStringPtr != L'\0')
  {
    // Current string is NUL-terminated, so get its length with wcslen()
    const size_t currStringLength = wcslen(currStringPtr);

    // Add current string to result vector
    result.push_back(std::wstring{ currStringPtr, currStringLength });

    // Move to the next string
    currStringPtr += currStringLength + 1;
  }

  return result;
}


inline std::vector<BYTE> Registry::RegGetBinary(HKEY hKey, const std::wstring& subKey, const std::wstring& value)
{
  //
  // Request the size of the binary data, in bytes
  //
  DWORD dataSize{};
  LONG retCode = ::RegGetValue(
    hKey,
    subKey.c_str(),
    value.c_str(),
    RRF_RT_REG_BINARY,
    nullptr,
    nullptr,
    &dataSize);
  if (retCode != ERROR_SUCCESS)
  {
    throw RegistryError{ "Cannot read binary data from registry", retCode };
  }

  //
  // Allocate room for the result binary data
  // 
  std::vector<BYTE> data;
  data.resize(dataSize);

  //
  // Read the binary data from the registry into the vector object
  //
  retCode = ::RegGetValue(
    hKey,
    subKey.c_str(),
    value.c_str(),
    RRF_RT_REG_BINARY,
    nullptr,
    &data[0],
    &dataSize);
  if (retCode != ERROR_SUCCESS)
  {
    throw RegistryError{ "Cannot read binary data from registry", retCode };
  }

  //
  // Resize vector to the actual size returned by GetRegValue().
  // 
  data.resize(dataSize);


  return data;
}


inline std::vector<std::pair<std::wstring, DWORD>> Registry::RegEnumValues(HKEY hKey)
{
  //
  // Get useful pre-enumeration info, like the total number of values
  // and the maximum length of the value names
  //
  DWORD valueCount{};
  DWORD maxValueNameLen{};
  LONG retCode = ::RegQueryInfoKey(
    hKey,
    nullptr,    // no user-defined class
    nullptr,    // no user-defined class size
    nullptr,    // reserved
    nullptr,    // no subkey count
    nullptr,    // no subkey max length
    nullptr,    // no subkey class length
    &valueCount,
    &maxValueNameLen,
    nullptr,    // no max value length
    nullptr,    // no security descriptor
    nullptr     // no last write time
  );
  if (retCode != ERROR_SUCCESS)
  {
    throw RegistryError{ "RegQueryInfoKey failed while preparing for value enumeration.", retCode };
  }

  //
  // NOTE: According to the MSDN documentation, the size returned for value name max length
  // does *not* include the terminating NUL, so let's add +1 to take it into account
  // when I allocate the buffer for reading value names.
  //
  maxValueNameLen++;

  //
  // Preallocate a buffer for the value names
  //
  auto nameBuffer = std::make_unique<wchar_t[]>(maxValueNameLen);

  //
  // The value names and types will be stored here
  //
  std::vector<std::pair<std::wstring, DWORD>> valueInfo;

  //
  // Enumerate all the values
  //
  for (DWORD index = 0; index < valueCount; index++)
  {
    //
    // Get the name and the type of the current value
    //
    DWORD valueNameLen = maxValueNameLen;
    DWORD valueType{};
    retCode = ::RegEnumValue(
      hKey,
      index,
      nameBuffer.get(),
      &valueNameLen,
      nullptr,    // reserved
      &valueType,
      nullptr,    // no data
      nullptr     // no data size
    );
    if (retCode != ERROR_SUCCESS)
    {
      throw RegistryError{ "Cannot enumerate values: RegEnumValue failed.", retCode };
    }

    //
    // On success, the RegEnumValue API writes the length of the
    // value name in the valueNameLen output parameter 
    // (not including the terminating NUL).
    // So we can build a wstring based on that length.
    // 
    // The value info pair is made by the value's name and the value's type.
    //
    valueInfo.push_back(std::make_pair(
      std::wstring{ nameBuffer.get(), valueNameLen },
      valueType
    ));
  }

  return valueInfo;
}


inline std::vector<std::wstring> Registry::RegEnumSubKeys(HKEY hKey)
{
  //
  // Get some useful pre-enumeration info, like the total number of subkeys
  // and the maximum length of the subkey names
  //
  DWORD subKeyCount{};
  DWORD maxSubKeyNameLen{};
  LONG retCode = ::RegQueryInfoKey(
    hKey,
    nullptr,    // no user-defined class
    nullptr,    // no user-defined class size
    nullptr,    // reserved
    &subKeyCount,
    &maxSubKeyNameLen,
    nullptr,    // no subkey class length
    nullptr,    // no value count
    nullptr,    // no value name max length
    nullptr,    // no max value length
    nullptr,    // no security descriptor
    nullptr     // no last write time
  );
  if (retCode != ERROR_SUCCESS)
  {
    throw RegistryError{ "RegQueryInfoKey failed while preparing for subkey enumeration.", retCode };
  }

  //
  // NOTE: According to the MSDN documentation, the size returned for subkey name max length
  // does *not* include the terminating NUL, so let's add +1 to take it into account
  // when I allocate the buffer for reading subkey names.
  //
  maxSubKeyNameLen++;

  //
  // Preallocate a buffer for the subkey names
  //
  auto nameBuffer = std::make_unique<wchar_t[]>(maxSubKeyNameLen);

  //
  // The subkey names will be stored here
  //
  std::vector<std::wstring> subkeyNames;

  //
  // Enumerate all the subkeys
  //
  for (DWORD index = 0; index < subKeyCount; index++)
  {
    //
    // Get the name of the current subkey
    //
    DWORD subKeyNameLen = maxSubKeyNameLen;
    retCode = ::RegEnumKeyEx(
      hKey,
      index,
      nameBuffer.get(),
      &subKeyNameLen,
      nullptr, // reserved
      nullptr, // no class
      nullptr, // no class
      nullptr  // no last write time
    );
    if (retCode != ERROR_SUCCESS)
    {
      throw RegistryError{ "Cannot enumerate subkeys: RegEnumKeyEx failed.", retCode };
    }

    //
    // On success, the ::RegEnumKeyEx API writes the length of the
    // subkey name in the subKeyNameLen output parameter 
    // (not including the terminating NUL).
    // So I can build a wstring based on that length.
    //
    subkeyNames.push_back(std::wstring{ nameBuffer.get(), subKeyNameLen });
  }

  return subkeyNames;
}


inline RegKey Registry::RegCreateKey(
  HKEY hKeyParent,
  const std::wstring& keyName,
  LPTSTR keyClass,
  DWORD options,
  REGSAM access,
  SECURITY_ATTRIBUTES* securityAttributes,
  DWORD* disposition
)
{
  //
  // Create the key invoking the native RegCreateKeyEx Win32 API
  // 
  HKEY hKey = nullptr;
  LONG retCode = ::RegCreateKeyEx(
    hKeyParent,
    keyName.c_str(),
    0,          // reserved
    keyClass,
    options,
    access,
    securityAttributes,
    &hKey,
    disposition
  );
  if (retCode != ERROR_SUCCESS)
  {
    throw RegistryError{ "Cannot create registry key: RegCreateKeyEx failed.", retCode };
  }

  //
  // Return the raw key handle safely wrapped in the C++ key resource manager
  // 
  return RegKey{ hKey };
}


inline RegKey Registry::RegOpenKey(
  HKEY hKeyParent,
  const std::wstring& keyName,
  REGSAM access
)
{
  //
  // Open the key invoking the native RegOpenKeyEx Win32 API
  // 
  HKEY hKey = nullptr;
  LONG retCode = ::RegOpenKeyEx(
    hKeyParent,
    keyName.c_str(),
    0, // default options
    access,
    &hKey
  );
  if (retCode != ERROR_SUCCESS)
  {
    throw RegistryError{ "Cannot open registry key: RegOpenKeyEx failed.", retCode };
  }

  //
  // Return the raw key handle safely wrapped in the C++ key resource manager
  // 
  return RegKey{ hKey };
}

} // namespace Registry_Helper

