
// ObGenDlg.cpp : implementation file
//

#include "ObjInput.h"

#include <map>
#include <set>
#include <random>
#include <fstream>
#define _USE_MATH_DEFINES 
#include <cmath>  
//#include <io.h>
#include <iostream>
#include <sstream>
#include <strings.h>
#include <string.h>
#include "Eigen/Dense"
#include <assert.h>


const double G = 6.67408e-11;
const double kEccMax = 0.5; // max eccentricity

double GetUniformRandomDouble(double rmin, double rmax);
double GetExponentialRandomDouble(double rmax);

// count of initially overlapping locs
int hasOverlap = 0;


ObjInput::ObjInput(std::string& inf, std::string& outf)
: mInFName(inf)
, mOutFName(outf)
, mInputOK(false)
, mDoAngMom(kNoAngMom)
, mFormat(kSimple)
, mNumObjects(0)
, mNumSPH(0)
, mNumDark(0)
, mNumStar(0)
, mRho(1.0)
, mGasTemp(1.0)
, mEps(0.01)
, mMetals(0.0)
, mMassMin(1.0e-6)
, mMassMax(1.0e-5)
, mMassDist(kUniformMD)
, mAMx(0.0)
, mAMy(0.0)
, mAMz(1.0)
, mDiskPressure(0.0)
, mEcc(0.0)
, mShape(kSpherical)
, mVolDim1(1.0)
, mVolDim2(1.0)
, mVolDim3(1.0)
, mLocationDist(kUniformVol)
, mLambda(0.0)
, mDataChanged(false)
, mAMRratio(1)
{
    mInputOK = ReadInput();
}

int ObjInput::CreateObjectsAndWrite(void)
{        
    if (!mInputOK || !Validate())
    {
        return 1;
    }

    bool okay = CreateObjects();

    if (okay)
    {
        WriteData();
    }
    else
    {
        printf("failed to create objects\n");
        return 1;
    }

    return 0;
}

