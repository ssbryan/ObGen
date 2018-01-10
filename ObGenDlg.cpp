
// ObGenDlg.cpp : implementation file
//

#include "ObGen.h"
#include "ObGenDlg.h"
#include "DlgShowData.h"

#include <map>
#include <set>
#include <random>
#include <fstream>
#include <io.h>
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const double G = 6.67408e-11;

double GetUniformRandomDouble(double rmin, double rmax);
double GetExponentialRandomDouble(double rmax);

// count of initially overlapping locs
static int hasOverlap = 0;

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

// ObGenDlg dialog

ObGenDlg::ObGenDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(IDD_OBGEN_DIALOG, pParent)
    , mNumObjects(100)
    , mMassMin(1)
    , mMassMax(1e6)
    , mMassDist(0)
    , mVMMin(0.1)
    , mVMMax(1.0)
    , mVMDist(0)
    , mVolumeRad(1e5)
    , mLocationDist(0)
    , mLambda(0)
    , mDataChanged(true)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void ObGenDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ObGenDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_BN_CLICKED(IDC_VIEW, OnBnClickedView)
    ON_BN_CLICKED(IDC_SIMULATE, OnBnClickedSimulate)
    ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
//    ON_BN_CLICKED(IDC_UNIFORM_DIST, OnBnClickedUniformDist)
//    ON_BN_CLICKED(IDC_EXP_DIST, OnBnClickedExpDist)
    ON_EN_CHANGE(IDC_NUM_OBJS, OnEnChangeNumObjs)
    ON_EN_CHANGE(IDC_MIN_MASS, OnEnChangeMinMass)
    ON_EN_CHANGE(IDC_MAX_MASS, OnEnChangeMaxMass)
    ON_CBN_SELCHANGE(IDC_COMBO_MASS, OnCbnSelchangeComboMass)
    ON_EN_CHANGE(IDC_MIN_VEL_MOM, OnEnChangeMinVelMom)
    ON_EN_CHANGE(IDC_MAX_VEL_MOM, OnEnChangeMaxVelMom)
    ON_CBN_SELCHANGE(IDC_COMBO_VM, OnCbnSelchangeComboVm)
    ON_EN_CHANGE(IDC_VOL_RADIUS, OnEnChangeVolRadius)
    ON_EN_CHANGE(IDC_OPT_MOMTOL, OnEnChangeOptMomtol)
    ON_EN_CHANGE(IDC_OPT_DVNORMTOL, OnEnChangeOptDvnormtol)
    ON_EN_CHANGE(IDC_OPT_RUNS, OnEnChangeOptRuns)
    ON_EN_CHANGE(IDC_OPT_TSTEP, OnEnChangeOptTstep)
    ON_EN_CHANGE(IDC_OPT_MINVEL, OnEnChangeOptMinvel)
    ON_BN_CLICKED(IDC_FIND_SIM, OnBnClickedFindSim)
    ON_CBN_SELCHANGE(IDC_LOC_DIST, OnCbnSelchangeLocDist)
END_MESSAGE_MAP()


// ObGenDlg message handlers

