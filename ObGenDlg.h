
// ObGenDlg.h : header file
//

#pragma once

// to get M_PI
#define _USE_MATH_DEFINES 

#include "stdafx.h"
#include "afxdialogex.h"

#include <string>
#include <vector>
#include <cmath>

struct Object
{
    Object(double mass, double x, double y, double z, double vx, double vy, double vz, double radius, int xflag, int xflag2, int yflag, int yflag2, int zflag, int zflag2)
    {
        mMass = mass;
        mX = x;
        mY = y;
        mZ = z;
        mVx = vx;
        mVy = vy;
        mVz = vz;
        mRadius = radius;
        mXflag = xflag;
        mXflag2 = xflag2;
        mYflag = yflag;
        mYflag2 = yflag2;
        mZflag = zflag;
        mZflag2 = zflag2;
    }

    double  mRadius;
    double  mMass;
    double  mX;
    double  mY;
    double  mZ;
    double  mVx;
    double  mVy;
    double  mVz;
    int     mXflag;
    int     mXflag2;
    int     mYflag;
    int     mYflag2;
    int     mZflag;
    int     mZflag2;
};

// ObGenDlg dialog
class ObGenDlg : public CDialogEx
{
public:
    ObGenDlg(CWnd* pParent = NULL); // standard constructor
    bool    Validate() const;
    bool    SetUpData(void);
    bool    CreateObjects(void);
    double  GetMass(double min, double max, int dist);
    double  GetLoc(double locmax, int dist);
    void    CalcFlags(double x, double y, double z, int& xflag, int& xflag2, int& yflag, int& yflag2, int& zflag, int& zflag2);
    bool    AdjustLocForOverlap(double& x, double& y, double& z, double radius, int& xflag, int& xflag2, int& yflag, int& yflag2, int& zflag, int& zflag2);
    double  GetVP(double mass, double min, double max, int dist);
    bool    WriteData(CString& fname, CString& ext);
    void    ShowData(CString& str);
    bool    CreateAnalyticString(CString& str);

private:
    int     mNumObjects;

    // mass
    double  mMassMin;
    double  mMassMax;
    int     mMassDist;

    // velocity/momentum
    double  mVMMin;
    double  mVMMax;
    int     mVMDist;

    // environment
    double  mVolumeRad;
    int     mLocationDist;
    double  mLambda;

    CString mFName;
    bool    mDataChanged;
    std::vector<Object>	mObjects;

public:
// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_OBGEN_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

public:
//    afx_msg void OnBnClickedUniformDist();
//    afx_msg void OnBnClickedExpDist();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedView();
    afx_msg void OnBnClickedSimulate();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnEnChangeNumObjs();
    afx_msg void OnEnChangeMinMass();
    afx_msg void OnEnChangeMaxMass();
    afx_msg void OnCbnSelchangeComboMass();
    afx_msg void OnEnChangeMinVelMom();
    afx_msg void OnEnChangeMaxVelMom();
    afx_msg void OnCbnSelchangeComboVm();
    afx_msg void OnEnChangeVolRadius();
    afx_msg void OnEnChangeOptMomtol();
    afx_msg void OnEnChangeOptDvnormtol();
    afx_msg void OnEnChangeOptRuns();
    afx_msg void OnEnChangeOptTstep();
    afx_msg void OnEnChangeOptMinvel();
    afx_msg void OnBnClickedFindSim();
    afx_msg void OnCbnSelchangeLocDist();
};