bool ObjInput::Validate() const
{
    if (mNumObjects < 2)
    {
        return false;
    }

    switch (mShape)
    {
        case kSpherical:
            if (mVolDim1 <= 0)
            {
                return false;
            }
            break;
        case kRectilinear:
            break;
            if ((mVolDim1 <= 0) || (mVolDim2 <= 0) || (mVolDim3 <= 0))
            {
                return false;
            }
        case kCylindrical:
            if ((mVolDim1 <= 0) || (mVolDim2 <= 0))
            {
                return false;
            }
            break;
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

bool ObjInput::ReadInput(void)
{
    std::ifstream infile;
    infile.open(mInFName, std::ios::in);

    if (infile.is_open())
    {
        bool okay = true;
        // format: numObjects = [0],
        //   outformat = ["Simple", "Tipsy"], 
        //   numSPH = [0], numDark =  [0], numStar = [0],
        //   rho = [1.0], gastemp = [1.0], eps = [0.01], metals = [0.0],
        //   minMass = [1e-6], maxMass = [1e-5], 
        //       massDist = ["uniform", "uniform low", "uniform lower", "exponential", "const"],
        //   AMX = [0], AMY = [0], AMZ = [1], 
        //   ECC = [0]
        // ###  momMin = [1], momMax = [1e3], momDist = ["uniform"], 
        //   DiskPressure = [0.0],
        //   volShape = ["spherical", "rectilinear", "cylindrical"], vol1 = [1], vol2 = [1], vol3 = [1], 
        //   volDist = ["uniform", "uniform low", "uniform lower", "exponential"], lambda = [0]
        //   angmom = [0, 1, 2], AMRratio = [1]
        //
        // numObjects (int), only applies to "simple" format, not correlated with sph, dark, star for "tipsy"
        // AM* (angular momentum) values are doubles, define an axis, ECC is eccentricity of orbit, 0 to 0.8
        // DiskPressure is the max fraction (0 to 1) of the XY rotational velocity for a random Z velocity
        // ### momDist values: \"uniform\", \"exponential\"
        // volShape values: "spherical" (vol1 is radius, others ignored), 
        //     "rectilinear" (vol1/2/3 are x, y, z mag from center), 
        //     "cylindrical" (vol1 is radius, vol2 is z, vol3 is ignored)
        // angmom is 0 for no angular momentum, or 1 for a calculated value such that each particle is 
        //    rotating about the AM axis, with velocities derated by AMRratio (0 to 1)
 
        char params[512];

        infile.getline(params, 511);
        bool isOkay = infile.good();
        //                bool isEnd = infile.eof();
        char name[40] = {0};
        char val[40] = {0};

        while (isOkay)
        {
            char* opt = name;

            for (char* c = params; ; ++c)
            {
                if ((*c == ' ') || (*c == '\t'))
                {
                    // skip
                    continue;
                }

                if ((*c == ',') || (*c == 0))
                {
                    // finished with option:  set value
                    *opt = 0;
                    char* endptr = 0;

                    if (!strcasecmp(name, "numObjects"))
                    {
                        mNumObjects = (long long)strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "OutFormat"))
                    {
                        if (!strcasecmp(val, "Tipsy"))
                        {
                            mFormat = kTipsy;
                        }
                        else if (!strcasecmp(val, "Simple"))
                        {
                            mFormat = kSimple;
                        }
                    }
                    else if (!strcasecmp(name, "numSPH"))
                    {
                        mNumSPH = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "numDark"))
                    {
                        mNumDark = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "numStar"))
                    {
                        mNumStar = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "rho"))
                    {
                        mRho = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "gastemp"))
                    {
                        mGasTemp = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "eps"))
                    {
                        mEps = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "metals"))
                    {
                        mMetals = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "minMass"))
                    {
                        mMassMin = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "maxMass"))
                    {
                        mMassMax = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "massDist"))
                    {
                        if (!strcasecmp(val, "uniform"))
                        {
                            mMassDist = kUniformMD;
                        }
                        else if (!strcasecmp(val, "uniform low"))
                        {
                            mMassDist = kUniformLowMD;
                        }
                        else if (!strcasecmp(val, "uniform lower"))
                        {
                            mMassDist = kUniformLowerMD;
                        }
                        else if (!strcasecmp(val, "exponential"))
                        {
                            mMassDist = kExponentialMD;
                        }
                        else if (!strcasecmp(val, "const"))
                        {
                            mMassDist = kConstMD;
                        }
                    }
                    //                            else if (!strcasecmp(name, "momMin"))
                    //                            {
                    //                                mVMMin = strtod(val, &endptr);
                    //                            }
                    //                            else if (!strcasecmp(name, "momMax"))
                    //                            {
                    //                                mVMMax = strtod(val, &endptr);
                    //                            }
                    //                            else if (!strcasecmp(name, "momDist"))
                    //                            {
                    //                                                if (!strcasecmp(val, "uniform"))
                    //                                                {
                    //                                                     mVMDist = kUniformMom;
                    //                                                }
                    //                                                else if (!strcasecmp(val, "exponential"))
                    //                                                {
                    //                                                     mVMDist = kExponentialMom;
                    //                                                }
                    //                                                else
                    //                                                {
                    //                                                     mVMDist = kUniformMom;
                    //                                                }
                    //                            }
                    else if (!strcasecmp(name, "AMX"))
                    {
                        mAMx = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "AMY"))
                    {
                        mAMy = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "AMZ"))
                    {
                        mAMz = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "ECC"))
                    {
                        mEcc = strtod(val, &endptr);

                        if (mEcc < 0)
                        {
                            mEcc = 0;
                        }
                        else if (mEcc > kEccMax)
                        {
                            mEcc = kEccMax;
                        }
                    }
                    else if (!strcasecmp(name, "DiskPressure"))
                    {
                        // max fraction of orbiting velocity 
                        // for adding random perpendicular velocity
                        // to generate pressure to keep disk from contracting
                        mDiskPressure = strtod(val, &endptr);

                        if (mDiskPressure < 0)
                        {
                            mDiskPressure = 0;
                        }
                        else if (mDiskPressure > 1)
                        {
                            mDiskPressure = 1;
                        }
                    }
                    else if (!strcasecmp(name, "volShape"))
                    {
                        if (!strcasecmp(val, "spherical"))
                        {
                            mShape = kSpherical;
                        }
                        else if (!strcasecmp(val, "rectilinear"))
                        {
                            mShape = kRectilinear;
                        }
                        else if (!strcasecmp(val, "cylindrical"))
                        {
                            mShape = kCylindrical;
                        }
                    }
                    else if (!strcasecmp(name, "vol1"))
                    {
                        mVolDim1 = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "vol2"))
                    {
                        mVolDim2 = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "vol3"))
                    {
                        mVolDim3 = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "volDist"))
                    {
                        if (!strcasecmp(val, "uniform"))
                        {
                            mLocationDist = kUniformVol;
                        }
                        else if (!strcasecmp(val, "uniform low"))
                        {
                            mLocationDist = kUniformLowVol;
                        }
                        else if (!strcasecmp(val, "uniform lower"))
                        {
                            mLocationDist = kUniformLowerVol;
                        }
                        else if (!strcasecmp(val, "exponential"))
                        {
                            mLocationDist = kExponentialVol;
                        }
                    }
                    else if (!strcasecmp(name, "lambda"))
                    {
                        mLambda = strtod(val, &endptr);
                    }
                    else if (!strcasecmp(name, "angmom"))
                    {
                        int amval = (int)strtod(val, &endptr);

                        switch (amval)
                        {
                            default:
                            case 0:
                                mDoAngMom = kNoAngMom;
                                break;
                            case 1:
                                mDoAngMom = kCalcMom;
                                break;
                            case 2:
                                mDoAngMom = kCustomAngMom;
                                break;
                        }
                    }
                    
                    else if (!strcasecmp(name, "AMRratio"))
                    {
                        mAMRratio = strtod(val, &endptr);

                        if (mAMRratio < 0)
                        {
                            mAMRratio = 0;
                        }
                        else if (mAMRratio > 1)
                        {
                            mAMRratio = 1;
                        }
                    }

                    // and clear bufs
                    name[0] = 0;
                    val[0] = 0;
                    opt = name;

                    if (*c == 0)
                    {
                        // end of line
                        break;
                    }
                }
                else if (*c == '=')
                {
                    *opt = 0;
                    opt = val;
                }
                else
                {
                    *opt++ = *c;
                }
            }

            memset(params, 0, 512);
            infile.getline(params, 511);
            isOkay = infile.good();
            //                        isEnd = infile.eof();
        }
    }

    // check whether we actually have an axis of rotation, or just a point
    if ((mAMx == 0) && (mAMy == 0) && (mAMz == 0))
    {
        mAMz = 1.0;
    }
    else
    {
        // normalize
        double mag = std::sqrt(mAMx * mAMx + mAMy * mAMy + mAMz * mAMz);
        mAMx /= mag;
        mAMy /= mag;
        mAMz /= mag;
    }

    // if Tipsy format, make sure numbers add up
    if (mFormat == kTipsy)
    {
        if (mNumObjects != mNumSPH + mNumDark + mNumStar)
        {
            mNumObjects = mNumSPH + mNumDark + mNumStar;
            printf("Tipsy format: Number of objects must equal sum of SPH, Dark and Star objects, set to %d\n", (int)mNumObjects);
        }
    }

    return true;
}

