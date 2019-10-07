
// IMGRename.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CIMGRenameApp:
// See IMGRename.cpp for the implementation of this class
//

class CIMGRenameApp : public CWinApp
{
public:
  const static WCHAR AppName[MAX_PATH];  // application name for registry key

public:
	CIMGRenameApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CIMGRenameApp theApp;