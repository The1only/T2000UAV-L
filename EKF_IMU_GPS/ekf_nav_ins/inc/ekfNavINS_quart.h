// Quaternion-based EKF Orientation Filter (Partial Rewrite of Your Code)
// Focused on orientation part: state includes quaternion instead of roll/pitch/yaw

#ifndef EKFNAVINS_QUART_H
#define EKFNAVINS_QUART_H

#pragma once

#include <QCoreApplication>
#include <math.h>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry>

using namespace Eigen;

#define DEG_TO_RAD (3.141592/180)
#define RAD_TO_DEG (180.0/3.141592765)

// acceleration due to gravity
constexpr float Gfix = 9.82500f; //9.808f;

// earth semi-major axis radius (m)
constexpr double EARTH_RADIUS = 6378137.0;

struct ExtendedKalmanFilter_quart {
    VectorXd state;    // State vector: [pos(3), vel(3), quat(4), accel_bias(3), gyro_bias(3)]
    MatrixXd P;        // Covariance
    MatrixXd Q;        // Process noise
    MatrixXd Rco;        // Measurement noise

    double Gval = Gfix; // Gravity
    Vector3d gps_velocity_prev = Vector3d::Zero(); // For GPS acceleration estimation

    ExtendedKalmanFilter_quart() {
        state = VectorXd::Zero(16); // 3+3+4+3+3 = 16
        state.segment<4>(6) = Vector4d(1, 0, 0, 0); // Initial quaternion

        P = MatrixXd::Identity(16, 16) * 1e-3;

        Q = MatrixXd::Zero(16, 16);
        Q.diagonal() << 1e-4, 1e-4, 1e-4,
                        1e-2, 1e-2, 1e-2,
                        1e-3, 1e-3, 1e-3, 1e-3,
                        1e-5, 1e-5, 1e-5,
                        1e-5, 1e-5, 1e-5;

 //       R = MatrixXd::Identity(9, 9) * 1e-2; // Example: GPS+Baro+Accel for now

        // Measurement noise covariance matrix
        Rco = MatrixXd::Zero(16, 16);

        // Set diagonal elements of R explicitly
        Rco(0, 0) = 1e-2; // GPS x noise
        Rco(1, 1) = 1e-2; // GPS y noise
        Rco(2, 2) = 1e-2; // GPS z noise
        Rco(3, 3) = 1e-2; // GPS vx noise
        Rco(4, 4) = 1e-2; // GPS vy noise
        Rco(5, 5) = 1e-2; // GPS vz noise
        Rco(6, 6) = 1e-1; // Barometer noise
        Rco(7, 7) = 0.03; //1e-2; // Accelerometer quat. 0.04 is slow, 0.01 is faster 0.06 is too slow...
        Rco(8, 8) = 0.03; //1e-2; // Accelerometer quat.
        Rco(9, 9) = 0.03; //1e-2; // Accelerometer quat.
        Rco(10,10)= 0.03; //1e-2; // Accelerometer quat.
        Rco(11,11)= 1e-3; // 1e-1 Magnetometer x noise
        Rco(12,12)= 1e-3; // 1e-1 Magnetometer y noise
        Rco(13,13)= 1e-3; // 1e-1 Magnetometer z noise
    }

    Quaterniond getQuat() const {
        return Quaterniond(state(6), state(7), state(8), state(9));
    }

    void normalizeQuat() {
        Vector4d q = state.segment<4>(6);
        q.normalize();
        state.segment<4>(6) = q;
    }

    // GPS to Body Frame Acceleration Correction
    Vector3d getGPSAccelBody(const Vector3d& gps_velocity_now, double dt) {
        if (dt <= 0.0001) return Vector3d::Zero();
        Vector3d gps_accel_ned = (gps_velocity_now - gps_velocity_prev) / dt;
        gps_velocity_prev = gps_velocity_now;
        Quaterniond q = getQuat();
        Vector3d gps_accel_body = q.conjugate() * gps_accel_ned;
        return gps_accel_body;
    }

    VectorXd f(const VectorXd &state, const Vector3d &gyro, const Vector3d &accel, double dt) {
        VectorXd newState = state;
        Quaterniond q(state(6), state(7), state(8), state(9));
        q.normalize();

        Vector3d gyro_corr = gyro - state.segment<3>(13);
        double omega_mag = gyro_corr.norm();

        Quaterniond dq;
        if (omega_mag > 1e-6) {
            Vector3d axis = gyro_corr.normalized();
            dq = AngleAxisd(omega_mag * dt, axis);
        } else {
            dq = Quaterniond::Identity();
        }

        Quaterniond q_new = q * dq;
        q_new.normalize();

        // Predict velocity from accel (in world frame)
        Vector3d acc_body = accel - state.segment<3>(10);
        Vector3d acc_world = q_new * acc_body + Vector3d(0, 0, -Gval);

        // Position and velocity update
        newState.segment<3>(0) += state.segment<3>(3) * dt + 0.5 * acc_world * dt * dt;
        newState.segment<3>(3) += acc_world * dt;

        newState.segment<4>(6) = Vector4d(q_new.w(), q_new.x(), q_new.y(), q_new.z());
        return newState;
    }

