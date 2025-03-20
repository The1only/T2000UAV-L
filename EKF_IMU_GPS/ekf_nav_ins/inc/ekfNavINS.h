/*
Written By Terje Nilsen with help from ChatGPT...
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

#include "ekfNavINS_quart.h"

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

         Q.diagonal() <<1e-4, 1e-4, 1e-4,
                        1e-2, 1e-2, 1e-2,
                        1e-3, 1e-3, 1e-3,
                        1e-5, 1e-5, 1e-5,
                        1e-5, 1e-5, 1e-5;

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
        measurements.segment<2>(13) = Vector2d(0, 0);       // Correction
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

class ekfNavINS {
  public:
    ekfNavINS(void);

    // ekf_update
    void ekf_update( double time/*, unsigned long TOW*/,   /* Time, Time of the week from GPS */
                    double vn, double ve, double vd,    /* Velocity North, Velocity East, Velocity Down */
                    double lat, double lon, double alt, /* GPS latitude, GPS longitude, GPS/Barometer altitude */
                    double baroalt,
                    float p, float q, float r,          /* Gyro P, Q and R  */
                    float ax, float ay, float az,       /* Accelarometer X, Y and Z */
                    float hx, float hy, float hz        /* Magnetometer HX, HY, HZ */ );

    ExtendedKalmanFilter *ekf = nullptr;
    ExtendedKalmanFilter_quart *ekf_quart = nullptr;
    int m_use_gpt = 1;

    // returns the pitch angle, rad
    float getPitch_rad(){
        if(m_use_gpt==2) return ekf_quart->getPitch();
        else          return ekf->state[6];
    }
    // returns the roll angle, rad
    float getRoll_rad(){
        if(m_use_gpt==2) return ekf_quart->getRoll();
        else          return ekf->state[7];
    }
    // returns the heading angle, rad
    float getHeading_rad(){
        if(m_use_gpt==2) return ekf_quart->getYaw();
        else          return ekf->state[8];
    }
    // returns the INS latitude, rad
    double getLatitude_rad(){
        if(m_use_gpt==2) return ekf_quart->state[0];
        else          return ekf->state[0];
    }
    // returns the INS longitude, rad
    double getLongitude_rad(){
        if(m_use_gpt==2) return ekf_quart->state[1];
        else          return ekf->state[1];
    }
    // returns the INS altitude, m
    double getAltitude_m(){
        if(m_use_gpt==2) return ekf_quart->state[2];
        else          return ekf->state[2];
    }
    // returns the INS north velocity, m/s
    double getVelNorth_ms(){
        if(m_use_gpt==2) return ekf_quart->state[3];
        else          return ekf->state[3];
    }
    // returns the INS east velocity, m/s
    double getVelEast_ms(){
        if(m_use_gpt==2) return ekf_quart->state[4];
        else          return ekf->state[4];
    }
    // returns the INS down velocity, m/s
    double getVelDown_ms(){
        if(m_use_gpt==2) return ekf_quart->state[5];
        else          return ekf->state[5];
    }

    // return pitch, roll and yaw
    std::tuple<float,float,float> getPitchRoll(float ax, float ay, float az, float Gcal);
    std::tuple<float,float,float> getYaw(float ax, float ay, float hx, float hy, float hz, float Gcal);

    // Use Kalmanfilter of build in function...
    bool initialized_ = false;
    double Gval = Gfix;

  private:
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////// member variables /////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////

    double m_dt = 0.1;

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