BOOL ObGenDlg::OnInitDialog()
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

    GetDlgItem(IDC_NUM_OBJS)->SetWindowText(CString("1000"));
    GetDlgItem(IDC_VOL_RADIUS)->SetWindowText(CString("1.0e5"));
    GetDlgItem(IDC_MIN_MASS)->SetWindowText(CString("100.0"));
    GetDlgItem(IDC_MAX_MASS)->SetWindowText(CString("1.0e6"));
    GetDlgItem(IDC_MIN_VEL_MOM)->SetWindowText(CString("1.0"));
    GetDlgItem(IDC_MAX_VEL_MOM)->SetWindowText(CString("1.0e6"));
    GetDlgItem(IDC_OPT_MOMTOL)->SetWindowText(CString("1.0e-4"));
    GetDlgItem(IDC_OPT_DVNORMTOL)->SetWindowText(CString("1.0e-5"));
    GetDlgItem(IDC_OPT_RUNS)->SetWindowText(CString("1.0e6"));
    GetDlgItem(IDC_OPT_TSTEP)->SetWindowText(CString("1"));
    GetDlgItem(IDC_OPT_MINVEL)->SetWindowText(CString("1.0e-2"));
    GetDlgItem(IDC_LOC_LAMBDA)->SetWindowText(CString("5000"));

    // set up mass distribution combo
    CComboBox* mcombo = (CComboBox*)GetDlgItem(IDC_COMBO_MASS);
    assert(mcombo);
    mcombo->AddString(CString("Uniform"));
    mcombo->AddString(CString("Uniform-weighted low"));
    mcombo->AddString(CString("Uniform-weighted lower"));
    mcombo->AddString(CString("Exponential"));
    mcombo->SetCurSel(0);

    // set up velocity/momentum distribution combo
    CComboBox* vmcombo = (CComboBox*)GetDlgItem(IDC_COMBO_VM);
    assert(vmcombo);
    vmcombo->AddString(CString("Uniform (momentum)"));
    vmcombo->AddString(CString("Uniform (velocity)"));
    vmcombo->AddString(CString("Exponential (momentum)"));
    vmcombo->AddString(CString("Exponential (velocity)"));
    vmcombo->SetCurSel(0);

    // set up location distribution combo
    CComboBox* locombo = (CComboBox*)GetDlgItem(IDC_LOC_DIST);
    assert(locombo);
    locombo->AddString(CString("Uniform"));
    locombo->AddString(CString("Uniform-weighted low"));
    locombo->AddString(CString("Uniform-weighted lower"));
    locombo->AddString(CString("Exponential"));
    locombo->SetCurSel(0);

    // set up location distribution radio buttons
//    CButton* mdist = (CButton*)GetDlgItem(IDC_UNIFORM_DIST);
//    mdist->SetCheck(TRUE);
    mLocationDist = 0;
//    GetDlgItem(IDC_LOC_LAMBDA)->EnableWindow(FALSE);
//    CButton* mdiste = (CButton*)GetDlgItem(IDC_EXP_DIST);
//    mdiste->SetCheck(FALSE);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void ObGenDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void ObGenDlg::OnPaint()
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
HCURSOR ObGenDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

void ObGenDlg::OnBnClickedOk()
{
    // bring up file save dialog
    // if saved, enable buttons
    TCHAR filter[] = _T("Initial Data Files (*.data)|*.data||");
    TCHAR ext[] = _T(".data");
    CFileDialog dlg(FALSE, ext, 0, OFN_OVERWRITEPROMPT, filter, 0, 0, TRUE);

    if (dlg.DoModal() == IDOK)
    {
        GetDlgItem(IDC_VIEW)->EnableWindow(TRUE);
        GetDlgItem(IDC_SIMULATE)->EnableWindow(TRUE);

        // set up data and save to file
        CString fname = (dlg.GetPathName());
        CString fnameExt = (dlg.GetFileExt());

        if (WriteData(fname, fnameExt))
        {
            // save filename
            mFName = fname;
        }
    }
}

void ObGenDlg::OnBnClickedView()
{
    // create string with analytics
    CString str;
    CreateAnalyticString(str);
    ShowData(str);
}


void ObGenDlg::OnBnClickedSimulate()
{
    // check whether data was recently saved
    // if so, launch simulation with saved fname
    // otherwise, call OnBnClickedOk and then (if okay) simulate
    // in SetupData, clear fname
    // fname only set in OnBnClickedOk
    CString sim;
    GetDlgItem(IDC_SIM_PATH)->GetWindowText(sim);
    wchar_t exepath[256];
    GetShortPathName(sim, exepath, 256);
    CT2CA aconvertedAnsiString(exepath);

    if (_access(aconvertedAnsiString, 0) == 0)
    {
        // execute with data file
        wchar_t datapath[256];
        GetShortPathName(mFName, datapath, 256);
        CT2CA aconvertedData(datapath);

        if (_execl(aconvertedAnsiString, aconvertedAnsiString, aconvertedData, 0) == -1)
        {
            // error in executing
            CString simerr("Could not execute: ");
            simerr += sim;
            simerr += " ";
            simerr += mFName;
            AfxMessageBox(simerr, IDOK);
        }
    }
}


