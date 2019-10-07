#include "stdafx.h"
#include "afxdialogex.h"

namespace Tools
{

  std::wstring PickDirectory(const std::wstring& path)
  {
    std::wstring Result{};

    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (SUCCEEDED(hr))
    {
      IFileOpenDialog *pFileOpen;

      // Create the FileOpenDialog object.
      hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

      // limit to folders
      pFileOpen->SetOptions(FOS_PICKFOLDERS | FOS_PATHMUSTEXIST);

      if (SUCCEEDED(hr))
      {
        // Show the Open dialog box.
        hr = pFileOpen->Show(NULL);

        // Get the file name from the dialog box.
        if (SUCCEEDED(hr))
        {
          IShellItem *pItem;
          hr = pFileOpen->GetResult(&pItem);
          if (SUCCEEDED(hr))
          {
            PWSTR pszFilePath;
            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
            Result = pszFilePath;
            pItem->Release();
          }
        }
        pFileOpen->Release();
      }
      CoUninitialize();
    }
    return Result;
  }
}