bool ObjInput::CreateObjects(void)
{
    // create objects in a tmp list
    // sort based on mass, with higher mass objects first
    // scale momentum from origin
    hasOverlap = 0;
    std::map<double, double> massLocs;
    std::map<double, double> massTots;

    for (int i = 0; i < mNumObjects; ++i)
    {
        double m = GetMass(mMassMin, mMassMax, mMassDist);

        double x = 1;
        double y = 1;
        double z = 1;

        if (mShape == kSpherical)
        {
            // use rectilinear coords, but redo any that are outside our sphere
            // need neg values, use rand()
            double rmag = 0;

            do
            {
                x = GetLoc(mVolDim1, mLocationDist);
                y = GetLoc(mVolDim1, mLocationDist);
                z = GetLoc(mVolDim1, mLocationDist);
                rmag = sqrt(x * x + y * y + z * z);
            }        
            while (rmag > mVolDim1);

            if (rand() > RAND_MAX / 2)
            {
                x = -x;
            }

            if (rand() > RAND_MAX / 2)
            {
                y = -y;
            }

            if (rand() > RAND_MAX / 2)
            {
                z = -z;
            }
        }
        else if (mShape == kRectilinear)
        {
            // need neg values, use rand()
            x = GetLoc(mVolDim1, mLocationDist);

            if (rand() > RAND_MAX / 2)
            {
                x = -x;
            }

            y = GetLoc(mVolDim2, mLocationDist);

            if (rand() > RAND_MAX / 2)
            {
                y = -y;
            }

            z = GetLoc(mVolDim3, mLocationDist);

            if (rand() > RAND_MAX / 2)
            {
                z = -z;
            }
        }
        else // cylindrical
        {
            // as for spherical, use rectilinear coords, redo points outside cyl
            double rmag = 0;

            do
            {
                x = GetLoc(mVolDim1, mLocationDist);
                y = GetLoc(mVolDim1, mLocationDist);
                rmag = sqrt(x * x + y * y);
            }        
            while (rmag > mVolDim1);

            // need neg values
            if (rand() > RAND_MAX / 2)
            {
                x = -x;
            }

            if (rand() > RAND_MAX / 2)
            {
                y = -y;
            }

            z = GetLoc(mVolDim2 / 2, mLocationDist);

            if (rand() > RAND_MAX / 2)
            {
                z = -z;
            }
        }

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

        int xflag = 0;
        int yflag = 0;
        int zflag = 0;
        int xflag2 = 0;
        int yflag2 = 0;
        int zflag2 = 0;
        CalcFlags(x, y, z, xflag, xflag2, yflag, yflag2, zflag, zflag2);

        if (1) //(AdjustLocForOverlap(x, y, z, radius, xflag, xflag2, yflag, yflag2, zflag, zflag2))
        {
            //            double vx = GetVP(m, mVMMin, mVMMax, mVMDist);
            //            double vy = GetVP(m, mVMMin, mVMMax, mVMDist);
            //            double vz = GetVP(m, mVMMin, mVMMax, mVMDist);

            PartType type = kGeneric; // for kSimple format

            if (mFormat == kTipsy)
            {
                if (i < mNumSPH)
                {
                    type = kSPH;
                }
                else if (i < mNumSPH + mNumDark)
                {
                    type = kDark;
                }
                else
                {
                    type = kStar;
                }
            }

            // get distance to axis AB from point C, with normal intersecting AB at D
            // Vector AB = (mAMx, mAMy, mAMz) - (0, 0, 0)
            // Vector AC = (x, y, z) - (0, 0, 0)
            // norm(AB) = (mAMx / mag(AB), mAMy / mag(AB), mAMz / mag(AB))  
            // AD = norm(AB) dot AC; 
            // CD = sqrt(AC^2 - AD^2); 
            double magAB2 = (mAMx * mAMx + mAMy * mAMy + mAMz * mAMz);
            double dot = mAMx * x + mAMy * y + mAMz * z;
            double dist = sqrt(x * x + y * y + z * z - dot * dot / magAB2);

            double mdist = std::sqrt(x * x + y * y + z * z);

            mObjects.push_back(Object(type, m, dist, mdist, x, y, z, radius, 
                                      xflag, xflag2, yflag, yflag2, zflag, zflag2));
            massLocs[mdist] = m;
        }
    }

    if (mDoAngMom == kCalcMom)
    {
        // go through the masses and determine the total interior mass for each radius
        double totmass = 0;

        for (std::map<double, double>::iterator mIt = massLocs.begin();
             mIt != massLocs.end();
             ++mIt)
        {
            totmass += mIt->second;
            massTots[mIt->first] = totmass;
        }

        // loop through and determine velocities for the angular momentum
        int num = mObjects.size();

        for (int i = 0; i < num; ++i)
        {
            double m = 1;
            std::map<double, double>::iterator mIt = massTots.find(mObjects[i].mMassDist);

            if (mIt != massTots.end())
            {
                if (mIt == massTots.begin())
                {
                    // object basically at the origin
                    mObjects[i].SetVelocity(0, 0, 0);
                }
                else
                {
                    // we're at the location of our object, so step back one to get interior mass
                    --mIt;
                    m = mIt->second;
                    Val3D v = GetAMVel(m, mObjects[i].mX, mObjects[i].mY, mObjects[i].mZ, mObjects[i].mDist);
                    mObjects[i].SetVelocity(v.mX, v.mY, v.mZ);
                }
            }
            else
            {
                assert(0);
            }
        }
    }

    return true;
}

