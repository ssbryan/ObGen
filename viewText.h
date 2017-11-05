#pragma once
#include "stdafx.h"
#include "afxdialogex.h"
#include "afxcmn.h"


// viewText dialog

class viewText : public CDialogEx
{
	DECLARE_DYNAMIC(viewText)

public:
	viewText(CWnd* pParent = NULL);   // standard constructor
	virtual ~viewText();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHOW_DATA};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CRichEditCtrl mCtrl;
};
