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
std::tuple<double,double,double> ekfNavINS::getPitchRoll(double ax, double ay, double az, double Gcal )
{
    (void) Gcal;
    Vector3d att = {ax,ay,az};
    att = att.normalized();

    double theta2 = -atan2(att[0],att[2]);
    double phi2 = atan2(-att[1], sqrt((att[0]*att[0]) + (att[2]*att[2])));
    return (std::make_tuple(theta2,phi2,0));
}

// -------------------------------------------------------
// Working...
float ekfNavINS::getHeading(float Mx, float My, float Mz, float roll, float pitch)
{
    Vector3d mag = {Mx,My,Mz};
    mag = mag.normalized();

    float mx2 = mag[0] * cos(pitch) + mag[2] * sin(pitch);
    float my2 = mag[0] * sin(roll) * sin(pitch) + mag[1] * cos(roll) - mag[2] * sin(roll) * cos(pitch);

    float heading = atan2(my2, mx2);  // North-referenced
    //float heading  = atan2(mag[1],mag[0]);
    //if (heading < 0) heading += 2 * M_PI;

 //   qDebug() << pitch*RAD_TO_DEG << roll*RAD_TO_DEG << heading*RAD_TO_DEG;
    return heading;  // In radians
}
//--------------------------------------

