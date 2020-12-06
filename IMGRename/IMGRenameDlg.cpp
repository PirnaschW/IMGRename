
// IMGRenameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Registry.h"
#include "IMGRename.h"
#include "IMGRenameDlg.h"
#include "afxdialogex.h"

#include <string.h>         // For wcslen()
#include <string>           // For std::wstring

#include "Tools.h"

using namespace Registry;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CIMGRenameDlg dialog

CIMGRenameDlg::CIMGRenameDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_IMGRENAME_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

  std::wstring regval{};
  regval = Reg::GetString(HKEY_CURRENT_USER, CIMGRenameApp::AppName, L"Path", L"C:\\");
  m_path = regval.c_str();
  regval = Reg::GetString(HKEY_CURRENT_USER, CIMGRenameApp::AppName, L"From", L"IMG_");
  m_from = regval.c_str();
  regval = Reg::GetString(HKEY_CURRENT_USER, CIMGRenameApp::AppName, L"To", L"6D-04");
  m_replace = regval.c_str();
  m_subdir = Reg::GetInt(HKEY_CURRENT_USER, CIMGRenameApp::AppName, L"SubDirs", 0);
}

void CIMGRenameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_PATH, m_path);
  DDV_MaxChars(pDX, m_path, 260);
  DDX_Check(pDX, IDC_SUBDIR, m_subdir);
  DDX_Text(pDX, IDC_FROM, m_from);
  DDX_Text(pDX, IDC_REPLACE, m_replace);
}

BEGIN_MESSAGE_MAP(CIMGRenameDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
  ON_BN_CLICKED(IDC_SELECT, OnSelect)
END_MESSAGE_MAP()


// CIMGRenameDlg message handlers

BOOL CIMGRenameDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIMGRenameDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIMGRenameDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIMGRenameDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CIMGRenameDlg::OnSelect()
{
  UpdateData(TRUE);
  m_path = Tools::PickDirectory(m_path.GetString()).c_str();
  UpdateData(FALSE);
}

void CIMGRenameDlg::OnOK()
{
  UpdateData(TRUE);

  ProcessDirectory(m_path.GetString());

  // save defaults for next runs
  std::wstring regval{};
  regval = m_path;
  Reg::SetString(HKEY_CURRENT_USER, CIMGRenameApp::AppName, L"Path", regval);
  regval = m_from;
  Reg::SetString(HKEY_CURRENT_USER, CIMGRenameApp::AppName, L"From", regval);
  regval = m_replace;
  Reg::SetString(HKEY_CURRENT_USER, CIMGRenameApp::AppName, L"To", regval);
  Reg::SetInt(HKEY_CURRENT_USER, CIMGRenameApp::AppName, L"SubDirs", m_subdir);

  CDialog::OnOK();
}


void CIMGRenameDlg::ProcessFiles(std::wstring path)
{
  std::wstring Pattern = path + L"\\" + m_from.GetString() + L"*.*";
  WIN32_FIND_DATA data;

  HANDLE h = ::FindFirstFile(Pattern.c_str(), &data);
  BOOL more = (h != INVALID_HANDLE_VALUE);
  while (more)
  {
		if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
		{
			CString Path = path.c_str();
			CString OldName = Path + L"\\" + data.cFileName;
			CString NewName = Path + L"\\" + m_replace + (data.cFileName + wcslen(m_from.GetString()));
			CFile::Rename(OldName, NewName);
		}
    more = FindNextFile(h, &data);
  };
  ::FindClose(h);
}

void CIMGRenameDlg::ProcessDirectory(std::wstring path)
{
  ProcessFiles(path);
  if (!m_subdir) return;

  std::wstring pattern = path + L"\\*.*";
  WIN32_FIND_DATA data;
  HANDLE h = ::FindFirstFile(pattern.c_str(), &data);
  BOOL more = (h != INVALID_HANDLE_VALUE);
  while (more)
  {
    if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
      if (wcscmp(data.cFileName, L".") != 0 && wcscmp(data.cFileName, L"..") != 0)
      {
        ProcessDirectory(path + L"\\" + data.cFileName);
      }
    }
    more = FindNextFile(h, &data);
  };
  ::FindClose(h);
}
