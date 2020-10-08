
// ObjInput.h : header file
//

#pragma once

// to get M_PI
#define _USE_MATH_DEFINES 

#include <string>
#include <vector>
#include <cmath>

struct Val3D
{
    Val3D(double x, double y, double z)
        : mX(x)
        , mY(y)
        , mZ(z)
    {}
    
    double mX;
    double mY;
    double mZ;
};

enum MassDist
{
    kConstMD = 0,
    kUniformMD = 1,
    kUniformLowMD = 2,
    kUniformLowerMD = 3,
    kExponentialMD = 4
};

enum MomDist
{
    kUniformMom = 0,
    kExponentialMom = 3
};

enum VolShape
{
    kSpherical = 0,
    kRectilinear = 1,
    kCylindrical = 2,
};

enum VolDist
{
    kUniformVol = 0,
    kUniformLowVol = 1,
    kUniformLowerVol = 2,
    kExponentialVol = 3
};

enum OutFormat
{
    kTipsy = 0,
    kSimple = 1
};

enum PartType
{
    kGeneric,
    kSPH,
    kDark,
    kStar
};

enum AngMomType
{
    kNoAngMom,
    kCalcMom,
    kCustomAngMom
};

struct Object
{
    Object(PartType type, double mass, double dist, double mdist, 
           double x, double y, double z, double radius, 
           int xflag, int xflag2, int yflag, int yflag2, int zflag, int zflag2)
    {
        mType = type;
        mMass = mass;
        mX = x;
        mY = y;
        mZ = z;
        mRadius = radius;
        mDist = dist;
        mMassDist = mdist;
        SetFlags(xflag, xflag2, yflag, yflag2, zflag, zflag2);
        SetVelocity(0, 0, 0);
    };

    void SetVelocity(double vx, double vy, double vz)
    {
        if (std::isnan(vx) || std::isnan(vy) || std::isnan(vz))
        {
            return;
        }

        mVx = vx;
        mVy = vy;
        mVz = vz;
    };

    void SetFlags(int xflag, int xflag2, int yflag, int yflag2, int zflag, int zflag2)
    {
        mXflag = xflag;
        mXflag2 = xflag2;
        mYflag = yflag;
        mYflag2 = yflag2;
        mZflag = zflag;
        mZflag2 = zflag2;
    }

    PartType mType;
    double  mDist;
    double  mMassDist;
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

// Read parameters from file
class ObjInput
{
public:
    ObjInput(std::string& inf, std::string& outf);
    bool    ReadInput();
    int     CreateObjectsAndWrite(void);
    bool    Validate() const;
    bool    SetUpData(void);
    bool    CreateObjects(void);
    double  GetMass(double min, double max, MassDist dist);
    double  GetLoc(double locmax, int dist);
    void    CalcFlags(double x, double y, double z, int& xflag, int& xflag2, int& yflag, int& yflag2, int& zflag, int& zflag2);
    bool    AdjustLocForOverlap(double& x, double& y, double& z, double radius, int& xflag, int& xflag2, int& yflag, int& yflag2, int& zflag, int& zflag2);
    Val3D   GetAMVel(double mass, double x, double y, double z, double dist);
//    double  GetVP(double mass, double min, double max, int dist);
    bool    WriteData(void);

private:
    bool    mInputOK;
    AngMomType mDoAngMom;
    OutFormat mFormat;
    long long mNumObjects;
    long long mNumSPH;
    long long mNumDark;
    long long mNumStar;

    // Tipsy parameters    
    double    mRho;  // gas density 
                          //  0.216 testonestar, 
                          //  1.0 testcollapse, 
                          //  1.0 testdamping, 
                          //  1.0 testndsph/accel & density, 
                          //  3.45257 testshock
    double    mGasTemp; // gas temperature 
                          //  5134 testonestar, 
                          //  0.05 testcollapse, 
                          //  0.05 testdamping, 
                          //  1.0 testndsph/accel & density, 
                          //  115.86 testshock
    double    mEps; // gravitational softening 
                          //  0.00125 testonestar, 
                          //  0.01 testcollapse, 
                          //  0.01 testdamping, 
                          //  200.0 testndsph/accel & density, 
                          //  0.0027027 testshock
    double    mMetals; // metallicity 
                          //  0 testonestar, 
                          //  0 testcollapse, 
                          //  0 testdamping, 
                          //  0 testndsph/accel & density, 
                          // -1.084e-19 testshock

    // mass
    double  mMassMin;
    double  mMassMax;
    MassDist mMassDist;

    // velocity/momentum
//    double  mVMMin;
//    double  mVMMax;
//    int     mVMDist;
    
    // angular momentum axis
    double  mAMx;
    double  mAMy;
    double  mAMz;
    double  mDiskPressure;
    double  mAMRratio;

    // orbit eccentricity limit, random within limits (0 to mEcc)
    // objects are at random points in their orbits
    // CoM is presumed to be at 0,0,0
    double  mEcc;

    // environment
    VolShape mShape;
    double  mVolDim1;
    double  mVolDim2;
    double  mVolDim3;
    int     mLocationDist;
    double  mLambda;

    std::string mInFName;
    std::string mOutFName;
    bool    mDataChanged;
    std::vector<Object>    mObjects;
};
