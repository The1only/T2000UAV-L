/*
*) Refactor the code to remove reduentent part and improve the readabilty. 
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

#pragma once
#include <QCoreApplication>

#include <stdint.h>
#include <math.h>
#include <tuple>
#include <mutex>
#include <shared_mutex>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <iostream>
#include <vector>

#define DEG_TO_RAD (3.141592/180)
#define RAD_TO_DEG (180.0/3.141592765)

// acceleration due to gravity
constexpr float Gfix = 9.82500f; //9.808f;

// earth semi-major axis radius (m)
constexpr double EARTH_RADIUS = 6378137.0;

using namespace Eigen;

// Struct to hold sensor data
struct SensorData {
    Vector3d accelerometer; // Accelerometer readings (x, y, z)
    Vector3d gyroscope;     // Gyroscope readings (roll rate, pitch rate, yaw rate)
    Vector3d magnetometer;  // Magnetometer readings (x, y, z)
    double barometer;       // Barometer reading (altitude)
    Vector3d gps;           // GPS readings (x, y, z position)
    Vector3d gps_velocity;  // GPS velocity readings (vx, vy, vz)
};

// Extended Kalman Filter class
struct ExtendedKalmanFilter {
    VectorXd state;   // State vector: [x, y, z, vx, vy, vz, roll, pitch, yaw, accel_bias_x, accel_bias_y, accel_bias_z, gyro_bias_x, gyro_bias_y, gyro_bias_z]
    MatrixXd P;       // Covariance matrix
    MatrixXd Q;       // Process noise covariance matrix
    MatrixXd R;       // Measurement noise covariance matrix

    // calibrated G...
    double Gval=Gfix;

    // Constructor to initialize the filter
    ExtendedKalmanFilter() {
        // Initialize state vector
        state = VectorXd::Zero(15);

        // Initialize covariance matrix
        P = MatrixXd::Identity(15, 15) * 1e-3;

        // Process noise covariance matrix
        Q = MatrixXd::Zero(15, 15);
        Q.diagonal() << 1e-4, 1e-4, 1e-4, 1e-2, 1e-2, 1e-2, 1e-3, 1e-3, 1e-3, 1e-5, 1e-5, 1e-5, 1e-5, 1e-5, 1e-5;

        // Measurement noise covariance matrix
        R = MatrixXd::Zero(15, 15);

        // Set diagonal elements of R explicitly
        R(0, 0) = 1e-2; // GPS x noise
        R(1, 1) = 1e-2; // GPS y noise
        R(2, 2) = 1e-2; // GPS z noise
        R(3, 3) = 1e-2; // GPS vx noise
        R(4, 4) = 1e-2; // GPS vy noise
        R(5, 5) = 1e-2; // GPS vz noise
        R(6, 6) = 1e-1; // Barometer noise
        R(7, 7) = 0.01; //1e-2; // Accelerometer roll noise
        R(8, 8) = 0.01; //1e-2; // Accelerometer pitch noise
        R(9, 9) = 0.01; //1e-2; // Accelerometer yaw noise
        R(10, 10) = 1e-3; // Magnetometer x noise
        R(11, 11) = 1e-3; // Magnetometer y noise
        R(12, 12) = 1e-3; // Magnetometer z noise
    }

    // State vector: [x, y, z, vx, vy, vz, roll, pitch, yaw, accel_bias_x, accel_bias_y, accel_bias_z, gyro_bias_x, gyro_bias_y, gyro_bias_z]
    // Non-linear state transition function
    VectorXd f(const VectorXd &state, const Vector3d &gyro, double dt) {
        VectorXd newState = state;
        newState.segment<3>(0) += state.segment<3>(3) * dt; // Update position from speed...
        newState.segment<3>(3) += Vector3d(0, 0, -Gval) * dt; // Velocity Gravity effect to accelerometer values ...
        newState.segment<3>(6) += (gyro - state.segment<3>(12)) * dt; // Update orientation (roll, pitch, yaw) considering gyro bias
        newState.segment<3>(9)  = state.segment<3>(9); // Accelerometer bias remains constant
        newState.segment<3>(12) = state.segment<3>(12); // Gyroscope bias remains constant
        return newState;
    }

    // Jacobian of the state transition function
    MatrixXd F_jacobian(const VectorXd &state, double dt) {
        MatrixXd F = MatrixXd::Identity(15, 15);
        F(0, 3) = dt;
        F(1, 4) = dt;
        F(2, 5) = dt;
        F(6, 12) = -dt;
        F(7, 13) = -dt;
        F(8, 14) = -dt;
        return F;
    }

    // Non-linear measurement function
    VectorXd h(const VectorXd &state) {
        VectorXd measurements(15);
        measurements.segment<3>(0) = state.segment<3>(0); // Estimated GPS position from velocity...
        measurements.segment<3>(3) = state.segment<3>(3); // GPS velocity
        measurements(6) = state(2);                      // Barometer altitude
        measurements.segment<3>(7) = state.segment<3>(6); // Accelerometer orientation
        measurements.segment<3>(10) = state.segment<3>(10); // Magnetometer orientation
        measurements.segment<2>(13) = Vector2d(0, 0);                   // Correction
        return measurements;
    }

    // Jacobian of the measurement function
    MatrixXd H_jacobian(const VectorXd &state) {
        MatrixXd H = MatrixXd::Zero(15, 15);
        H.block<3, 3>(0, 0) = Matrix3d::Identity(); // GPS position measurements
        H.block<3, 3>(3, 3) = Matrix3d::Identity(); // GPS velocity measurements
        H(6, 2) = 1.0;                             // Barometer altitude measurement
        H.block<3, 3>(7, 6) = Matrix3d::Identity(); // Accelerometer orientation measurements
        H.block<3, 3>(10, 6) = Matrix3d::Identity(); // Magnetometer orientation measurements
        return H;
    }

    // Prediction step
    void predict(double dt, const Vector3d &gyro, __attribute__((unused)) const Vector3d &accel) {
        // Predict the state using the non-linear transition function
        state = f(state, gyro, dt);

        // Update the covariance using the Jacobian
        MatrixXd F = F_jacobian(state, dt);
        P = F * P * F.transpose() + Q;
    }

    // Update step
    void update(const VectorXd &measurements) {
        // Compute the expected measurement
        VectorXd y = measurements - h(state);

        // Compute the Jacobian of the measurement function
        MatrixXd H = H_jacobian(state);

        // Compute Kalman gain with regularization to avoid NaN
        MatrixXd S = H * P * H.transpose() + R;
        S += MatrixXd::Identity(S.rows(), S.cols()) * 1e-9; // Regularization term

        MatrixXd K = P * H.transpose() * S.inverse();

        // Update the state estimate
        state = state + K * y;

        // Update the covariance matrix
        P = (MatrixXd::Identity(15, 15) - K * H) * P;
    }
};


// Extended Kalman Filter class
struct ExtendedKalmanFilter_with_bias {
    VectorXd state;   // State vector: [x, y, z, vx, vy, vz, roll, pitch, yaw, accel_bias_x, accel_bias_y, accel_bias_z, gyro_bias_x, gyro_bias_y, gyro_bias_z]
    MatrixXd P;       // Covariance matrix
    MatrixXd Q;       // Process noise covariance matrix
    MatrixXd R;       // Measurement noise covariance matrix

    // calibrated G...
    double Gval=Gfix;

    // Constructor to initialize the filter
    ExtendedKalmanFilter_with_bias() {
        // Initialize state vector
        state = VectorXd::Zero(18);

        // Initialize covariance matrix
        P = MatrixXd::Identity(18, 18) * 1e-3;

        // Process noise covariance matrix
        Q = MatrixXd::Zero(18, 18);
        Q.diagonal() <<
            1e-4, 1e-4, 1e-4,     // position x, y, z
            1e-2, 1e-2, 1e-2,     // velocity vx, vy, vz
            1e-3, 1e-3, 1e-3,     // orientation roll, pitch, yaw
            1e-5, 1e-5, 1e-5,     // accel bias x, y, z
            1e-5, 1e-5, 1e-5,     // gyro bias x, y, z
            1e-5, 1e-5, 1e-5;     // magnetometer bias x, y, z
//        Q.diagonal() << 1e-4, 1e-4, 1e-4, 1e-2, 1e-2, 1e-2, 1e-3, 1e-3, 1e-3, 1e-5, 1e-5, 1e-5, 1e-5, 1e-5, 1e-5;

        // Measurement noise covariance matrix
        R = MatrixXd::Zero(15, 15);

        // Set diagonal elements of R explicitly
        R(0, 0) = 1e-2; // GPS x noise
        R(1, 1) = 1e-2; // GPS y noise
        R(2, 2) = 1e-2; // GPS z noise
        R(3, 3) = 1e-2; // GPS vx noise
        R(4, 4) = 1e-2; // GPS vy noise
        R(5, 5) = 1e-2; // GPS vz noise
        R(6, 6) = 1e-1; // Barometer noise
        R(7, 7) = 0.05; //1e-2; // Accelerometer roll noise
        R(8, 8) = 0.05; //1e-2; // Accelerometer pitch noise
        R(9, 9) = 0.05; //1e-2; // Accelerometer yaw noise
        R(10, 10) = 0.02; // Magnetometer x noise
        R(11, 11) = 0.02; // Magnetometer y noise
        R(12, 12) = 0.02; //1e-2; // Magnetometer z noise
    }

    // State vector: [x, y, z, vx, vy, vz, roll, pitch, yaw, accel_bias_x, accel_bias_y, accel_bias_z, gyro_bias_x, gyro_bias_y, gyro_bias_z]
    // Non-linear state transition function
    VectorXd f_old(const VectorXd &state, const Vector3d &gyro, double dt) {
        VectorXd newState = state;
        newState.segment<3>(0) += state.segment<3>(3) * dt; // Update position from speed...
        newState.segment<3>(3) += Vector3d(0, 0, -Gval) * dt; // Velocity Gravity effect to accelerometer values ...
        newState.segment<3>(6) += (gyro - state.segment<3>(12)) * dt; // Update orientation (roll, pitch, yaw) considering gyro bias
        newState.segment<3>(9)  = state.segment<3>(9); // Accelerometer bias remains constant
        newState.segment<3>(12) = state.segment<3>(12); // Gyroscope bias remains constant
        return newState;
    }

    VectorXd f(const VectorXd &state, const Vector3d &gyro, const Vector3d &accel, double dt) {
        VectorXd newState = state;

        // Extract orientation (roll, pitch, yaw)
        double roll = state(6), pitch = state(7), yaw = state(8);

        // Rotation matrix from body to world frame
        Matrix3d Rd;
        Rd = AngleAxisd(yaw, Vector3d::UnitZ()) *
            AngleAxisd(pitch, Vector3d::UnitY()) *
            AngleAxisd(roll, Vector3d::UnitX());

        // Remove bias from accelerometer
        Vector3d accel_corrected = accel - state.segment<3>(9);

        // Transform to world frame and add gravity
        Vector3d world_accel = Rd * accel_corrected + Vector3d(0, 0, -Gval);

        // Update position and velocity
        newState.segment<3>(0) += state.segment<3>(3) * dt + 0.5 * world_accel * dt * dt;
        newState.segment<3>(3) += world_accel * dt;

        // Update orientation from gyro
        newState.segment<3>(6) += (gyro - state.segment<3>(12)) * dt;

        // Biases stay constant
        return newState;
    }

    // Jacobian of the state transition function
    MatrixXd F_jacobian_old(const VectorXd &state, double dt) {
        MatrixXd F = MatrixXd::Identity(15, 15);
        F(0, 3) = dt;
        F(1, 4) = dt;
        F(2, 5) = dt;
        F(6, 12) = -dt;
        F(7, 13) = -dt;
        F(8, 14) = -dt;
        return F;
    }

    MatrixXd F_jacobian(const VectorXd &state, double dt) {
        MatrixXd F = MatrixXd::Identity(18, 18);
        F(0, 3) = dt;
        F(1, 4) = dt;
        F(2, 5) = dt;
        F(6, 12) = -dt;
        F(7, 13) = -dt;
        F(8, 14) = -dt;
        return F;
    }

    // Non-linear measurement function
    VectorXd h_old(const VectorXd &state) {
        VectorXd measurements(15);
        measurements.segment<3>(0) = state.segment<3>(0); // Estimated GPS position from velocity...
        measurements.segment<3>(3) = state.segment<3>(3); // GPS velocity
        measurements(6) = state(2);                      // Barometer altitude
        measurements.segment<3>(7) = state.segment<3>(6); // Accelerometer orientation
        measurements.segment<3>(10) = state.segment<3>(10); // Magnetometer orientation
        measurements.segment<2>(13) = Vector2d(0, 0);                   // Correction
        return measurements;
    }

    VectorXd h(const VectorXd &state) {
        VectorXd measurements(15);
        measurements.segment<3>(0) = state.segment<3>(0);               // GPS position
        measurements.segment<3>(3) = state.segment<3>(3);               // GPS velocity
        measurements(6) = state(2);                                     // Barometer altitude
        measurements.segment<3>(7) = state.segment<3>(6) + state.segment<3>(9);   // Accel = orientation + bias
        measurements.segment<3>(10) = state.segment<3>(6) + state.segment<3>(15); // Magnetometer = orientation + bias
        measurements(13) = state(8);                                    // Yaw
        measurements(14) = 0;                                           // Placeholder
        return measurements;
    }

    // Jacobian of the measurement function
    MatrixXd H_jacobian_old(const VectorXd &state) {
        MatrixXd H = MatrixXd::Zero(15, 15);
        H.block<3, 3>(0, 0) = Matrix3d::Identity(); // GPS position measurements
        H.block<3, 3>(3, 3) = Matrix3d::Identity(); // GPS velocity measurements
        H(6, 2) = 1.0;                             // Barometer altitude measurement
        H.block<3, 3>(7, 6) = Matrix3d::Identity(); // Accelerometer orientation measurements
        H.block<3, 3>(10, 6) = Matrix3d::Identity(); // Magnetometer orientation measurements
        return H;
    }

    MatrixXd H_jacobian(const VectorXd &state) {
        MatrixXd H = MatrixXd::Zero(15, 18);
        H.block<3, 3>(0, 0) = Matrix3d::Identity();   // GPS pos
        H.block<3, 3>(3, 3) = Matrix3d::Identity();   // GPS vel
        H(6, 2) = 1.0;                                // Baro z
        H.block<3, 3>(7, 6) = Matrix3d::Identity();   // Accel orientation part
        H.block<3, 3>(7, 9) = Matrix3d::Identity();   // Accel bias
        H.block<3, 3>(10, 6) = Matrix3d::Identity();  // Mag orientation part
        H.block<3, 3>(10, 15) = Matrix3d::Identity(); // Mag bias
        H(13, 8) = 1.0;                               // Yaw
        return H;
    }

    // Prediction step
//    void predict(double dt, const Vector3d &gyro) {
    void predict(double dt, const Vector3d &gyro, const Vector3d &accel) {
        // Predict the state using the non-linear transition function
        state = f(state, gyro, accel, dt);

        // Update the covariance using the Jacobian
        MatrixXd F = F_jacobian(state, dt);
        P = F * P * F.transpose() + Q;
    }

    // Update step
    void update(const VectorXd &measurements) {
        // Compute the expected measurement
        VectorXd y = measurements - h(state);

        // Compute the Jacobian of the measurement function
        MatrixXd H = H_jacobian(state);

        // Compute Kalman gain with regularization to avoid NaN
        MatrixXd S = H * P * H.transpose() + R;
        S += MatrixXd::Identity(S.rows(), S.cols()) * 1e-9; // Regularization term

        MatrixXd K = P * H.transpose() * S.inverse();

        // Update the state estimate
        state = state + K * y;

        // Update the covariance matrix
        P = (MatrixXd::Identity(18, 18) - K * H) * P;
    }
};

class ekfNavINS {
  public:
    // ekf_update
    void ekf_update( double time/*, unsigned long TOW*/,   /* Time, Time of the week from GPS */
                    double vn, double ve, double vd,    /* Velocity North, Velocity East, Velocity Down */
                    double lat, double lon, double alt, /* GPS latitude, GPS longitude, GPS/Barometer altitude */
                    double baroalt,
                    float p, float q, float r,          /* Gyro P, Q and R  */
                    float ax, float ay, float az,       /* Accelarometer X, Y and Z */
                    float hx, float hy, float hz        /* Magnetometer HX, HY, HZ */ );

      ExtendedKalmanFilter *ekf = nullptr;

    // returns the pitch angle, rad
    void setPitch_rad(float x) { theta = x ;}
    // returns the roll angle, rad
    void setRoll_rad(float x) { phi = x ;}
    // returns the heading angle, rad
    void setHeading_rad(float x) { psi = x ;}

    // returns the pitch angle, rad
    float getPitch_rad()        { return theta; }
    // returns the roll angle, rad
    float getRoll_rad()         { return phi; }
    // returns the heading angle, rad
    float getHeading_rad()      { return psi; }

    // returns the INS latitude, rad
    double getLatitude_rad()    { return lat_ins; }
    // returns the INS longitude, rad
    double getLongitude_rad()   { return lon_ins; }
    // returns the INS altitude, m
    double getAltitude_m()      { return alt_ins; }

    // returns the INS north velocity, m/s
    double getVelNorth_ms()     { return vn_ins; }
    // returns the INS east velocity, m/s
    double getVelEast_ms()      { return ve_ins; }
    // returns the INS down velocity, m/s
    double getVelDown_ms()      { return vd_ins; }

    // returns the INS ground track, rad
    float getGroundTrack_rad()  { return atan2f((float)ve_ins,(float)vn_ins); }

    // returns the gyro bias estimate in the x direction, rad/s
    float getGyroBiasX_rads()   { return gbx; }
    // returns the gyro bias estimate in the y direction, rad/s
    float getGyroBiasY_rads()   { return gby; }
    // returns the gyro bias estimate in the z direction, rad/s
    float getGyroBiasZ_rads()   { return gbz; }

    // returns the accel bias estimate in the x direction, m/s/s
    float getAccelBiasX_mss()   { return abx; }
    // returns the accel bias estimate in the y direction, m/s/s
    float getAccelBiasY_mss()   { return aby; }
    // returns the accel bias estimate in the z direction, m/s/s
    float getAccelBiasZ_mss()   { return abz; }

    // return pitch, roll and yaw
    std::tuple<float,float,float> getPitchRollYaw(float ax, float ay, float az, float hx, float hy, float hz, double Gcal);

    // Use Kalmanfilter of build in function...
    bool m_use_gpt = false;
    bool initialized_ = false;
    double Gval = 9.82500f;


  private:
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////// member variables /////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////

    double m_dt = 0.1;
    // timing
    // estimated attitude
    float phi, theta, psi;
    // estimated NED velocity
    double vn_ins, ve_ins, vd_ins;
    // estimated location
    double lat_ins, lon_ins, alt_ins;
    // magnetic heading corrected for roll and pitch angle
    float Bxc, Byc;
    // accelerometer bias
    float abx = 0.0, aby = 0.0, abz = 0.0;
    // gyro bias
    float gbx = 0.0, gby = 0.0, gbz = 0.0;

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////// member functions /////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    void ekf_init(double time,
                 double vn,double ve,double vd, 
                 double lat,double lon,double alt,
                 double baroalt,
                 float p,float q,float r,
                 float ax,float ay,float az,
                 float hx,float hy, float hz);
};