void ObGenDlg::OnBnClickedCancel()
{
    CDialogEx::OnCancel();
}

bool ObGenDlg::Validate() const
{
    if (mNumObjects < 2)
    {
        return false;
    }

    if (mVolumeRad <= 0)
    {
        return false;
    }

    if (mMassMin <= 0)
    {
        return false;
    }

    if (mMassMax <= mMassMin)
    {
        return false;
    }

    //if ((mLocationDist == 1) && (mLambda <= 0))
    //{
    //    return false;
    //}

    return true;
}

bool ObGenDlg::SetUpData(void)
{
    CString str;
    bool okay = (GetDlgItemText(IDC_NUM_OBJS, str) != 0);
    CString dformat("%d");
    CString format("%f");
    int dnum = 0;
    float num = 0;
    okay = okay && (swscanf_s((LPCWSTR)str, (LPCWSTR)dformat, &dnum) == 1);

    if (okay)
    {
        mNumObjects = dnum;
    }

    // get mass min/max
    okay = (GetDlgItemText(IDC_MIN_MASS, str) != 0);
    num = 0;
    okay = okay && (swscanf_s((LPCWSTR)str, (LPCWSTR)format, &num) == 1);

    if (okay)
    {
        mMassMin = num;
    }

    okay = (GetDlgItemText(IDC_MAX_MASS, str) != 0);
    num = 0;
    okay = okay && (swscanf_s((LPCWSTR)str, (LPCWSTR)format, &num) == 1);

    if (okay)
    {
        mMassMax = num;
    }

    // get velocity/momentum min/max
    okay = (GetDlgItemText(IDC_MIN_VEL_MOM, str) != 0);
    num = 0;
    okay = okay && (swscanf_s((LPCWSTR)str, (LPCWSTR)format, &num) == 1);

    if (okay)
    {
        mVMMin = num;
    }

    okay = (GetDlgItemText(IDC_MAX_VEL_MOM, str) != 0);
    num = 0;
    okay = okay && (swscanf_s((LPCWSTR)str, (LPCWSTR)format, &num) == 1);

    if (okay)
    {
        mVMMax = num;
    }

    // get spatial extent
    okay = (GetDlgItemText(IDC_VOL_RADIUS, str) != 0);
    num = 0;
    okay = okay && (swscanf_s((LPCWSTR)str, (LPCWSTR)format, &num) == 1);

    if (okay)
    {
        mVolumeRad = num;
    }

    okay = (GetDlgItemText(IDC_LOC_LAMBDA, str) != 0);
    num = 0;
    okay = okay && (swscanf_s((LPCWSTR)str, (LPCWSTR)format, &num) == 1);

    if (okay)
    {
        mLambda = num;
    }

    return true;
}

