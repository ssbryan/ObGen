#pragma once


// DlgShowData dialog
#include "stdafx.h"
#include "resource.h"
#include "afxdialogex.h"

class DlgShowData : public CDialogEx
{
    DECLARE_DYNAMIC(DlgShowData)

public:
    DlgShowData(CWnd* pParent = NULL);   // standard constructor
    virtual ~DlgShowData();
    void	SetString(CString* str);

private:
    CString		mData;

public:
// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_SHOW_DATA};
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
};