double ObjInput::GetMass(double mmin, double mmax, MassDist dist)
{
    double m = (mmax - mmin) / 2;

    switch (dist)
    {
        case kConstMD:
            // constant m set to min value
            m = mmin;
        break;
        case kUniformMD: // uniform
            m = GetUniformRandomDouble(mmin, mmax);
        break;
        case kUniformLowMD: // Uniform-weighted low
        {
                double mnrt = std::sqrt(mmin);
                double mxrt = std::sqrt(mmax);
                m = GetUniformRandomDouble(mnrt, mxrt);
                m *= m;
            }
        break;
        case kUniformLowerMD:  // Uniform-weighted lower
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
        case kExponentialMD:  // Exponential
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

double ObjInput::GetLoc(double locmax, int dist)
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
            d *= locmax;
            break;
        }
    }

    return d;
}

// get object's velocity given its location (r is distance to axis)
// and the mass of everything interior to it 
// using parameter values for axis of rotation
Val3D ObjInput::GetAMVel(double mass, double x, double y, double z, double r)
{
    // check whether max eccentricity is 0
    // if not,
    // get random eccentricity (between 0 and kEccMax -> e)
    // get random position in orbit (0 to 2PI -> eccentric anomaly, E)
    // v = sqrt(M*G*((2 / r) - (1 / a)))
    // r = a(1 - e * cos(E))
    // tan(v / 2) = sqrt((1 + e) / (1 - e)) * tan(E / 2)

    // transform axis of rotation to lie on z axis
    // rotate end point around z to x axis
    // rotate around y so it lies on z axis
    // calc vx and vy, add random vz for xformed loc
    // rotate v back (y, then z)

    bool xval = (mAMx != 0.0);
    bool yval = (mAMy != 0.0);

    Val3D v(0, 0, 0);

    if (r == 0.0)
    {
        return v;
    }

    Eigen::Vector3d pin(x, y, z);
    Eigen::Vector3d pout(x, y, z);
    Eigen::Vector3d vout(0, 0, 0);

    double zFrac = GetUniformRandomDouble(0.0, mDiskPressure);

    if (rand() > RAND_MAX / 2)
    {
        zFrac = -zFrac;
    }

    Eigen::Quaterniond qrz(-mAMy, 0, 0, 1);
    qrz.normalize();
    Eigen::Quaterniond qry(std::sqrt(mAMx * mAMx + mAMy * mAMy), 0, 1, 0);
    qry.normalize();

    if (xval || yval)
    {
        // axis not pure Z, so rotate        
        Eigen::Quaterniond pt;
        pt.w() = 0;
        pt.vec() = pin;
        Eigen::Quaterniond rot1 = qrz * pt * qrz.inverse();
        Eigen::Quaterniond rot2 = qry * rot1 * qry.inverse();
        pout = rot2.vec();
    }

    if (mEcc == 0.0)
    {
        // x**2 + y**2 = c**2
        // x = c * cos(t), dx/dt = -c * sin(t) = -y
        // y = c * sin(t), dy/dt = c * cos(t) = x

//        double vel = std::sqrt(mass * G / rmag);
        // Note: G=1 for Changa
        double vel = std::sqrt(mass / r);

        // derate the velocity to the force normal to the axis 
        double norm = r / std::sqrt(r * r + pout(2) * pout(2));
//        double zz = pout(2);

        if (std::isnan(norm))
        {
            return v;
        }

        vout << -(vel * pout(1) * norm / r) , vel * pout(0) * norm / r , vel * zFrac;  
    }
    else
    {
        // now, pout is rotating around the Z axis
        double rmag = std::sqrt(pout(0) * pout(0) + pout(1) * pout(1)); 

        double e = GetUniformRandomDouble(0, mEcc);
        double E = 2 * M_PI * GetUniformRandomDouble(0.0, 1.0);
        double a = rmag / (1 - e * cos(E));
        double b = a * std::sqrt(1 - e * e);
        double vel = std::sqrt(mass * G *(2 / rmag - 1 / a));

        // x**2/a**2 + y**2/b**2 = 1
        // x = a * cos(t), dx/dt = -a * sin(t) = -(b / a) * y
        // y = b * sin(t), dy/dt = b * cos(t) = (a / b) * x

        vout << -(b / a) * (vel * pout(1) / rmag) , (a / b) * vel * pout(0) / rmag , vel * zFrac;
    }

    if (xval || yval)
    {
        // axis not pure Z, so rotate back
        double angz = qrz.w();
        qrz.w() = -angz;
        double angy = qry.w();
        qry.w() = -angy;
        Eigen::Quaterniond vval;
        vval.w() = 0;
        vval.vec() = vout;
        Eigen::Quaterniond vrot1 = qrz * vval * qrz.inverse();
        Eigen::Quaterniond vrot2 = qry * vrot1 * qry.inverse();
        vout = vrot2.vec();
    }

    v.mX = vout(0) * mAMRratio;
    v.mY = vout(1) * mAMRratio;
    v.mZ = vout(2) * mAMRratio;
    return v;
}
/*
 double ObjInput::GetVP(double mass, double vpmin, double vpmax, int dist)
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
                             */