bool ObGenDlg::CreateObjects(void)
{
    // create objects in a tmp list
    // sort based on mass, with higher mass objects first
    // scale momentum from origin
    hasOverlap = 0;

    if (!Validate())
    {
        return false;
    }

    mObjects.clear();

    CComboBox* mcombo = (CComboBox*)GetDlgItem(IDC_COMBO_MASS);
    mMassDist = mcombo->GetCurSel();

    CComboBox* vmcombo = (CComboBox*)GetDlgItem(IDC_COMBO_VM);
    mVMDist = vmcombo->GetCurSel();

    CComboBox* loccombo = (CComboBox*)GetDlgItem(IDC_LOC_DIST);
    mLocationDist = loccombo->GetCurSel();

    for (int i = 0; i < mNumObjects; ++i)
    {
        double m = GetMass(mMassMin, mMassMax, mMassDist);
        double x = GetLoc(mVolumeRad, mLocationDist);
        double y = GetLoc(mVolumeRad, mLocationDist);
        double z = GetLoc(mVolumeRad, mLocationDist);
        int xflag = 0;
        int yflag = 0;
        int zflag = 0;
        int xflag2 = 0;
        int yflag2 = 0;
        int zflag2 = 0;
        CalcFlags(x, y, z, xflag, xflag2, yflag, yflag2, zflag, zflag2);

        // from Object.cpp in asteroid sln
        // determine radius: rho = M / V, V = M / rho

        // rho = 3g/cc plus porosity discount of 50% (for now - see ObjData.cpp in ObjDatad project)
        // rho = 3g/cc plus porosity discount of 50%, plus random element that increases 
        double rho0 = 3000; // 3g/cc (* 1e-3kg/g * 1e6cc/m^3), similar to carbonaceous chondrites
        double porosity = 0.5; // 50% porosity, changes effective rho

                               // should do an "if" to see if a random element is needed (solid rock, not rubble), 
                               // or if we want a nickel-iron chunk at 7.9g/cc (3.84% of meteorites)
        double rho = rho0 * porosity;

        // volume
        double v = m / rho;

        // R = (3 * V / (4 * pi))^(1/3)
        double radius = pow(3 * v / (4 * M_PI), 0.33333333);

        AdjustLocForOverlap(x, y, z, radius, xflag, xflag2, yflag, yflag2, zflag, zflag2);

        double vx = GetVP(m, mVMMin, mVMMax, mVMDist);
        double vy = GetVP(m, mVMMin, mVMMax, mVMDist);
        double vz = GetVP(m, mVMMin, mVMMax, mVMDist);
        mObjects.push_back(Object(m, x, y, z, vx, vy, vz, radius, xflag, xflag2, yflag, yflag2, zflag, zflag2));
    }

    // mark so we can reuse Objects
    mDataChanged = false;
    return true;
}

//void ObGenDlg::OnBnClickedUniformDist()
//{
//    mDataChanged = true;
//    mLocationDist = 0;
//    GetDlgItem(IDC_LOC_LAMBDA)->EnableWindow(FALSE);
//}
//
//void ObGenDlg::OnBnClickedExpDist()
//{
//    mDataChanged = true;
//    mLocationDist = 1;
//    GetDlgItem(IDC_LOC_LAMBDA)->EnableWindow(TRUE);
//}

double ObGenDlg::GetMass(double mmin, double mmax, int dist)
{
    double m = (mmax - mmin) / 2;

    switch (dist)
    {
    case 0: // uniform
        m = GetUniformRandomDouble(mmin, mmax);
        break;
    case 1: // Uniform-weighted low
        {
            double mnrt = std::sqrt(mmin);
            double mxrt = std::sqrt(mmax);
            m = GetUniformRandomDouble(mnrt, mxrt);
            m *= m;
        }
        break;
    case 2:  // Uniform-weighted lower
        {
            double mnrt = std::sqrt(mmin);
            double mxrt = std::sqrt(mmax);
            double mnrt4 = std::sqrt(mnrt);
            double mxrt4 = std::sqrt(mxrt);
            m = GetUniformRandomDouble(mnrt4, mxrt4);
            m *= m;
            m *= m;
        }
        break;
    case 3:  // Exponential
        {
            double lambda = mmax / 3;  // heuristic
            m = GetExponentialRandomDouble(lambda);
        }
        break;
    default:
        assert(!"bad case");
    }

    return m;
}

double ObGenDlg::GetLoc(double locmax, int dist)
{
    // sign randomized before return
    double d = locmax;

    switch (dist)
    {
    case 0:
    {
        // uniform over entire space
        d = GetUniformRandomDouble(0, locmax);
        break;
    }
    case 1: // Uniform-weighted low
    {
        double mxrt = std::sqrt(locmax);
        d = GetUniformRandomDouble(0, mxrt);
        d *= d;
        break;
    }
    case 2:  // Uniform-weighted lower
    {
        double mxrt = std::sqrt(locmax);
        double mxrt4 = std::sqrt(mxrt);
        d = GetUniformRandomDouble(0, mxrt4);
        d *= d;
        d *= d;
        break;
    }
    case 3:
    {
        // this gives the magnitude but not the sign
        double val = mLambda;

        if (val <= 0)
        {
            val = locmax / 3;
        }

        d = GetExponentialRandomDouble(val);
        break;
    }
    }

    if (rand() > RAND_MAX / 2)
    {
        d = -d;
    }

    return d;
}

