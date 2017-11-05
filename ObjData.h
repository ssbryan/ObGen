#ifndef OBJDATA_H
#define OBJDATA_H

// to get M_PI
#define _USE_MATH_DEFINES 

#include <cmath>

// Author Stephen Bryan
// June 28, 2017

class ObjData
{
public:
    ObjData(void) {}
    ~ObjData(void) {}

    void    SetLocation(double x, double y, double z);
    void    SetVelocity(double vx, double vy, double vz);
    void    SetMass(double mass);
    void    SetIndex(int index);
    int     GetIndex(void) const;

private:
    int     mId;
    double  mX;
    double  mY;
    double  mZ;

    double  mVx;
    double  mVy;
    double  mVz;

    double  mMass;
    double  mRadius;
};

inline void ObjData::SetLocation(double x, double y, double z)
{
    mX = x;
    mY = y;
    mZ = z;
}

inline void ObjData::SetVelocity(double vx, double vy, double vz)
{
    mVx = vx;
    mVy = vy;
    mVz = vz;
}

inline void ObjData::SetMass(double mass)
{
    mMass = mass;

    // determine radius: rho = M / V, V = M / rho

    // rho = 3g/cc plus porosity discount of 50% (for now - see Object.cpp in Asteroid project)
    double rho0 = 3000; // 3g/cc (* 1e-3kg/g * 1e6cc/m^3), similar to carbonaceous chondrites
    double porosity = 0.5; // 50% porosity, changes effective rho

                           // should do an "if" to see if a random element is needed (solid rock, not rubble), 
                           // or if we want a nickel-iron chunk at 7.9g/cc (3.84% of meteorites)
    double rho = rho0 * porosity;

    // volume
    double v = mMass / rho;

    // R = (3 * V / (4 * pi))^(1/3)
    mRadius = pow(3 * v / (4 * M_PI), 0.33333333);
}

inline void ObjData::SetIndex(int index)
{
    mId = index;
}

inline int ObjData::GetIndex(void) const
{
    return mId;
}

#endif
