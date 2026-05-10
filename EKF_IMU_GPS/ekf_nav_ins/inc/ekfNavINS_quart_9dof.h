#ifndef EKFNAVINS_QUART_9DOF_H
#define EKFNAVINS_QUART_9DOF_H

// Quaternion-based EKF Orientation Filter (Updated: GPS acceleration correction + gravity alignment correction using accelerometer in update step + clarified accel + test function)
// Quaternion-based EKF Orientation Filter (Fully enhanced: 9DOF support with GPS + Barometer + Magnetometer integration)
// Detailed comments included throughout for educational clarity and maintainability

#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <iostream>

using namespace Eigen;

// Extended Kalman Filter with orientation represented using quaternions
struct QuaternionEKF {
    VectorXd state;    // State vector: [position(3), velocity(3), quaternion(4), accel_bias(3), gyro_bias(3)]
    MatrixXd P;        // Covariance matrix (16x16)
    MatrixXd Q;        // Process noise covariance matrix (16x16)
    MatrixXd R;        // Measurement noise covariance matrix (10x10)

    double Gval = 9.825; // Acceleration due to gravity [m/s^2]
    Vector3d gps_velocity_prev = Vector3d::Zero(); // Previous GPS velocity for acceleration estimation

    // Constructor to initialize the filter
    QuaternionEKF() {
        state = VectorXd::Zero(16); // Initialize state vector with zeros
        state.segment<4>(6) = Vector4d(1, 0, 0, 0); // Set quaternion to identity [w, x, y, z]

        // Initialize covariance and noise matrices
        P = MatrixXd::Identity(16, 16) * 1e-3;
        Q = MatrixXd::Identity(16, 16) * 1e-4;

        // Measurement noise for: GPS pos(3), GPS vel(3), Baro(1), Magnetometer(3)
        R = MatrixXd::Zero(10, 10);
        R.block<3,3>(0,0) = Matrix3d::Identity() * 0.5;  // GPS position noise
        R.block<3,3>(3,3) = Matrix3d::Identity() * 0.2;  // GPS velocity noise
        R(6,6) = 1.0;                                   // Barometer altitude noise
        R.block<3,3>(7,7) = Matrix3d::Identity() * 0.3;  // Magnetometer noise
    }

    // Extract current orientation as quaternion
    Quaterniond getQuat() const {
        return Quaterniond(state(6), state(7), state(8), state(9));
    }

    // Normalize the quaternion portion of the state vector
    void normalizeQuat() {
        Vector4d q = state.segment<4>(6);
        q.normalize();
        state.segment<4>(6) = q;
    }

    // Calculate GPS-derived acceleration in body frame using previous and current velocity
    Vector3d getGPSAccelBody(const Vector3d& gps_velocity_now, double dt) {
        if (dt <= 0.0001) return Vector3d::Zero();
        Vector3d gps_accel_ned = (gps_velocity_now - gps_velocity_prev) / dt;
        gps_velocity_prev = gps_velocity_now;
        Quaterniond q = getQuat();
        return q.conjugate() * gps_accel_ned;
    }

    // State prediction model
    VectorXd f(const VectorXd &state, const Vector3d &gyro, const Vector3d &imuAccel, double dt, const Vector3d &gps_velocity) {
        VectorXd newState = state;
        Quaterniond q(state(6), state(7), state(8), state(9));
        q.normalize();

        // Remove gyro bias and integrate orientation
        Vector3d gyro_corr = gyro - state.segment<3>(13);
        double omega_mag = gyro_corr.norm();
        Quaterniond dq = (omega_mag > 1e-6) ? Quaterniond(AngleAxisd(omega_mag * dt, gyro_corr.normalized())) : Quaterniond::Identity();

        Quaterniond q_new = q * dq;
        q_new.normalize();

        // Compute corrected acceleration
        Vector3d acc_body = imuAccel - state.segment<3>(10);
        Vector3d gps_accel_body = getGPSAccelBody(gps_velocity, dt);
        Vector3d corrected_accel = acc_body - gps_accel_body;
        Vector3d acc_world = q_new * corrected_accel + Vector3d(0, 0, -Gval);

        // Update position and velocity using physics integration
        newState.segment<3>(0) += state.segment<3>(3) * dt + 0.5 * acc_world * dt * dt;
        newState.segment<3>(3) += acc_world * dt;
        newState.segment<4>(6) = Vector4d(q_new.w(), q_new.x(), q_new.y(), q_new.z());

        return newState;
    }

