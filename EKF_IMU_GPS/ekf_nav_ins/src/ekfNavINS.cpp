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
        R(7, 7) = 1e-2; // Accelerometer roll noise
        R(8, 8) = 1e-2; // Accelerometer pitch noise
        R(9, 9) = 1e-2; // Accelerometer yaw noise
        R(10, 10) = 1e-3; // Magnetometer x noise
        R(11, 11) = 1e-3; // Magnetometer y noise
        R(12, 12) = 1e-3; // Magnetometer z noise
    }

    // Non-linear state transition function
    VectorXd f(const VectorXd &state, const Vector3d &gyro, double dt) {
        VectorXd newState = state;
        newState.segment<3>(0) += state.segment<3>(3) * dt; // Update position
        newState.segment<3>(3) += Vector3d(0, 0, -9.81) * dt; // Gravity effect on velocity
        newState.segment<3>(6) += (gyro - state.segment<3>(12)) * dt; // Update orientation (roll, pitch, yaw) considering gyro bias
        newState.segment<3>(9) = state.segment<3>(9); // Accelerometer bias remains constant
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
        measurements.segment<3>(0) = state.segment<3>(0); // GPS position
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
    void predict(double dt, const Vector3d &gyro) {
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

static ExtendedKalmanFilter ekf;

void ekfNavINS::ekf_initGPT(uint64_t time, double vn,double ve,double vd,double lat,double lon,double alt,float p,float q,float r,float ax,float ay,float az,float hx,float hy, float hz)
{

    if (!initialized_) {
        m_dt = 0.1;
        // Initialize Extended Kalman filter
//        ekf = new ExtendedKalmanFilter();
        initialized_ = true;
    }


    // Example sensor data (add more data points as needed)
    std::vector<SensorData> sensorData = {
        {
            Vector3d(ax, ay, az),  // Accelerometer readings
            Vector3d(p, q, r), // Gyroscope readings
            Vector3d(hx, hy, hz),   // Magnetometer readings
            alt,                     // Barometer reading
            Vector3d(lat, lon, alt),      // GPS position readings
            Vector3d(vn, ve, vd)          // GPS velocity readings
        }
    };
 /*
    // Example sensor data (add more data points as needed)
    std::vector<SensorData> sensorData = {
        {
            Vector3d(0.00, 0.00, 9.81),  // Accelerometer readings
            Vector3d(0.0, 0.0, 0.75),   // Magnetometer readings
            Vector3d(0.00, 0.00, 0.00), // Gyroscope readings
            1013.24,                     // Barometer reading
            Vector3d(60.4000, 12.4000, 1013.0),      // GPS position readings
            Vector3d(0, 0, 0)          // GPS velocity readings
        }
    };
*/
    qDebug() << "Accelerometer (x, y, z): (" << ax << ay << az << Qt::endl;
    qDebug() << "Gyroscope (p, q, r): (" << p << q << r << Qt::endl;
    qDebug() << "Magnetometer (x, y, z): (" << hx << hy << hz << Qt::endl;
    qDebug() << "GPS position (x, y, z): (" << lat << lon << alt << Qt::endl;


    for (const auto &data : sensorData) {
        // Construct the measurement vector
        VectorXd measurements(15);
        measurements.segment<3>(0) = data.gps;           // GPS position
        measurements.segment<3>(3) = data.gps_velocity;  // GPS velocity
        measurements(6) = data.barometer;               // Barometer altitude
        measurements.segment<3>(7) = data.accelerometer; // Accelerometer orientation
        measurements.segment<3>(10) = data.magnetometer; // Magnetometer orientation
        measurements.segment<2>(13) = Vector2d(0, 0);                   // Correction

        m_dt = 0.1;
        // Prediction step
        ekf.predict(m_dt, data.gyroscope);

        // Update step
        ekf.update(measurements);

        theta = ekf.state[6];
        phi = ekf.state[7];
        psi = ekf.state[8];
        lat_ins = ekf.state[0];
        lon_ins = ekf.state[1];
        alt_ins = ekf.state[2];
        vn_ins = ekf.state[3];
        ve_ins = ekf.state[4];
        vd_ins = ekf.state[5];

        // Use corrected sensor values for external applications
//        Vector3d corrected_accel = data.accelerometer - ekf->state.segment<3>(9); // Correct accelerometer bias
//        Vector3d corrected_gyro = data.gyroscope - ekf->state.segment<3>(12);     // Correct gyroscope bias

        // Print corrected values for debugging
//        std::cout << "Corrected Accelerometer: " << corrected_accel.transpose() << std::endl;
//        std::cout << "Corrected Gyroscope: " << corrected_gyro.transpose() << std::endl;

        std::cout << "Estimated State:\n" << ekf.state << std::endl;
    }

    // Output the estimated state
//    std::cout << "Final Estimated State:\n" << ekf->state << std::endl;

    qDebug() << "Position (x, y, z): (" << ekf.state[0] << ", " << ekf.state[1] << ", " << ekf.state[2] << ")" << Qt::endl;
    qDebug() << "Velocity (vx, vy, vz): (" << ekf.state[3] << ", " << ekf.state[4] << ", " << ekf.state[5] << ")" << Qt::endl;
    qDebug() << "Orientation (roll, pitch, yaw): (" << ekf.state[6] << ", " << ekf.state[7] << ", " << ekf.state[8] << ")" << Qt::endl;
}




// ...............................................

void ekfNavINS::ekf_init(uint64_t time, double vn,double ve,double vd,double lat,double lon,double alt,float p,float q,float r,float ax,float ay,float az,float hx,float hy, float hz) {
    // grab initial gyro values for biases
    gbx = p;
    gby = q;
    gbz = r;
    std::tie(theta,phi,psi) = getPitchRollYaw(ax, ay, az, hx, hy, hz,p,q,r);
    // euler to quaternion
    quat = toQuaternion(psi, theta, phi);
    // Assemble the matrices
    // ... gravity
    grav(2,0) = G;
    // ... H
    H.block(0,0,5,5) = Eigen::Matrix<float,5,5>::Identity();
    // ... Rw
    Rw.block(0,0,3,3) = powf(SIG_W_A,2.0f) * Eigen::Matrix<float,3,3>::Identity();
    Rw.block(3,3,3,3) = powf(SIG_W_G,2.0f) * Eigen::Matrix<float,3,3>::Identity();
    Rw.block(6,6,3,3) = 2.0f * powf(SIG_A_D,2.0f) / TAU_A*Eigen::Matrix<float,3,3>::Identity();
    Rw.block(9,9,3,3) = 2.0f * powf(SIG_G_D,2.0f) / TAU_G*Eigen::Matrix<float,3,3>::Identity();
    // ... P
    P.block(0,0,3,3) = powf(P_P_INIT,2.0f) * Eigen::Matrix<float,3,3>::Identity();
    P.block(3,3,3,3) = powf(P_V_INIT,2.0f) * Eigen::Matrix<float,3,3>::Identity();
    P.block(6,6,2,2) = powf(P_A_INIT,2.0f) * Eigen::Matrix<float,2,2>::Identity();
    P(8,8) = powf(P_HDG_INIT,2.0f);
    P.block(9,9,3,3) = powf(P_AB_INIT,2.0f) * Eigen::Matrix<float,3,3>::Identity();
    P.block(12,12,3,3) = powf(P_GB_INIT,2.0f) * Eigen::Matrix<float,3,3>::Identity();
    // ... R
    R.block(0,0,2,2) = powf(SIG_GPS_P_NE,2.0f) * Eigen::Matrix<float,2,2>::Identity();
    R(2,2) = powf(SIG_GPS_P_D,2.0f);
    R.block(3,3,2,2) = powf(SIG_GPS_V_NE,2.0f) * Eigen::Matrix<float,2,2>::Identity();
    R(5,5) = powf(SIG_GPS_V_D,2.0f);
    // .. then initialize states with GPS Data
    lat_ins = lat;
    lon_ins = lon;
    alt_ins = alt;
    vn_ins = vn;
    ve_ins = ve;
    vd_ins = vd;
    // specific force
    f_b(0,0) = ax;
    f_b(1,0) = ay;
    f_b(2,0) = az;
    /* initialize the time */
    _tprev = time;
}

void ekfNavINS::ekf_update( uint64_t time/*, unsigned long TOW*/, double vn,double ve,double vd,double lat,double lon,double alt,
                           float p,float q,float r,float ax,float ay,float az,float hx,float hy, float hz ) {
    // get the change in time
    m_dt = ((double)(time - _tprev)) / 1e3;
    qDebug() << "Time: " << m_dt;

    if(m_use_gpt == true){
        ekf_initGPT(time, vn, ve, vd, lat, lon, alt, p, q, r, ax, ay, az, hx, hy, hz);
        _tprev = time;
    }
    else{
        if (!initialized_) {
            ekf_init(time, vn, ve, vd, lat, lon, alt, p, q, r, ax, ay, az, hx, hy, hz);
            // initialized flag
            initialized_ = true;
        } else {
            // get the change in time
            m_dt = ((double)(time - _tprev)) / 1e3;
            qDebug() << "Time: " << m_dt;
            // Update Gyro and Accelerometer biases
            updateBias(ax, ay, az, p, q, r);
            // Update INS values
            updateINS();
            // Attitude Update
            dq(0) = 1.0f;
            dq(1) = 0.5f*om_ib(0,0)*m_dt;
            dq(2) = 0.5f*om_ib(1,0)*m_dt;
            dq(3) = 0.5f*om_ib(2,0)*m_dt;
            quat = qmult(quat,dq);
            quat.normalize();
            // Avoid quaternion flips sign
            if (quat(0) < 0) {
                quat = -1.0f*quat;
            }
            // AHRS Transformations
            C_N2B = quat2dcm(quat);
            C_B2N = C_N2B.transpose();
            // obtain euler angles from quaternion

            // How to caulcate Euler Angles [Roll Φ(Phi) Gyro Z, Pitch θ(Theta) gyro Y, Yaw Ψ(Psi) Gyro X]
            std::tie(phi, theta, psi) = toEulerAngles(quat);

            // Velocity Update
            dx = C_B2N*f_b + grav;
            vn_ins += m_dt*dx(0,0);
            ve_ins += m_dt*dx(1,0);
            vd_ins += m_dt*dx(2,0);
            // Position Update
            dxd = llarate(V_ins,lla_ins);
            lat_ins += m_dt*dxd(0,0);
            lon_ins += m_dt*dxd(1,0);
            alt_ins += m_dt*dxd(2,0);
            // Jacobian update
            updateJacobianMatrix();
            // Update process noise and covariance time
            updateProcessNoiseCovarianceTime(m_dt);
            // Gps measurement update
            //if ((TOW - previousTOW) > 0) {
            if ((time - _tprev) > 0) {
                //previousTOW = TOW;
                lla_gps(0,0) = lat;
                lla_gps(1,0) = lon;
                lla_gps(2,0) = alt;
                V_gps(0,0) = vn;
                V_gps(1,0) = ve;
                V_gps(2,0) = vd;
                // Update INS values
                updateINS();
                // Create measurement Y
                updateCalculatedVsPredicted();
                // Kalman gain
                K = P*H.transpose()*(H*P*H.transpose() + R).inverse();
                // Covariance update
                P = (Eigen::Matrix<float,15,15>::Identity()-K*H)*P*(Eigen::Matrix<float,15,15>::Identity()-K*H).transpose() + K*R*K.transpose();
                // State update
                x = K*y;
                // Update the results
                update15statesAfterKF();
                _tprev = time;
            }
            // Get the new Specific forces and Rotation Rate
            updateBias(ax, ay, az, p, q, r);
        }
    }
}

void ekfNavINS::ekf_update(uint64_t time) {
    std::shared_lock lock(shMutex);
    ekf_update(time, /*0,*/ gpsVel.vN, gpsVel.vE, gpsVel.vD,
               gpsCoor.lat, gpsCoor.lon, gpsCoor.alt,
               imuDat.gyroX, imuDat.gyroY, imuDat.gyroZ,
               imuDat.accX, imuDat.accY, imuDat.accZ,
               imuDat.hX, imuDat.hY, imuDat.hZ);
}

void ekfNavINS::imuUpdateEKF(uint64_t time, imuData imu) {
    {
        std::unique_lock lock(shMutex);
        imuDat = imu;
    }
    ekf_update(time);
}

void ekfNavINS::gpsCoordinateUpdateEKF(gpsCoordinate coor) {
    std::unique_lock lock(shMutex);
    gpsCoor = coor;
}

void ekfNavINS::gpsVelocityUpdateEKF(gpsVelocity vel) {
    std::unique_lock lock(shMutex);
    gpsVel = vel;
}

void ekfNavINS::updateINS() {
    // Update lat, lng, alt, velocity INS values to matrix
    lla_ins(0,0) = lat_ins;
    lla_ins(1,0) = lon_ins;
    lla_ins(2,0) = alt_ins;
    V_ins(0,0) = vn_ins;
    V_ins(1,0) = ve_ins;
    V_ins(2,0) = vd_ins;
}
// az = up donwn, ax = Roll, ay = flatt up
std::tuple<float,float,float> ekfNavINS::getPitchRollYaw(float ax, float ay, float az, float hx, float hy, float hz, float gx, float gy, float gz)
{
    // initial attitude and heading
    //   qDebug() << "ax= " << ax << " ay= " << ay << " az= " << az << " hx= " << hx << " hy= " << hy << " hz= " << hz;
    //    theta = theta+gy*DEG_TO_RAD*m_dt;
    theta = asinf(ax/G); //+acosf(ay/G);
    //    qDebug() << theta;
    //    phi = phi+gx*DEG_TO_RAD*m_dt;
    phi = -asinf(ay/G*cosf(theta));
    // magnetic heading correction due to roll and pitch angle
    Bxc = hx*cosf(theta) + (hy*sinf(phi) + hz*cosf(phi))*sinf(theta);
    Byc = hy*cosf(phi) - hz*sinf(phi);
    // finding initial heading
    psi = -atan2f(Byc,Bxc);
    //    psi = psi + gz*DEG_TO_RAD*m_dt;
    return (std::make_tuple(theta,phi,psi));
}

void ekfNavINS::updateCalculatedVsPredicted() {
    // Position, converted to NED
    pos_ecef_ins = lla2ecef(lla_ins);
    pos_ecef_gps = lla2ecef(lla_gps);
    pos_ned_gps = ecef2ned(pos_ecef_gps - pos_ecef_ins, lla_ins);
    // Update the difference between calculated and predicted
    y(0,0) = (float)(pos_ned_gps(0,0));
    y(1,0) = (float)(pos_ned_gps(1,0));
    y(2,0) = (float)(pos_ned_gps(2,0));
    y(3,0) = (float)(V_gps(0,0) - V_ins(0,0));
    y(4,0) = (float)(V_gps(1,0) - V_ins(1,0));
    y(5,0) = (float)(V_gps(2,0) - V_ins(2,0));
}

void ekfNavINS::update15statesAfterKF() {
    estmimated_ins = llarate ((x.block(0,0,3,1)).cast<double>(), lat_ins, alt_ins);
    lat_ins += estmimated_ins(0,0);
    lon_ins += estmimated_ins(1,0);
    alt_ins += estmimated_ins(2,0);
    vn_ins = vn_ins + x(3,0);
    ve_ins = ve_ins + x(4,0);
    vd_ins = vd_ins + x(5,0);
    // Attitude correction
    dq(0,0) = 1.0f;
    dq(1,0) = x(6,0);
    dq(2,0) = x(7,0);
    dq(3,0) = x(8,0);
    quat = qmult(quat,dq);
    quat.normalize();
    // obtain euler angles from quaternion
    std::tie(phi, theta, psi) = toEulerAngles(quat);
    abx = abx + x(9,0);
    aby = aby + x(10,0);
    abz = abz + x(11,0);
    gbx = gbx + x(12,0);
    gby = gby + x(13,0);
    gbz = gbz + x(14,0);
}

void ekfNavINS::updateBias(float ax,float ay,float az,float p,float q, float r) {
    f_b(0,0) = ax - abx;
    f_b(1,0) = ay - aby;
    f_b(2,0) = az - abz;
    om_ib(0,0) = p - gbx;
    om_ib(1,0) = q - gby;
    om_ib(2,0) = r - gbz;
}

void ekfNavINS::updateProcessNoiseCovarianceTime(float _dt) {
    PHI = Eigen::Matrix<float,15,15>::Identity()+Fs*_dt;
    // Process Noise
    Gs.setZero();
    Gs.block(3,0,3,3) = -C_B2N;
    Gs.block(6,3,3,3) = -0.5f*Eigen::Matrix<float,3,3>::Identity();
    Gs.block(9,6,6,6) = Eigen::Matrix<float,6,6>::Identity();
    // Discrete Process Noise
    Q = PHI*_dt*Gs*Rw*Gs.transpose();
    Q = 0.5f*(Q+Q.transpose());
    // Covariance Time Update
    P = PHI*P*PHI.transpose()+Q;
    P = 0.5f*(P+P.transpose());
}

void ekfNavINS::updateJacobianMatrix() {
    // Jacobian
    Fs.setZero();
    // ... pos2gs
    Fs.block(0,3,3,3) = Eigen::Matrix<float,3,3>::Identity();
    // ... gs2pos
    Fs(5,2) = -2.0f*G/EARTH_RADIUS;
    // ... gs2att
    Fs.block(3,6,3,3) = -2.0f*C_B2N*sk(f_b);
    // ... gs2acc
    Fs.block(3,9,3,3) = -C_B2N;
    // ... att2att
    Fs.block(6,6,3,3) = -sk(om_ib);
    // ... att2gyr
    Fs.block(6,12,3,3) = -0.5f*Eigen::Matrix<float,3,3>::Identity();
    // ... Accel Markov Bias
    Fs.block(9,9,3,3) = -1.0f/TAU_A*Eigen::Matrix<float,3,3>::Identity();
    Fs.block(12,12,3,3) = -1.0f/TAU_G*Eigen::Matrix<float,3,3>::Identity();
}

// This function gives a skew symmetric matrix from a given vector w
Eigen::Matrix<float,3,3> ekfNavINS::sk(Eigen::Matrix<float,3,1> w) {
    Eigen::Matrix<float,3,3> C;
    C(0,0) = 0.0f;    C(0,1) = -w(2,0); C(0,2) = w(1,0);
    C(1,0) = w(2,0);  C(1,1) = 0.0f;    C(1,2) = -w(0,0);
    C(2,0) = -w(1,0); C(2,1) = w(0,0);  C(2,2) = 0.0f;
    return C;
}

constexpr std::pair<double, double> ekfNavINS::earthradius(double lat) {
    double denom = fabs(1.0 - (ECC2 * pow(sin(lat),2.0)));
    double Rew = EARTH_RADIUS / sqrt(denom);
    double Rns = EARTH_RADIUS * (1.0-ECC2) / (denom*sqrt(denom));
    return (std::make_pair(Rew, Rns));
}

// This function calculates the rate of change of latitude, longitude, and altitude.
Eigen::Matrix<double,3,1> ekfNavINS::llarate(Eigen::Matrix<double,3,1> V,Eigen::Matrix<double,3,1> lla) {
    double Rew, Rns, denom;
    Eigen::Matrix<double,3,1> lla_dot;
    std::tie(Rew, Rns) = earthradius(lla(0,0));
    lla_dot(0,0) = V(0,0)/(Rns + lla(2,0));
    lla_dot(1,0) = V(1,0)/((Rew + lla(2,0))*cos(lla(0,0)));
    lla_dot(2,0) = -V(2,0);
    return lla_dot;
}

// This function calculates the rate of change of latitude, longitude, and altitude.
Eigen::Matrix<double,3,1> ekfNavINS::llarate(Eigen::Matrix<double,3,1> V, double lat, double alt) {
    Eigen::Matrix<double,3,1> lla;
    lla(0,0) = lat;
    lla(1,0) = 0.0; // Not used
    lla(2,0) = alt;
    return llarate(V, lla);
}

// This function calculates the ECEF Coordinate given the Latitude, Longitude and Altitude.
Eigen::Matrix<double,3,1> ekfNavINS::lla2ecef(Eigen::Matrix<double,3,1> lla) {
    double Rew, denom;
    Eigen::Matrix<double,3,1> ecef;
    std::tie(Rew, std::ignore) = earthradius(lla(0,0));
    ecef(0,0) = (Rew + lla(2,0)) * cos(lla(0,0)) * cos(lla(1,0));
    ecef(1,0) = (Rew + lla(2,0)) * cos(lla(0,0)) * sin(lla(1,0));
    ecef(2,0) = (Rew * (1.0 - ECC2) + lla(2,0)) * sin(lla(0,0));
    return ecef;
}

// This function converts a vector in ecef to ned coordinate centered at pos_ref.
Eigen::Matrix<double,3,1> ekfNavINS::ecef2ned(Eigen::Matrix<double,3,1> ecef,Eigen::Matrix<double,3,1> pos_ref) {
    Eigen::Matrix<double,3,1> ned;
    ned(1,0)=-sin(pos_ref(1,0))*ecef(0,0) + cos(pos_ref(1,0))*ecef(1,0);
    ned(0,0)=-sin(pos_ref(0,0))*cos(pos_ref(1,0))*ecef(0,0)-sin(pos_ref(0,0))*sin(pos_ref(1,0))*ecef(1,0)+cos(pos_ref(0,0))*ecef(2,0);
    ned(2,0)=-cos(pos_ref(0,0))*cos(pos_ref(1,0))*ecef(0,0)-cos(pos_ref(0,0))*sin(pos_ref(1,0))*ecef(1,0)-sin(pos_ref(0,0))*ecef(2,0);
    return ned;
}

// quaternion to dcm
Eigen::Matrix<float,3,3> ekfNavINS::quat2dcm(Eigen::Matrix<float,4,1> q) {
    Eigen::Matrix<float,3,3> C_N2B;
    C_N2B(0,0) = 2.0f*powf(q(0,0),2.0f)-1.0f + 2.0f*powf(q(1,0),2.0f);
    C_N2B(1,1) = 2.0f*powf(q(0,0),2.0f)-1.0f + 2.0f*powf(q(2,0),2.0f);
    C_N2B(2,2) = 2.0f*powf(q(0,0),2.0f)-1.0f + 2.0f*powf(q(3,0),2.0f);

    C_N2B(0,1) = 2.0f*q(1,0)*q(2,0) + 2.0f*q(0,0)*q(3,0);
    C_N2B(0,2) = 2.0f*q(1,0)*q(3,0) - 2.0f*q(0,0)*q(2,0);

    C_N2B(1,0) = 2.0f*q(1,0)*q(2,0) - 2.0f*q(0,0)*q(3,0);
    C_N2B(1,2) = 2.0f*q(2,0)*q(3,0) + 2.0f*q(0,0)*q(1,0);

    C_N2B(2,0) = 2.0f*q(1,0)*q(3,0) + 2.0f*q(0,0)*q(2,0);
    C_N2B(2,1) = 2.0f*q(2,0)*q(3,0) - 2.0f*q(0,0)*q(1,0);
    return C_N2B;
}

// quaternion multiplication
Eigen::Matrix<float,4,1> ekfNavINS::qmult(Eigen::Matrix<float,4,1> p, Eigen::Matrix<float,4,1> q) {
    Eigen::Matrix<float,4,1> r;
    r(0,0) = p(0,0)*q(0,0) - (p(1,0)*q(1,0) + p(2,0)*q(2,0) + p(3,0)*q(3,0));
    r(1,0) = p(0,0)*q(1,0) + q(0,0)*p(1,0) + p(2,0)*q(3,0) - p(3,0)*q(2,0);
    r(2,0) = p(0,0)*q(2,0) + q(0,0)*p(2,0) + p(3,0)*q(1,0) - p(1,0)*q(3,0);
    r(3,0) = p(0,0)*q(3,0) + q(0,0)*p(3,0) + p(1,0)*q(2,0) - p(2,0)*q(1,0);
    return r;
}

// bound angle between -180 and 180
float ekfNavINS::constrainAngle180(float dta) {
    if(dta >  M_PI) dta -= (M_PI*2.0f);
    if(dta < -M_PI) dta += (M_PI*2.0f);
    return dta;
}

// bound angle between 0 and 360
float ekfNavINS::constrainAngle360(float dta){
    dta = fmod(dta,2.0f*M_PI);
    if (dta < 0)
        dta += 2.0f*M_PI;
    return dta;
}

Eigen::Matrix<float,4,1> ekfNavINS::toQuaternion(float yaw, float pitch, float roll) {
    float cy = cosf(yaw * 0.5f);
    float sy = sinf(yaw * 0.5f);
    float cp = cosf(pitch * 0.5f);
    float sp = sinf(pitch * 0.5f);
    float cr = cosf(roll * 0.5f);
    float sr = sinf(roll * 0.5f);
    Eigen::Matrix<float,4,1> q;
    q(0) = cr * cp * cy + sr * sp * sy; // w
    q(1) = cr * cp * sy - sr * sp * cy; // x
    q(2) = cr * sp * cy + sr * cp * sy; // y
    q(3) = sr * cp * cy - cr * sp * sy; // z
    return q;
}

std::tuple<float, float, float> ekfNavINS::toEulerAngles(Eigen::Matrix<float,4,1> quat) {
    float roll, pitch, yaw;
    // roll (x-axis rotation)
    float sinr_cosp = 2.0f * (quat(0,0)*quat(1,0)+quat(2,0)*quat(3,0));
    float cosr_cosp = 1.0f - 2.0f * (quat(1,0)*quat(1,0)+quat(2,0)*quat(2,0));
    roll = atan2f(sinr_cosp, cosr_cosp);
    // pitch (y-axis rotation)
    double sinp = 2.0f * (quat(0,0)*quat(2,0) - quat(1,0)*quat(3,0));
    //angles.pitch = asinf(-2.0f*(quat(1,0)*quat(3,0)-quat(0,0)*quat(2,0)));
    if (std::abs(sinp) >= 1)
        pitch = std::copysign(M_PI / 2.0f, sinp); // use 90 degrees if out of range
    else
        pitch = asinf(sinp);
    // yaw (z-axis rotation)
    float siny_cosp = 2.0f * (quat(1,0)*quat(2,0)+quat(0,0)*quat(3,0));
    float cosy_cosp = 1.0f - 2.0f * (quat(2,0)*quat(2,0)+quat(3,0)*quat(3,0));
    yaw = atan2f(siny_cosp, cosy_cosp);
    return std::make_tuple(roll, pitch, yaw);
}


//--------------------------------------

