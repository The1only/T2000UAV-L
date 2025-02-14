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

#define DEG_TO_RAD (3.141592/180)
#define RAD_TO_DEG (180.0/3.141592765)

constexpr float SIG_W_A = 0.05f;
// Std dev of gyro output noise (rad/s)
constexpr float SIG_W_G = 0.00175f;
// Std dev of Accelerometer Markov Bias
constexpr float SIG_A_D = 0.01f;
// Correlation time or time constant
constexpr float TAU_A = 100.0f;
// Std dev of correlated gyro bias
constexpr float SIG_G_D = 0.00025;
// Correlati1on time or time constant
constexpr float TAU_G = 50.0f;
// GPS measurement noise std dev (m)
constexpr float SIG_GPS_P_NE = 3.0f;
constexpr float SIG_GPS_P_D = 6.0f;
// GPS measurement noise std dev (m/s)
constexpr float SIG_GPS_V_NE = 0.5f;
constexpr float SIG_GPS_V_D = 1.0f;
// Initial set of covariance
constexpr float P_P_INIT = 10.0f;
constexpr float P_V_INIT = 1.0f;
constexpr float P_A_INIT = 0.34906f;
constexpr float P_HDG_INIT = 3.14159f;
constexpr float P_AB_INIT = 0.9810f;
constexpr float P_GB_INIT = 0.01745f;
// acceleration due to gravity
constexpr float G = 9.808f;
//constexpr float G = 9.807f;
// major eccentricity squared
constexpr double ECC2 = 0.0066943799901;
// earth semi-major axis radius (m)
constexpr double EARTH_RADIUS = 6378137.0;

class gpsCoordinate {
    public:
        double lat;
        double lon;
        double alt;
};

class gpsVelocity {
    public:
        double vN;
        double vE;
        double vD;
};

class imuData {
    public:
        float gyroX;
        float gyroY;
        float gyroZ;
        float accX;
        float accY;
        float accZ;
        float hX;
        float hY;
        float hZ;
};

class ekfNavINS {
  public:
    // ekf_update
    void ekf_update( uint64_t time/*, unsigned long TOW*/,   /* Time, Time of the week from GPS */
                    double vn, double ve, double vd,    /* Velocity North, Velocity East, Velocity Down */
                    double lat, double lon, double alt, /* GPS latitude, GPS longitude, GPS/Barometer altitude */
                    float p, float q, float r,          /* Gyro P, Q and R  */
                    float ax, float ay, float az,       /* Accelarometer X, Y and Z */
                    float hx, float hy, float hz        /* Magnetometer HX, HY, HZ */ );
    // returns whether the INS has been initialized
    bool initialized()          { return initialized_; }

    // returns the pitch angle, rad
    void setPitch_rad(float x) { theta = x ;}
    // returns the roll angle, rad
    void setRoll_rad(float x) { phi = x ;}
    // returns the heading angle, rad
    void setHeading_rad(float x) { psi = x ;}
    // returns the INS latitude, rad


    // returns the pitch angle, rad
    float getPitch_rad()        { return theta; }
    // returns the roll angle, rad
    float getRoll_rad()         { return phi; }
    // returns the heading angle, rad
    float getHeadingConstrainAngle180_rad()      { return constrainAngle180(psi); }
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
    std::tuple<float,float,float> getPitchRollYaw(float ax, float ay, float az, float hx, float hy, float hz, float gx, float gy, float gz); //
//    std::tuple<float,float,float> getPitchRollYaw(float ax, float ay, float az, float hx, float hy, float hz);
    void imuUpdateEKF(uint64_t time, imuData imu);
    void gpsCoordinateUpdateEKF(gpsCoordinate coor);
    void gpsVelocityUpdateEKF(gpsVelocity vel);

  private:
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////// member variables /////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    gpsCoordinate gpsCoor;
    gpsVelocity   gpsVel;
    imuData       imuDat;
    mutable std::shared_mutex shMutex;
    // initialized
    bool initialized_ = false;

    bool m_use_gpt = true;

