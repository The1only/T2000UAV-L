/*
*) Refactor the code to remove reduentent part. 
*) Compiled for Linux with C++14 standard
Copyright (c) 2021 Balamurugan Kandan.
MIT License; See LICENSE.md for complete details
Author: 2021 Balamurugan Kandan
*/

/*
Updated to be a class, use Eigen, and compile as an Arduino library.
Added methods to get gyro and accel bias. Added initialization to
estimated angles rather than assuming IMU is level.

Copyright (c) 2016 - 2019 Regents of the University of Minnesota and Bolder Flight Systems Inc.
MIT License; See LICENSE.md for complete details
Author: Brian Taylor
*/

/*
Addapted from earlier version
Copyright 2011 Regents of the University of Minnesota. All rights reserved.
Original Author: Adhika Lie
*/

#include "ekfNavINS_quart.h"

/*
ψ (psi): Rotation about the z-axis (yaw).
θ (theta): Rotation about the y-axis (pitch).
φ (phi): Rotation about the x-axis (roll).

Yaw (ψ) is a rotation around the z-axis.
Pitch (θ) is a rotation around the y-axis.
Roll (φ) is a rotation around the x-axis.
*/


#include <iostream>
#include <vector>
#include <Eigen/Dense>
#include "ekfNavINS.h"

using namespace Eigen;

ekfNavINS::ekfNavINS()
{
    // Initialize Extended Kalman filter
    ekf_quart = new ExtendedKalmanFilter_quart();
    ekf = new ExtendedKalmanFilter();

    initialized_ = true;
    ekf->Gval = Gfix;
    ekf_quart->Gval = Gfix;
}

void ekfNavINS::ekf_update(double time, double vn,double ve,double vd,double lat,double lon,double alt,double baroalt,float p,float q,float r,float ax,float ay,float az,float hx,float hy, float hz)
{
    m_dt = time;

    // Construct the measurement vector
    VectorXd measurements = VectorXd::Zero(15);
    measurements.segment<3>(0)  = Vector3d(lat, lon, alt);           // GPS position
    measurements.segment<3>(3)  = Vector3d(vn, ve, vd);  // GPS velocity
    measurements(6)             = baroalt;                // Barometer altitude
    measurements.segment<3>(7)  = Vector3d(ax, ay, az); // Accelerometer orientation
    measurements.segment<3>(10) = Vector3d(hx, hy, hz); // Magnetometer orientation
    measurements.segment<2>(13) = Vector2d(0, 0);    // Correction

    // Prediction step
    if(m_use_gpt==2) ekf_quart->predict(m_dt, Vector3d(p, q, r), Vector3d(ax, ay, az));
    else          ekf->predict(m_dt, Vector3d(p, q, r), Vector3d(ax, ay, az));

    // Update step
    if(m_use_gpt==2) ekf_quart->update(measurements);
    else             ekf->update(measurements);
}

// ...............................................

// az = up donwn, ax = Roll, ay = flatt up
std::tuple<float,float,float> ekfNavINS::getPitchRoll(float ax, float ay, float az, float Gcal )
{
    // initial attitude and heading
    if(ax > Gcal) ax = Gcal;
    if(ay > Gcal) ay = Gcal;

    float theta = -asinf(ax/Gcal);
    float phi = -asinf(ay/Gcal*cosf(theta));

    return (std::make_tuple(theta,phi,0));
}

// Working...
std::tuple<float,float,float> ekfNavINS::getYaw(float roll,float pitch, float hx, float hy, float hz, float Gcal )
{
    // magnetic heading corrected for roll and pitch angle
    float Bxc, Byc;

    double hz_ = (hx * sin(pitch)) + (hz * cos(pitch)) ;
    double hx_ = (hx * cos(pitch)) + (hz * sin(pitch)) ;

    // Magnetic heading correction due to roll and pitch angle (after 90-degree rotation)
    Byc = hz*cosf(pitch) + (hy*sinf(roll) + hx*cosf(roll))*sinf(pitch);
    Bxc = (hy*cosf(roll) - hx*sinf(roll));

    // finding initial heading
    float psi = -atan2f(Bxc,-Byc);
  //  float psi = -atan2f(hy,-hz_);

    float p = atan2(hz, sqrt(hy*hy + hx*hx));
    float r = atan2(hy, sqrt(hz_*hz_ + hx_*hx_));

    //    psi = psi + gz*DEG_TO_RAD*m_dt;
    return (std::make_tuple(p,r,psi));
}
//--------------------------------------