double ObGenDlg::GetVP(double mass, double vpmin, double vpmax, int dist)
{
    double v = (vpmax - vpmin) / 2;

    switch (dist)
    {
    case 0:  // Uniform (momentum)
        v = GetUniformRandomDouble(vpmin, vpmax);
        v /= mass;
        break;
    case 1:  // Uniform (velocity)
        v = GetUniformRandomDouble(vpmin, vpmax);
        break;
    case 2:  // Exponential (momentum)
        v = GetExponentialRandomDouble(vpmax / 3);
        v /= mass;
        break;
    case 3:  // Exponential (velocity)
        v = GetExponentialRandomDouble(vpmax / 3);
        break;
    default:
        assert(!"bad case");
    }

    return v;
}

double GetUniformRandomDouble(double rmin, double rmax)
{
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(rmin, rmax);
    return dis(gen);
}

double GetExponentialRandomDouble(double lambda)
{
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::exponential_distribution<> d(lambda);
    return d(gen);
}

void ObGenDlg::OnEnChangeNumObjs()
{
    mDataChanged = true;
}

void ObGenDlg::OnEnChangeMinMass()
{
    mDataChanged = true;
}

void ObGenDlg::OnEnChangeMaxMass()
{
    mDataChanged = true;
}

void ObGenDlg::OnCbnSelchangeComboMass()
{
    mDataChanged = true;
}

void ObGenDlg::OnEnChangeMinVelMom()
{
    mDataChanged = true;
}

void ObGenDlg::OnEnChangeMaxVelMom()
{
    mDataChanged = true;
}

void ObGenDlg::OnCbnSelchangeComboVm()
{
    mDataChanged = true;
}

void ObGenDlg::OnEnChangeVolRadius()
{
    mDataChanged = true;
}

void ObGenDlg::OnEnChangeOptMomtol()
{
    mDataChanged = true;
}

void ObGenDlg::OnEnChangeOptDvnormtol()
{
    mDataChanged = true;
}

void ObGenDlg::OnEnChangeOptRuns()
{
    mDataChanged = true;
}

void ObGenDlg::OnEnChangeOptTstep()
{
    mDataChanged = true;
}

void ObGenDlg::OnEnChangeOptMinvel()
{
    mDataChanged = true;
}

void ObGenDlg::ShowData(CString& str)
{
    // pop up dlg with one large rich text edit control, 
    // analytics at top, then options and data

    DlgShowData dlg(this);
    dlg.SetString(&str);
    dlg.DoModal();
}

