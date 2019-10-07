#ifndef IMGRENAMEDLG
#define IMGRENAMEDLG

// CIMGRenameDlg dialog
class CIMGRenameDlg : public CDialogEx
{
// Construction
public:
	CIMGRenameDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IMGRENAME_DIALOG };
#endif
  CString m_path;
  BOOL	m_subdir;
  CString	m_from;
  CString	m_replace;

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnSelect();
  virtual void OnOK();
	DECLARE_MESSAGE_MAP()

  void ProcessFiles(std::wstring path);
  void ProcessDirectory(std::wstring path);
};

#endif  IMGRENAMEDLG