    double m_dt = 0.1;
    // timing
    uint64_t _tprev;
    //float _dt;
    unsigned long previousTOW;
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
    // earth radius at location
    double Re, Rn, denom;
    // State matrix
    Eigen::Matrix<float,15,15> Fs = Eigen::Matrix<float,15,15>::Identity();
    // State transition matrix
    Eigen::Matrix<float,15,15> PHI = Eigen::Matrix<float,15,15>::Zero();
    // Covariance matrix
    Eigen::Matrix<float,15,15> P = Eigen::Matrix<float,15,15>::Zero();
    // For process noise transformation
    Eigen::Matrix<float,15,12> Gs = Eigen::Matrix<float,15,12>::Zero();
    Eigen::Matrix<float,12,12> Rw = Eigen::Matrix<float,12,12>::Zero();
    // Process noise matrix
    Eigen::Matrix<float,15,15> Q = Eigen::Matrix<float,15,15>::Zero();
    // Gravity model
    Eigen::Matrix<float,3,1> grav = Eigen::Matrix<float,3,1>::Zero();
    // Rotation rate
    Eigen::Matrix<float,3,1> om_ib = Eigen::Matrix<float,3,1>::Zero();
    // Specific force
    Eigen::Matrix<float,3,1> f_b = Eigen::Matrix<float,3,1>::Zero();
    // DCM
    Eigen::Matrix<float,3,3> C_N2B = Eigen::Matrix<float,3,3>::Zero();
    // DCM transpose
    Eigen::Matrix<float,3,3> C_B2N = Eigen::Matrix<float,3,3>::Zero();
    // Temporary to get dxdt
    Eigen::Matrix<float,3,1> dx = Eigen::Matrix<float,3,1>::Zero();
    Eigen::Matrix<double,3,1> dxd = Eigen::Matrix<double,3,1>::Zero();
    // Estimated INS
    Eigen::Matrix<double,3,1> estmimated_ins = Eigen::Matrix<double,3,1>::Zero();
    // NED velocity INS
    Eigen::Matrix<double,3,1> V_ins = Eigen::Matrix<double,3,1>::Zero();
    // LLA INS
    Eigen::Matrix<double,3,1> lla_ins = Eigen::Matrix<double,3,1>::Zero();
    // NED velocity GPS
    Eigen::Matrix<double,3,1> V_gps = Eigen::Matrix<double,3,1>::Zero();
    // LLA GPS
    Eigen::Matrix<double,3,1> lla_gps = Eigen::Matrix<double,3,1>::Zero();
    // Position ECEF INS
    Eigen::Matrix<double,3,1> pos_ecef_ins = Eigen::Matrix<double,3,1>::Zero();
    // Position NED INS
    Eigen::Matrix<double,3,1> pos_ned_ins = Eigen::Matrix<double,3,1>::Zero();
    // Position ECEF GPS
    Eigen::Matrix<double,3,1> pos_ecef_gps = Eigen::Matrix<double,3,1>::Zero();
    // Position NED GPS
    Eigen::Matrix<double,3,1> pos_ned_gps = Eigen::Matrix<double,3,1>::Zero();
    // Quat
    Eigen::Matrix<float,4,1> quat = Eigen::Matrix<float,4,1>::Zero();
    // dquat
    Eigen::Matrix<float,4,1> dq = Eigen::Matrix<float,4,1>::Zero();
    // difference between GPS and INS
    Eigen::Matrix<float,6,1> y = Eigen::Matrix<float,6,1>::Zero();
    // GPS measurement noise
    Eigen::Matrix<float,6,6> R = Eigen::Matrix<float,6,6>::Zero();
    Eigen::Matrix<float,15,1> x = Eigen::Matrix<float,15,1>::Zero();
    // Kalman Gain
    Eigen::Matrix<float,15,6> K = Eigen::Matrix<float,15,6>::Zero();
    Eigen::Matrix<float,6,15> H = Eigen::Matrix<float,6,15>::Zero();
    // skew symmetric
    Eigen::Matrix<float,3,3> sk(Eigen::Matrix<float,3,1> w);