bool ObGenDlg::CreateAnalyticString(CString& str)
{
    bool isOkay = true;
    // kinetic and potential energy
    double ke = 0;
    double pe = 0;

    if (mDataChanged)
    {
        // get user input and create objects
        isOkay = SetUpData();
        isOkay &= CreateObjects();
    }

    if (!isOkay)
    {
        return isOkay;
    }

    // get statistics for loc, vel
    double avgx = 0.0;
    double avgy = 0.0;
    double avgz = 0.0;
    double avgvx = 0.0;
    double avgvy = 0.0;
    double avgvz = 0.0;

    CString analyticstr;

    // traverse Object vector and create CString
    int n = mObjects.size();
    std::map<int, std::set<int> > peTraversal;

    for (int i = 0; i < n; ++i)
    {
        Object& obj = mObjects[i];
        avgx += obj.mX;
        avgy += obj.mY;
        avgz += obj.mZ;
        avgvx += obj.mVx;
        avgvy += obj.mVy;
        avgvz += obj.mVz;
#if 0
        ke += (obj.mVx * obj.mVx + obj.mVy * obj.mVy + obj.mVz * obj.mVz) * obj.mMass / 2;
        std::set<int>& traversed = peTraversal[i];

        for (int j = 0; j < n; ++j)
        {
            if (j == i)
            {
                continue;
            }

            Object& obj1 = mObjects[j];
            std::set<int>::iterator oIt = traversed.find(j);

            if (oIt != traversed.end())
            {
                continue;
            }

            traversed.insert(j);

            // Gm0m1/r
            double d = std::sqrt((obj.mX - obj1.mX) * (obj.mX - obj1.mX) + (obj.mY - obj1.mY) * (obj.mY - obj1.mY) + (obj.mZ - obj1.mZ) * (obj.mZ - obj1.mZ));
            pe += G * obj.mMass * obj1.mMass / d;
        }
#endif
    }

    avgx /= n;
    avgy /= n;
    avgz /= n;
    avgvx /= n;
    avgvy /= n;
    avgvz /= n;

    // analyze input data to get
    // --total momentum
    // --total angular momentum
    // --CoM compared to 0,0,0
    // --total kinetic energy

    // first, get CoM and total momentum
    double xc = 0.0;
    double yc = 0.0;
    double zc = 0.0;
    double totmass = 0.0;
    double totmomx = 0.0;
    double totmomy = 0.0;
    double totmomz = 0.0;

    for (int i = 0; i < n; i++)
    {
        Object& obj = mObjects[i];
        double m = obj.mMass;
        xc += m * obj.mX;
        yc += m * obj.mY;
        zc += m * obj.mZ;
        totmass += m;

        totmomx += m * obj.mVx;
        totmomy += m * obj.mVy;
        totmomz += m * obj.mVz;
    }

    xc /= totmass;
    yc /= totmass;
    zc /= totmass;

    // now get angular momentum
    double angmomx = 0.0;
    double angmomy = 0.0;
    double angmomz = 0.0;

    for (int i = 0; i < n; i++)
    {
        Object& obj = mObjects[i];
        double dist2 = (obj.mX - xc) * (obj.mX - xc) + (obj.mY - yc) * (obj.mY - yc) + (obj.mZ - zc) * (obj.mZ - zc);
        double dist = std::sqrt(dist2);
        double dot = obj.mVx * obj.mX + obj.mVy * obj.mY + obj.mVz * obj.mZ;
        angmomx += obj.mMass * (obj.mVx * dist - (obj.mVx - xc) * dot / dist);
        angmomy += obj.mMass * (obj.mVy * dist - (obj.mVy - xc) * dot / dist);
        angmomz += obj.mMass * (obj.mVz * dist - (obj.mVz - xc) * dot / dist);
    }

    CString mdist;

    switch (mMassDist)
    {
    case 0:
        mdist = "Uniform";
        break;
    case 1:
        mdist = "Uniform weighted low";
        break;
    case 2:
        mdist = "Uniform weighted lower";
        break;
    case 3:
        mdist = "Exponential";
        break;
    }

    CString vmdist;

    switch (mVMDist)
    {
    case 0:
        vmdist = "Uniform (momentum)";
        break;
    case 1:
        vmdist = "Uniform (velocity)";
        break;
    case 2:
        vmdist = "Exponential (momentum)";
        break;
    case 3:
        vmdist = "Exponential (velocity)";
        break;
    }

    CString locdist;

    switch (mLocationDist)
    {
    case 0:
        locdist = "Uniform";
        break;
    case 1:
        mdist = "Uniform weighted low";
        break;
    case 2:
        mdist = "Uniform weighted lower";
        break;
    case 3:
        locdist = "Exponential";
        break;
    }

    analyticstr.Format(_T("Number of objects: %d, Mass: min %g, max %g, distribution %s, Velocity/Momentum: min %g, max %g, distribution %s, Radius: %g, distribution %s, lambda %g\r\nAverage location: %g, %g, %g\r\nAverage velocity: %g, %g, %g\r\n Total mass: %g\r\n Average mass: %g\r\nCenter of Mass: %g, %g, %g\r\nMomentum: %g, %g, %g\r\nAng Momentum: %g, %g, %g\r\nOverlaps: %d"),
        n, mMassMin, mMassMax, mdist, mVMMin, mVMMax, vmdist, mVolumeRad, locdist, mLambda, avgx, avgy, avgz, avgvx, avgvy, avgvz, totmass, totmass / n, xc, yc, zc, totmomx, totmomy, totmomz, angmomx, angmomy, angmomz, hasOverlap);
#if 0
    analyticstr.Format(_T("Number of objects: %d, Mass: min %g, max %g, distribution %s, Velocity/Momentum: min %g, max %g, distribution %s, Radius: %g, distribution %s, lambda %g\r\nAverage location: %g, %g, %g\r\nAverage velocity: %g, %g, %g\r\n Total mass: %g\r\n Average mass: %g\r\nCenter of Mass: %g, %g, %g\r\nMomentum: %g, %g, %g\r\nAng Momentum: %g, %g, %g\r\nKE: %g, PE: %g"),
        n, mMassMin, mMassMax, mdist, mVMMin, mVMMax, vmdist, mVolumeRad, locdist, mLambda, avgx, avgy, avgz, avgvx, avgvy, avgvz, totmass, totmass / n, xc, yc, zc, totmomx , totmomy, totmomz, angmomx, angmomy, angmomz, ke, pe);
#endif
    str += analyticstr;
    return true;
}