    // Use accelerometer to align orientation with gravity
    void updateOrientationUsingAccel(const Vector3d &imuAccel) {
        Quaterniond q = getQuat();
        Vector3d gravity_world(0, 0, -1);
        Vector3d acc_body = imuAccel - state.segment<3>(10); // Remove BIAS
        Vector3d acc_normalized = acc_body.normalized();     // Normalize (summ = 1)
        Vector3d gravity_est_body = q.conjugate() * gravity_world; // invers rotat world
        Vector3d gravity_error = acc_normalized.cross(gravity_est_body); // find the diff. of the two.

        double gain = 0.05;
        Vector3d delta_theta = gain * gravity_error;

        if (delta_theta.norm() > 1e-6) {
            Quaterniond dq;
            dq.w() = 1.0;
            dq.vec() = 0.5 * delta_theta;
            dq.normalize();
            q = dq * q;
            q.normalize();
            state.segment<4>(6) = Vector4d(q.w(), q.x(), q.y(), q.z());
        }
    }

    // ----------------------------------------------------
    // EKF prediction step
    void predict(double dt, const Vector3d &gyro, const Vector3d &imuAccel, const Vector3d &gps_velocity) {
        state = f(state, gyro, imuAccel, dt, gps_velocity);
        normalizeQuat();
        MatrixXd F = MatrixXd::Identity(16, 16); // Placeholder for Jacobian
        P = F * P * F.transpose() + Q;
    }

    // Full 9DOF update step using all available sensors
    void updateFull9DOF(const Vector3d &gps_position, const Vector3d &gps_velocity, double baro_alt, const Vector3d &mag, const Vector3d &imuAccel) {
        VectorXd z(10);
        z << gps_position, gps_velocity, baro_alt, mag;

        // Predicted measurements from current state
        VectorXd z_expected(10);
        z_expected.segment<3>(0) = state.segment<3>(0);
        z_expected.segment<3>(3) = state.segment<3>(3);
        z_expected(6) = state(2);

        // Predict magnetic field in body frame (assumed world field = (1,0,0))
        Vector3d mag_ned(1, 0, 0);
        Vector3d mag_expected = getQuat().conjugate() * mag_ned;
        z_expected.segment<3>(7) = mag_expected;

        // Measurement Jacobian matrix
        MatrixXd H = MatrixXd::Zero(10, 16);
        H.block<3,3>(0,0) = Matrix3d::Identity(); // GPS position
        H.block<3,3>(3,3) = Matrix3d::Identity(); // GPS velocity
        H(6,2) = 1.0;                             // Barometer altitude

        // Kalman update
        VectorXd y = z - z_expected;
        MatrixXd S = H * P * H.transpose() + R;
        MatrixXd K = P * H.transpose() * S.inverse();
        state = state + K * y;
        P = (MatrixXd::Identity(16, 16) - K * H) * P;
        normalizeQuat();

        // Final orientation correction using accelerometer
        updateOrientationUsingAccel(imuAccel);
    }

    // Retrieve orientation as Euler angles (ZYX order)
    double getYaw() const   { return getQuat().toRotationMatrix().eulerAngles(2, 1, 0)[0]; }
    double getPitch() const { return getQuat().toRotationMatrix().eulerAngles(2, 1, 0)[1]; }
    double getRoll() const  { return getQuat().toRotationMatrix().eulerAngles(2, 1, 0)[2]; }

    // Test run: simulate one EKF step
    void testEKFStep() {
        Vector3d dummyGyro(0.01, 0.02, 0.005);
        Vector3d dummyAccel(0.0, 0.0, -9.81);
        Vector3d dummyGPSVel(1.0, 0.0, 0.0);
        Vector3d dummyGPSPos(10.0, 5.0, 100.0);
        double dummyBaro = 100.0;
        Vector3d dummyMag(1.0, 0.0, 0.0);

        predict(0.01, dummyGyro, dummyAccel, dummyGPSVel);
        updateFull9DOF(dummyGPSPos, dummyGPSVel, dummyBaro, dummyMag, dummyAccel);

        std::cout << "Test Step Complete\n";
        std::cout << "Yaw:   " << getYaw() * 180.0 / M_PI << " deg\n";
        std::cout << "Pitch: " << getPitch() * 180.0 / M_PI << " deg\n";
        std::cout << "Roll:  " << getRoll() * 180.0 / M_PI << " deg\n";
    }
};

#endif // EKFNAVINS_QUART_9DOF_H
