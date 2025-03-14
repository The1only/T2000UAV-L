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

#include "ekfNavINS.h"

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

using namespace Eigen;

void ekfNavINS::ekf_update(double time, double vn,double ve,double vd,double lat,double lon,double alt,double baroalt,float p,float q,float r,float ax,float ay,float az,float hx,float hy, float hz)
{
    m_dt = time;

    if (!initialized_) {
        // Initialize Extended Kalman filter
        ekf = new ExtendedKalmanFilter();
        initialized_ = true;
        ekf->Gval = Gfix;
    }

    // Example sensor data (add more data points as needed)
    std::vector<SensorData> sensorData = {
        {
            Vector3d(ax, ay, az),  // Accelerometer readings
            Vector3d(p, q, r), // Gyroscope readings
            Vector3d(hx, hy, hz),   // Magnetometer readings
            baroalt,                     // Barometer reading
            Vector3d(lat, lon, alt),      // GPS position readings
            Vector3d(vn, ve, vd)          // GPS velocity readings
        }
    };

    for (const auto &data : sensorData) {
        // Construct the measurement vector
        VectorXd measurements(15);
        measurements.segment<3>(0) = data.gps;           // GPS position
        measurements.segment<3>(3) = data.gps_velocity;  // GPS velocity
        measurements(6) = data.barometer;                // Barometer altitude
        measurements.segment<3>(7) = data.accelerometer; // Accelerometer orientation
        measurements.segment<3>(10) = data.magnetometer; // Magnetometer orientation
        measurements.segment<2>(13) = Vector2d(0, 0);    // Correction

        // Prediction step
        ekf->predict(m_dt, data.gyroscope, data.accelerometer);

        // Update step
        ekf->update(measurements);

        theta = ekf->state[6];
        phi = ekf->state[7];
        psi = ekf->state[8];
        lat_ins = ekf->state[0];
        lon_ins = ekf->state[1];
        alt_ins = ekf->state[2];
        vn_ins = ekf->state[3];
        ve_ins = ekf->state[4];
        vd_ins = ekf->state[5];
    }
}

// ...............................................

// az = up donwn, ax = Roll, ay = flatt up
std::tuple<float,float,float> ekfNavINS::getPitchRollYaw(float ax, float ay, float az, float hx, float hy, float hz, double Gcal )
{
    // initial attitude and heading
    if(ax > Gcal) ax = Gcal;
    if(ay > Gcal) ay = Gcal;

    theta = -asinf(ax/Gcal);
    phi = -asinf(ay/Gcal*cosf(theta));

    // magnetic heading correction due to roll and pitch angle
    //Bxc = hx*cosf(theta) + (hy*sinf(phi) + hz*cosf(phi))*sinf(theta);
    //Byc = hy*cosf(phi) - hz*sinf(phi);

    // Magnetic heading correction due to roll and pitch angle (after 90-degree rotation)
    Bxc = -(hy*cosf(phi) - hx*sinf(phi));
    Byc = hx*cosf(theta) + (hy*sinf(phi) + hz*cosf(phi))*sinf(theta);

    // finding initial heading
    psi = -atan2f(Bxc,Byc);

//    qDebug() << "H: " << hx << "  "  << hy << "  "  <<  hz << " PSI: " << psi*RAD_TO_DEG;

    //    psi = psi + gz*DEG_TO_RAD*m_dt;
    return (std::make_tuple(theta,phi,psi));
}
//--------------------------------------