bool ObGenDlg::WriteData(CString& fname, CString& ext)
{
    std::ofstream ofs;
    ofs.open(fname.GetString(), std::ofstream::out);
    bool isOkay = true;

    if (mDataChanged)
    {
        // get user input and create objects
        isOkay = SetUpData();
        isOkay &= CreateObjects();
    }

    if (!isOkay)
    {
        return isOkay;
    }

    CString optionstr;

    // Options
    CString txt;
    GetDlgItem(IDC_OPT_MOMTOL)->GetWindowText(txt);

    if (txt.IsEmpty())
    {
        txt = CString("1.0e-4");
    }

    optionstr += "momtolOpt=";
    optionstr += txt;
    optionstr += ", ";

    GetDlgItem(IDC_OPT_DVNORMTOL)->GetWindowText(txt);

    if (txt.IsEmpty())
    {
        txt = CString("1.0e-5");
    }

    optionstr += "dvnormtolOpt=";
    optionstr += txt;
    optionstr += ", ";

    GetDlgItem(IDC_OPT_RUNS)->GetWindowText(txt);

    if (txt.IsEmpty())
    {
        txt = CString("1.0e6");
    }

    optionstr += "runs=";
    optionstr += txt;
    optionstr += ", ";

    GetDlgItem(IDC_OPT_TSTEP)->GetWindowText(txt);

    if (txt.IsEmpty())
    {
        txt = CString("1");
    }

    optionstr += "tstep=";
    optionstr += txt;
    optionstr += ", ";

    GetDlgItem(IDC_OPT_MINVEL)->GetWindowText(txt);

    if (txt.IsEmpty())
    {
        txt = CString("1.0e-2");
    }

    optionstr += "minvforcheck=";
    optionstr += txt;

    CT2CA convertedAnsiString(optionstr);
    std::string opts(convertedAnsiString);
    ofs << "#Options: " << opts << std::endl;

    // traverse Object vector and create CString
    int n = mObjects.size();

    for (int i = 0; i < n; ++i)
    {
        Object& obj = mObjects[i];
        ofs << obj.mX << " " 
            << obj.mY << " "
            << obj.mZ << " " 
            << obj.mVx << " " 
            << obj.mVy << " " 
            << obj.mVz << " " 
            << obj.mMass << std::endl;
    }

    ofs.close();

    CString astr;
    CreateAnalyticString(astr);
    std::ofstream ofsa;
    CString aname = fname;

    if (!ext.IsEmpty())
    {
        aname = fname.Left(fname.GetLength() - ext.GetLength());
    }

    aname += CString("stat");
    ofsa.open(aname.GetString(), std::ofstream::out);
    CT2CA aconvertedAnsiString(astr);
    std::string stats(aconvertedAnsiString);
    ofsa << stats << std::endl;
    ofsa.close();

    return true;
}