// global var, reuse to get different vals
//std::default_random_engine gen;

double GetUniformRandomDouble(double rmin, double rmax)
{
//    std::uniform_real_distribution<double> dis(0.0, 1.0);
//    double r = dis(gen) * (rmax - rmin) + rmin; 
//    printf("urd: %f [rmin = %f, rmax = %f]\n", r, rmin, rmax);
//    return (r);

    if (rmin >= rmax)
    {
        assert(rmin == rmax);
        return rmax;
    }

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

bool ObjInput::WriteData()
{
    std::ofstream ofs;

    // traverse Object vector and create string
    // NOTE: must flip byte order
    int n = mObjects.size();

    if (mFormat == kTipsy)
    {
        ofs.open(mOutFName, std::ios::out | std::ios::binary);

        struct Header
        {
            Header(int n, int g, int d, int s)
                : t(0.0)
                , pad(0)
            {
                int axes = 3;
                char* ca = (char*)&axes;
                char* cn = (char*)&n;
                char* cg = (char*)&g;
                char* cd = (char*)&d;
                char* cs = (char*)&s;

                for (int i = 0; i < 4; ++i)
                {
                    //int j = i;
                    int j = 3 - i;
                    tot[j] = *(cn + i);
                    dim[j] = *(ca + i);
                    nSph[j] = *(cg + i);
                    nDark[j] = *(cd + i);
                    nStar[j] = *(cs + i);
                }
            }

            double t; // time
            char tot[4];  // total number of particles
            char dim[4]; // dimensions
            char nSph[4];
            char nDark[4];
            char nStar[4];
            int pad;
        };

        Header h(n, (int)mNumSPH, (int)mNumDark, (int)mNumStar);
        ofs.write((const char*)(&h), sizeof(Header));

        struct GasP 
        {
            GasP(float m, float x, float y, float z, float vx, float vy, float vz, 
                float rho, float temp, float eps, float met)
            {
                mMass = m;
                mPos[0] = x;
                mPos[1] = y;
                mPos[2] = z;
                mVel[0] = vx;
                mVel[1] = vy;
                mVel[2] = vz;
                mRho = rho;
                mTemp = temp;
                mEps = eps;
                mMetals = met;
                mPhi = 0.0;
            }

            bool Write(std::ofstream& ofs)
            {
                float nums[12] = 
                {
                    // swap y and z
                    mMass, 
                    mPos[0],
                    mPos[2],
                    mPos[1],
                    mVel[0],
                    mVel[2],
                    mVel[1],
                    mRho,
                    mTemp,
                    mEps,
                    mMetals,
                    mPhi
                };

                for (int i = 0; i < 12; ++i)
                {
                    char swap[4];
                    char* f = (char*)&nums[i];
                    swap[3] = f[0];
                    swap[2] = f[1];
                    swap[1] = f[2];
                    swap[0] = f[3];
                    ofs.write(swap, 4);
                }

                return true;
            }

            float mMass; // 9.19e-4 testonestar, 3.482e-5 testcollapse, 3.482e-5 testdamping, 1.5625e-5 testndsph, 2.4193e-6 testshock
            float mPos[3];
            float mVel[3];
            float mRho;
            float mTemp;
            float mEps;
            float mMetals;
            float mPhi;
        };
        /*                
         float rho = mRho;  // gas density 
         //  0.216 testonestar, 
         //  1.0 testcollapse, 
         //  1.0 testdamping, 
         //  1.0 testndsph/accel & density, 
         //  3.45257 testshock
         float temp = mGasTemp; // gas temperature 
         //  5134 testonestar, 
         //  0.05 testcollapse, 
         //  0.05 testdamping, 
         //  1.0 testndsph/accel & density, 
         //  115.86 testshock
         float eps = mEps; // gravitational softening 
         //  0.00125 testonestar, 
         //  0.01 testcollapse, 
         //  0.01 testdamping, 
         //  200.0 testndsph/accel & density, 
         //  0.0027027 testshock
         float metals = mMetals; // metallicity 
         //  0 testonestar, 
         //  0 testcollapse, 
         //  0 testdamping, 
         //  0 testndsph/accel & density, 
         // -1.084e-19 testshock
         float phi = 0;    // gravitational potential 
         // -1.95 testonestar, 
         //  0 testcollapse, 
         //  0 testdamping, 
         //  0 testndsph/accel & density, 
         //  3.45257 testshock
         */                        
        for (int i = 0; i < n; ++i)
        {
            Object& obj = mObjects[i];

            if (obj.mType != kSPH)
            {
                continue;
            }

            GasP g((float)obj.mMass, (float)obj.mX, (float)obj.mY, (float)obj.mZ,
                (float)obj.mVx, (float)obj.mVy, (float)obj.mVz,
                (float)mRho, (float)mGasTemp, (float)mEps, (float)mMetals);
            g.Write(ofs);
            //ofs.write((const char*)(&g), sizeof(GasP));
        }

        struct DarkP 
        {
            DarkP(float m, float x, float y, float z, float vx, float vy, float vz, 
                float eps)
            {
                mMass = m;
                mPos[0] = x;
                mPos[1] = y;
                mPos[2] = z;
                mVel[0] = vx;
                mVel[1] = vy;
                mVel[2] = vz;
                mEps = eps;
                mPhi = 0.0;
            }

            bool Write(std::ofstream& ofs)
            {
                float nums[9] = 
                {
                    // swap y and z
                    mMass, 
                    mPos[0],
                    mPos[2],
                    mPos[1],
                    mVel[0],
                    mVel[2],
                    mVel[1],
                    mEps,
                    mPhi
                };

                for (int i = 0; i < 9; ++i)
                {
                    char swap[4];
                    char* f = (char*)&nums[i];
                    swap[3] = f[0];
                    swap[2] = f[1];
                    swap[1] = f[2];
                    swap[0] = f[3];
                    ofs.write(swap, 4);
                }

                return true;
            }

            float mMass; // 2.7127e-6 testcosmo, 6.959e-4 teststep
            float mPos[3];
            float mVel[3];
            float mEps; // 0.0035 testcosmo, 0.20422 teststep
            float mPhi ; // 0 testcosmo, 0 teststep
        };

        for (int i = 0; i < n; ++i)
        {
            Object& obj = mObjects[i];

            if (obj.mType != kDark)
            {
                continue;
            }

            DarkP d((float)obj.mMass, (float)obj.mX, (float)obj.mY, (float)obj.mZ,
                (float)obj.mVx, (float)obj.mVy, (float)obj.mVz,
                (float)mEps);
            d.Write(ofs);
            //ofs.write((const char*)(&d), sizeof(DarkP));
        }

        struct StarP 
        {
            StarP(float m, float x, float y, float z, float vx, float vy, float vz, 
                float met, float eps)
            {
                mMass = m;
                mPos[0] = x;
                mPos[1] = y;
                mPos[2] = z;
                mVel[0] = vx;
                mVel[1] = vy;
                mVel[2] = vz;
                mMetals = met;
                mTform = 0.0;
                mEps = eps;
                mPhi = 0.0;
            }

            bool Write(std::ofstream& ofs)
            {
                float nums[11] = 
                {
                    // swap y and z
                    mMass, 
                    mPos[0],
                    mPos[2],
                    mPos[1],
                    mVel[0],
                    mVel[2],
                    mVel[1],
                    mMetals,
                    mTform,
                    mEps,
                    mPhi
                };

                for (int i = 0; i < 11; ++i)
                {
                    char swap[4];
                    char* f = (char*)&nums[i];
                    swap[3] = f[0];
                    swap[2] = f[1];
                    swap[1] = f[2];
                    swap[0] = f[3];
                    ofs.write(swap, 4);
                }

                return true;
            }

            float mMass;    // 2.757e-4 testonestar
            float mPos[3];
            float mVel[3];
            float mMetals ; // .02 testonestar
            float mTform ;  // 0 testonestar
            float mEps;     // 1.25e-3 testonestar
            float mPhi ;    // 0 testonestar
        };

        for (int i = 0; i < n; ++i)
        {
            Object& obj = mObjects[i];

            if (obj.mType != kStar)
            {
                continue;
            }

            StarP s((float)obj.mMass, (float)obj.mX, (float)obj.mY, (float)obj.mZ,
                (float)obj.mVx, (float)obj.mVy, (float)obj.mVz,
                (float)mMetals, (float)mEps);
            s.Write(ofs);
            //ofs.write((const char*)(&s), sizeof(StarP));
        }
    }
    else if (mFormat == kSimple)
    {
        ofs.open(mOutFName, std::ios::out);
        std::ofstream ofsDisplay;
        std::string fnameDisp = mOutFName + ".js";
        ofsDisplay.open(fnameDisp, std::ios::out);
        std::string optionstr;

        // Options
        std::string txt;

        txt = std::string("1.0e-4");
        optionstr += "momtolOpt=";
        optionstr += txt;
        optionstr += ", ";

        txt = std::string("1.0e-5");
        optionstr += "dvnormtolOpt=";
        optionstr += txt;
        optionstr += ", ";

        txt = std::string("1.0e6");
        optionstr += "runs=";
        optionstr += txt;
        optionstr += ", ";

        txt = std::string("1");
        optionstr += "tstep=";
        optionstr += txt;
        optionstr += ", ";

        txt = std::string("1.0e-2");
        optionstr += "minvforcheck=";
        optionstr += txt;

        ofs << "Options: " << optionstr << std::endl;
        ofsDisplay << "const numSnapshots = 1;";
        ofsDisplay << "let sphereArray1 = [";
        ofsDisplay << std::endl;

        // traverse Object vector and create std::string
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
            ofsDisplay << obj.mX << ", "
                << obj.mY << ", "
                << obj.mZ << ", "
                << obj.mRadius << ", " << std::endl;
        }

        ofsDisplay << "];";
        ofsDisplay << std::endl;
        ofsDisplay.close();
    }

    ofs.close();

    return true;
}