    //////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////// member functions /////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    // ekf_init
    void ekf_init(uint64_t time, 
                 double vn,double ve,double vd, 
                 double lat,double lon,double alt,
                 float p,float q,float r,
                 float ax,float ay,float az,
                 float hx,float hy, float hz);

    void ekf_initGPT(uint64_t time,
                  double vn,double ve,double vd,
                  double lat,double lon,double alt,
                  float p,float q,float r,
                  float ax,float ay,float az,
                  float hx,float hy, float hz);
    // lla rate
    Eigen::Matrix<double,3,1> llarate(Eigen::Matrix<double,3,1> V, Eigen::Matrix<double,3,1> lla);
    Eigen::Matrix<double,3,1> llarate(Eigen::Matrix<double,3,1> V, double lat, double alt);
    // lla to ecef
    Eigen::Matrix<double,3,1> lla2ecef(Eigen::Matrix<double,3,1> lla);
    // ecef to ned
    Eigen::Matrix<double,3,1> ecef2ned(Eigen::Matrix<double,3,1> ecef, Eigen::Matrix<double,3,1> pos_ref);
    // quaternion to dcm
    Eigen::Matrix<float,3,3> quat2dcm(Eigen::Matrix<float,4,1> q);
    // quaternion multiplication
    Eigen::Matrix<float,4,1> qmult(Eigen::Matrix<float,4,1> p, Eigen::Matrix<float,4,1> q);
    // maps angle to +/- 180
    float constrainAngle180(float dta);
    // maps angle to 0-360
    float constrainAngle360(float dta);
    // Returns Radius - East West and Radius - North South
    constexpr std::pair<double, double> earthradius(double lat);
    // Yaw, Pitch, Roll to Quarternion
    Eigen::Matrix<float,4,1> toQuaternion(float yaw, float pitch, float roll);
    // Quarternion to Yaw, Pitch, Roll
    std::tuple<float, float, float> toEulerAngles(Eigen::Matrix<float,4,1> quat);
    // Update Jacobian matrix
    void updateJacobianMatrix();
    // Update Process Noise and Covariance Time
    void updateProcessNoiseCovarianceTime(float _dt);
    // Update Gyro and Accelerometer Bias
    void updateBias(float ax,float ay,float az,float p,float q, float r);
    // Update 15 states after KF state update
    void update15statesAfterKF();
    // Update differece between predicted and calculated GPS and IMU values
    void updateCalculatedVsPredicted();
    void ekf_update(uint64_t time);
    void updateINS();
};


class KalmanFilterIMU {
private:
    Eigen::VectorXd x;  // State vector  [x, y, z, vx, vy, vz, qw, qx, qy, qz, bx, by, bz, gbx, gby, gbz]
    Eigen::MatrixXd P;  // State covariance
    Eigen::MatrixXd Q;  // Process noise covariance
    Eigen::MatrixXd R;  // Measurement noise covariance
    double dt;

public:
    KalmanFilterIMU(double delta_t = 0.1) : dt(delta_t) {
        x.resize(16);
        x.setZero();
        x(6) = 1.0; // Quaternion w component

        P.resize(16, 16);
        Q.resize(16, 16);
        R.resize(6, 6);

        P.setIdentity();
        Q.setIdentity() *= 0.01;  // Process noise covariance
        R.setIdentity() *= 0.1;   // Measurement noise covariance
    }

    Eigen::Matrix3d quaternionToRotation(const Eigen::Vector4d& q) {
        Eigen::Matrix3d R;
        double qw = q(0), qx = q(1), qy = q(2), qz = q(3);

        R << 1 - 2*qy*qy - 2*qz*qz,     2*qx*qy - 2*qz*qw,      2*qx*qz + 2*qy*qw,
            2*qx*qy + 2*qz*qw,         1 - 2*qx*qx - 2*qz*qz,  2*qy*qz - 2*qx*qw,
            2*qx*qz - 2*qy*qw,         2*qy*qz + 2*qx*qw,      1 - 2*qx*qx - 2*qy*qy;
        return R;
    }