    // Non-linear measurement function
    VectorXd h(const VectorXd &state) {
        VectorXd measurements(16);
        measurements.segment<3>(0) = state.segment<3>(0); // Estimated GPS position from velocity...
        measurements.segment<3>(3) = state.segment<3>(3); // GPS velocity
        measurements(6) = state(2);                      // Barometer altitude
        measurements.segment<4>(7) = state.segment<4>(6); // Accelerometer orientation
        measurements.segment<3>(11) = state.segment<3>(10); // Magnetometer orientation
        measurements.segment<2>(14) = Vector2d(0, 0);       // Correction
        return measurements;
    }

    // Jacobian of the measurement function
    MatrixXd H_jacobian(const VectorXd &state) {
        MatrixXd H = MatrixXd::Zero(16, 16);
        H.block<3, 3>(0, 0) = Matrix3d::Identity(); // GPS position measurements
        H.block<3, 3>(3, 3) = Matrix3d::Identity(); // GPS velocity measurements
        H(6, 2) = 1.0;                             // Barometer altitude measurement
        H.block<4, 4>(7, 6) = Matrix4d::Identity(); // Accelerometer orientation measurements
        H.block<3, 3>(11, 6) = Matrix3d::Identity(); // Magnetometer orientation measurements
        return H;
    }

    Eigen::Quaterniond eulerToQuaternion(double roll, double pitch, double yaw) {
        // Note: Rotation order is ZYX (Yaw → Pitch → Roll)
        Eigen::AngleAxisd rollAngle(roll,   Eigen::Vector3d::UnitX());
        Eigen::AngleAxisd pitchAngle(pitch, Eigen::Vector3d::UnitY());
        Eigen::AngleAxisd yawAngle(yaw,     Eigen::Vector3d::UnitZ());

        Eigen::Quaterniond q = yawAngle * pitchAngle * rollAngle;
        q.normalize();
        return q;
    }

    void predict(double dt, const Vector3d &gyro, const Vector3d &accel)
    {
        state = f(state, gyro, accel, dt);
        normalizeQuat();
        MatrixXd F = MatrixXd::Identity(16, 16); // Placeholder
        P = F * P * F.transpose() + Q;
    }

    void update(const VectorXd &measurements)
    {
        /*
        Vector3d gps_velocity_now;
        float dt;
        Vector3d imuAccel = Vector3d(measurements[7],measurements[8],measurements[9]);
        Vector3d acc_body = imuAccel - getGPSAccelBody(gps_velocity_now, dt);
        Eigen::Quaterniond q = eulerToQuaternion(acc_body[0],acc_body[1],acc_body[2]);
        */
        Eigen::Quaterniond q = eulerToQuaternion(measurements[7],measurements[8],measurements[9]);
        Eigen::Vector4d q_vec;
        q_vec << q.w(), q.x(), q.y(), q.z();  // [w, x, y, z] order

        VectorXd measure = VectorXd::Zero(16);
        measure.segment<3>(0)  = measurements.segment<3>(0); // GPS position
        measure.segment<3>(3)  = measurements.segment<3>(3);   // GPS velocity
        measure(6)             = measurements[6];                // Barometer altitude
        measure.segment<4>(7)  = q_vec; // Accelerometer orientation
        measure.segment<3>(11) = measurements.segment<3>(10); // Magnetometer orientation
        measure.segment<2>(14) = Vector2d(0, 0 );    // Correction

        // Compute the expected measurement
        VectorXd y = measure - h(state);

        // Compute the Jacobian of the measurement function
        MatrixXd H = H_jacobian(state);
        MatrixXd S = H * P * H.transpose() + Rco;
        S += MatrixXd::Identity(S.rows(), S.cols()) * 1e-9; // Regularization term

        MatrixXd K = P * H.transpose() * S.inverse();
        state = state + K * y;
        P = (MatrixXd::Identity(16, 16) - K * H) * P;
        normalizeQuat();
    }

    // ------------------------------------------------
    // GPS ground speed... GPS bearing in radians...
    Vector3d getCompensatedAccel(double speed, double bearing, Vector3d imuAccel, double dt)
    {
        Vector3d gps_velocity_ned;
        gps_velocity_ned << speed * cos(bearing), // North
                            speed * sin(bearing), // East
                            0.0;

        Vector3d gps_velocity_body = getQuat().conjugate() * gps_velocity_ned;
        Vector3d corrected_accel = imuAccel - gps_velocity_body; // approximate inertial effect
        return corrected_accel;
    }
    double getYaw() const {
        Quaterniond q = getQuat();
        Matrix3d Rx = q.toRotationMatrix();
        return atan2(Rx(1, 0), Rx(0, 0)); // Yaw from rotation matrix
    }
    double getRoll() const {
        Quaterniond q = getQuat();
        Matrix3d Rx = q.toRotationMatrix();
        return atan2(-Rx(2, 0), sqrt(Rx(2, 1)*Rx(2, 1) + Rx(2, 2)*Rx(2, 2)));
    }
    double getPitch() const {
        Quaterniond q = getQuat();
        Matrix3d Rx = q.toRotationMatrix();
        return atan2(Rx(2, 1), Rx(2, 2));
    }
};


#endif // EKFGNAVINS_QUART_H
