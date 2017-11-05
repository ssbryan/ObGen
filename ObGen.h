
// ObGen.h : main header file for the PROJECT_NAME application
//

#pragma once

#include "stdafx.h"
#ifndef __AFXWIN_H__
    #error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// ObGenApp:
// See ObGen.cpp for the implementation of this class
//

class ObGenApp : public CWinApp
{
public:
    ObGenApp();

// Overrides
public:
    virtual BOOL InitInstance();

// Implementation

    DECLARE_MESSAGE_MAP()
};

extern ObGenApp theApp;