void ObGenDlg::OnBnClickedFindSim()
{
    // bring up file open dialog
    // if okay, set path in IDC_SIM_PATH
    TCHAR filter[] = _T("Executable Files (*.exe)|*.exe||");
    TCHAR ext[] = _T(".exe");
    CFileDialog dlg(TRUE, ext, 0, OFN_FILEMUSTEXIST, filter, 0, 0, TRUE);

    if (dlg.DoModal() == IDOK)
    {
        GetDlgItem(IDC_SIM_PATH)->SetWindowText(dlg.GetPathName());
    }
}


void ObGenDlg::OnCbnSelchangeLocDist()
{
    mDataChanged = true;
}

bool ObGenDlg::AdjustLocForOverlap(double& x, double& y, double& z, double radius, int& xflag, int& xflag2, int& yflag, int& yflag2, int& zflag, int& zflag2)
{
    // see whether there's any overlap with existing objects
    // if so, adjust until there's not
    // use flags initially to test overlap
    int n = mObjects.size();
    int counter = 0;
    bool initialOverlap = false;

    for (int i = 0; i < n; )
    {
        Object& obj = mObjects[i];
        bool overlap = (obj.mXflag & xflag || obj.mXflag2 & xflag2) && (obj.mYflag & yflag || obj.mYflag2 & yflag2) && (obj.mZflag & zflag || obj.mZflag2 & zflag2);

        if (overlap)
        {
            // check whether there's room
            double r2 = (obj.mX - x) * (obj.mX - x) + (obj.mY - y) * (obj.mY - y) + (obj.mZ - z) * (obj.mZ - z);
            double rad2 = (obj.mRadius + radius) * (obj.mRadius + radius);

            if (r2 > rad2)
            {
                // no problem
                ++i;
                continue;
            }
            else
            {
                initialOverlap = true;
                // get new coords and try again
                x = GetLoc(mVolumeRad, mLocationDist);
                y = GetLoc(mVolumeRad, mLocationDist);
                z = GetLoc(mVolumeRad, mLocationDist);
                CalcFlags(x, y, z, xflag, xflag2, yflag, yflag2, zflag, zflag2);

                // start over checking objects
                i = 0;

                if (++counter > 1000)
                {
                    // is the dpace too small?
                    assert(counter < 1000);
                    break;
                }
            }
        }
        else
        {
            counter = 0;
            ++i;
        }
    }

    if (initialOverlap)
    {
        hasOverlap++;
    }

    return counter < 1000;
}

void ObGenDlg::CalcFlags(double x, double y, double z, int& xflag, int& xflag2, int& yflag, int& yflag2, int& zflag, int& zflag2)
{
    // set up loc flags, based on 2 * mVolumeRad, 1 / 32 per bit
    int fullShift = (int)(((x + mVolumeRad) * 16 / mVolumeRad));

    if (fullShift > 31)
    {
        fullShift = 31;
    }

    xflag = 1 << fullShift;

    // also set up loc2 flags, 1/2 a division over from loc flags
    int halfShift = (int)(((x + mVolumeRad) * 16 / mVolumeRad) + (mVolumeRad / 32));

    if (halfShift > 31)
    {
        halfShift = 31;
    }

    xflag2 = 1 << halfShift;

    fullShift = (int)(((y + mVolumeRad) * 16 / mVolumeRad));

    if (fullShift > 31)
    {
        fullShift = 31;
    }

    yflag = 1 << fullShift;

    // also set up loc2 flags, 1/2 a division over from loc flags
    halfShift = (int)(((y + mVolumeRad) * 16 / mVolumeRad) + (mVolumeRad / 32));

    if (halfShift > 31)
    {
        halfShift = 31;
    }

    yflag2 = 1 << halfShift;

    fullShift = (int)(((z + mVolumeRad) * 16 / mVolumeRad));

    if (fullShift > 31)
    {
        fullShift = 31;
    }

    zflag = 1 << fullShift;

    // also set up loc2 flags, 1/2 a division over from loc flags
    halfShift = (int)(((z + mVolumeRad) * 16 / mVolumeRad) + (mVolumeRad / 32));

    if (halfShift > 31)
    {
        halfShift = 31;
    }

    zflag2 = 1 << halfShift;
}
