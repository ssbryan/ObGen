// viewText.cpp : implementation file
//

#include "stdafx.h"
#include "ObGen.h"
#include "viewText.h"


// viewText dialog

IMPLEMENT_DYNAMIC(viewText, CDialogEx)

viewText::viewText(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SHOW_DATA, pParent)
{

}

viewText::~viewText()
{
}

void viewText::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT_DATA, mCtrl);
}


BEGIN_MESSAGE_MAP(viewText, CDialogEx)
END_MESSAGE_MAP()


// viewText message handlers