bool ObjInput::AdjustLocForOverlap(double& x, double& y, double& z, double radius, int& xflag, int& xflag2, int& yflag, int& yflag2, int& zflag, int& zflag2)
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
                if (mShape == kSpherical)
                {
                    // use rectilinear coords, but redo any that are outside our sphere
                    // need neg values, use rand()
                    double rmag = 0;

                    do
                    {
                        x = GetLoc(mVolDim1, mLocationDist);
                        y = GetLoc(mVolDim1, mLocationDist);
                        z = GetLoc(mVolDim1, mLocationDist);
                        rmag = std::sqrt(x * x + y * y + z * z);
                    }        
                    while (rmag > mVolDim1);

                    if (rand() > RAND_MAX / 2)
                    {
                        x = -x;
                    }

                    if (rand() > RAND_MAX / 2)
                    {
                        y = -y;
                    }

                    if (rand() > RAND_MAX / 2)
                    {
                        z = -z;
                    }
                }
                else if (mShape == kRectilinear)
                {
                    x = GetLoc(mVolDim1, mLocationDist);

                    if (rand() > RAND_MAX / 2)
                    {
                        x = -x;
                    }

                    y = GetLoc(mVolDim2, mLocationDist);

                    if (rand() > RAND_MAX / 2)
                    {
                        y = -y;
                    }

                    z = GetLoc(mVolDim3, mLocationDist);

                    if (rand() > RAND_MAX / 2)
                    {
                        z = -z;
                    }
                }
                else // cylindrical
                {
                    // as for spherical, use rectilinear coords, redo points outside cyl
                    double rmag = 0;

                    do
                    {
                        x = GetLoc(mVolDim1, mLocationDist);
                        y = GetLoc(mVolDim1, mLocationDist);
                        rmag = sqrt(x * x + y * y);
                    }
                    while (rmag > mVolDim1);
                    
                    z = GetLoc(mVolDim2, mLocationDist);

                    if (rand() > RAND_MAX / 2)
                    {
                        x = -x;
                    }

                    if (rand() > RAND_MAX / 2)
                    {
                        y = -y;
                    }

                    if (rand() > RAND_MAX / 2)
                    {
                        z = -z;
                    }
                }

                CalcFlags(x, y, z, xflag, xflag2, yflag, yflag2, zflag, zflag2);

                // start over checking objects
                i = 0;

                if (++counter > 1000)
                {
                    // is the space too small?
                    //assert(counter < 1000);
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

void ObjInput::CalcFlags(double x, double y, double z, int& xflag, int& xflag2, int& yflag, int& yflag2, int& zflag, int& zflag2)
{
    int fullShiftx = 0;
    int halfShiftx = 0;

    // set up loc flags, based on 2 * mVol1 (if spherical), 1 / 32 per bit
    // also set up loc2 flags, 1/2 a division over from loc flags
    switch (mShape)
    {
        case kSpherical:
            fullShiftx = (int)(((x + mVolDim1) * 16 / mVolDim1));
            halfShiftx = (int)(((x + mVolDim1) * 16 / mVolDim1) + (mVolDim1 / 32));
            break;
        case kRectilinear:
            fullShiftx = (int)(((x + mVolDim1) * 16 / mVolDim1));
            halfShiftx = (int)(((x + mVolDim1) * 16 / mVolDim1) + (mVolDim1 / 32));
            break;
        case kCylindrical:
            fullShiftx = (int)(((x + mVolDim1) * 16 / mVolDim1));
            halfShiftx = (int)(((x + mVolDim1) * 16 / mVolDim1) + (mVolDim1 / 32));
            break;
    }

    if (fullShiftx > 31)
    {
        fullShiftx = 31;
    }

    xflag = 1 << fullShiftx;

    if (halfShiftx > 31)
    {
        halfShiftx = 31;
    }

    xflag2 = 1 << halfShiftx;

    int fullShifty = 0;
    int halfShifty = 0;

    switch (mShape)
    {
        case kSpherical:
            fullShifty = (int)(((y + mVolDim1) * 16 / mVolDim1));
            halfShifty = (int)(((y + mVolDim1) * 16 / mVolDim1) + (mVolDim1 / 32));
            break;
        case kRectilinear:
            fullShifty = (int)(((y + mVolDim2) * 16 / mVolDim2));
            halfShifty = (int)(((y + mVolDim2) * 16 / mVolDim2) + (mVolDim2 / 32));
            break;
        case kCylindrical:
            fullShifty = (int)(((y + mVolDim1) * 16 / mVolDim1));
            halfShifty = (int)(((y + mVolDim1) * 16 / mVolDim1) + (mVolDim1 / 32));
            break;
    }

    if (fullShifty > 31)
    {
        fullShifty = 31;
    }

    yflag = 1 << fullShifty;

    if (halfShifty > 31)
    {
        halfShifty = 31;
    }

    yflag2 = 1 << halfShifty;

    int fullShiftz = 0;
    int halfShiftz = 0;

    switch (mShape)
    {
        case kSpherical:
            fullShiftz = (int)(((z + mVolDim1) * 16 / mVolDim1));
            halfShiftz = (int)(((z + mVolDim1) * 16 / mVolDim1) + (mVolDim1 / 32));
            break;
        case kRectilinear:
            fullShiftz = (int)(((z + mVolDim3) * 16 / mVolDim3));
            halfShiftz = (int)(((z + mVolDim3) * 16 / mVolDim3) + (mVolDim3 / 32));
            break;
        case kCylindrical:
            fullShiftz = (int)(((z + mVolDim2) * 16 / mVolDim2));
            halfShiftz = (int)(((z + mVolDim2) * 16 / mVolDim2) + (mVolDim2 / 32));
            break;
    }

    if (fullShiftz > 31)
    {
        fullShiftz = 31;
    }

    zflag = 1 << fullShiftz;

    if (halfShiftz > 31)
    {
        halfShiftz = 31;
    }

    zflag2 = 1 << halfShiftz;
}