    void predict(const Eigen::Vector3d& gyro) {
        Eigen::Vector4d q(x(6), x(7), x(8), x(9));
        Eigen::Vector3d gyro_bias(x(13), x(14), x(15));
        Eigen::Vector3d omega = gyro - gyro_bias;

        // Evolve rotation quaternion
        Eigen::Matrix4d Omega;
        Omega << 0,         -omega(0),      -omega(1),      -omega(2),
            omega(0),   0,              omega(2),       -omega(1),
            omega(1),   -omega(2),      0,              omega(0),
            omega(2),   omega(1),       -omega(0),      0;

        q += 0.5 * dt * (Omega * q);
        q.normalize();

        // Update state...
        Eigen::Vector3d pos = x.segment<3>(0);
        Eigen::Vector3d vel = x.segment<3>(3);
        pos += vel * dt;

        x.segment<3>(0) = pos;
        x.segment<4>(6) = q;

        Eigen::MatrixXd F = Eigen::MatrixXd::Identity(16, 16);
        F.block<3,3>(0,3) = Eigen::Matrix3d::Identity() * dt;

        // Covariance update
        P = F * P * F.transpose() + Q;
    }

    void update(const Eigen::Vector3d& accel, const Eigen::Vector3d& mag) {
        Eigen::Vector3d g(0, 0, -9.81);
        // These specific magnetic field reference values (0.22, 0, -0.44) are chosen to approximate the Earth's typical magnetic field vector
        // at a specific location, likely near the surface at mid-latitudes.
        Eigen::Vector3d m_ref(0.22, 0, -0.44);

        Eigen::Vector4d q(x(6), x(7), x(8), x(9));
        Eigen::Matrix3d rot = quaternionToRotation(q);

        Eigen::Vector3d accel_pred = rot.transpose() * g;
        Eigen::Vector3d mag_pred = rot.transpose() * m_ref;

        Eigen::VectorXd z(6), z_pred(6);
        z << accel, mag;
        z_pred << accel_pred, mag_pred;

        Eigen::MatrixXd H = Eigen::MatrixXd::Zero(6, 16);
        H.block<3,3>(0,6) = Eigen::Matrix3d::Identity();
        H.block<3,3>(3,6) = Eigen::Matrix3d::Identity();

        Eigen::MatrixXd S = H * P * H.transpose() + R;
        Eigen::MatrixXd K = P * H.transpose() * S.inverse();

        x += K * (z - z_pred);
        P = (Eigen::MatrixXd::Identity(16, 16) - K * H) * P;

        q = x.segment<4>(6);
        q.normalize();
        x.segment<4>(6) = q;
    }

    // Utility function to convert quaternion to Euler angles
    Eigen::Vector3d quaternionToEuler(const Eigen::Vector4d& q) {
        double qw = q(0), qx = q(1), qy = q(2), qz = q(3);

        // Roll (x-axis rotation)
        double sinr_cosp = 2 * (qw * qx + qy * qz);
        double cosr_cosp = 1 - 2 * (qx * qx + qy * qy);
        double roll = std::atan2(sinr_cosp, cosr_cosp);

        // Pitch (y-axis rotation)
        double sinp = 2 * (qw * qy - qz * qx);
        double pitch;
        if (std::abs(sinp) >= 1)
            pitch = std::copysign(M_PI / 2, sinp); // use 90 degrees if out of range
        else
            pitch = std::asin(sinp);

        // Yaw (z-axis rotation)
        double siny_cosp = 2 * (qw * qz + qx * qy);
        double cosy_cosp = 1 - 2 * (qy * qy + qz * qz);
        double yaw = std::atan2(siny_cosp, cosy_cosp);

        // Convert to degrees
        return Eigen::Vector3d(
            roll * 180.0 / M_PI,
            pitch * 180.0 / M_PI,
            yaw * 180.0 / M_PI
            );
    }


    Eigen::Vector3d getPosition() const { return x.segment<3>(0); }
    Eigen::Vector3d getVelocity() const { return x.segment<3>(3); }
    Eigen::Vector4d getQuaternion() const { return x.segment<4>(6); }
    void getQuaternion(Eigen::Vector4d q){
        q.normalize();
        x.segment<4>(6) = q;
    }
};


