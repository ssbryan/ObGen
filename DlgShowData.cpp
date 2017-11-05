// DlgShowData.cpp : implementation file
//

#include "ObGen.h"
#include "DlgShowData.h"
#include <assert.h>


// DlgShowData dialog

IMPLEMENT_DYNAMIC(DlgShowData, CDialogEx)

DlgShowData::DlgShowData(CWnd* pParent /*=NULL*/)
    : CDialogEx(IDD_SHOW_DATA, pParent)
{
}

DlgShowData::~DlgShowData()
{
}

void DlgShowData::SetString(CString* str)
{
    mData = *str;
}

void DlgShowData::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(DlgShowData, CDialogEx)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
END_MESSAGE_MAP()

// DlgShowData message handlers
BOOL DlgShowData::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    CWnd* ctrl = GetDlgItem(IDC_TEXT);
    assert(ctrl);
    ctrl->SetWindowText((LPCTSTR)mData);

    return TRUE;  // return TRUE  unless you set the focus to a control
}


void DlgShowData::OnBnClickedOk()
{
    CDialogEx::OnOK();
}
