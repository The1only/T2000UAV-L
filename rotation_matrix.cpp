#include "rotation_matrix.h"


Matrix3x3 createRotationMatrixZ(double theta) {
    Matrix3x3 rotationMatrix = {{
        {cos(theta), -sin(theta), 0.0f},
        {sin(theta), cos(theta), 0.0f},
        {0.0f, 0.0f, 1.0f}
    }};
    return rotationMatrix;
}

Matrix3x3 createRotationMatrix(Vector3 theta){
    double roll =theta[0];
    double pitch=theta[1];
    double yaw  =theta[2];

    Matrix3x3 rx = {{
        {1, 0, 0},
        {0, cos(pitch), -sin(pitch)},
        {0, sin(pitch), cos(pitch)}
    }};
    Matrix3x3 ry = {{
        {cos(yaw), 0, sin(yaw)},
        {0, 1, 0},
        {-sin(yaw), 0, cos(yaw)}
    }};
    Matrix3x3 rz = {{
        {cos(roll), -sin(roll), 0},
        {sin(roll), cos(roll), 0},
        {0, 0, 1}
    }};

    // Combine rotations: R = Rz * Ry * Rx
    return multiplyMatrix(multiplyMatrix(rz, ry), rx);
}

Matrix3x3 multiplyMatrix(const Matrix3x3& mat1, const Matrix3x3& mat2) {
    Matrix3x3 result = {0};  // Initialize result matrix with zeros
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result[i][j] = 0;
            for (int k = 0; k < 3; ++k) {
                result[i][j] += mat1[i][k] * mat2[k][j];
            }
        }
    }
    return result;
}

Vector3 rotateVector(const Matrix3x3& rotationMatrix, const Vector3& vector) {
    Vector3 result = {0.0f, 0.0f, 0.0f};
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            result[i] += rotationMatrix[i][j] * vector[j];
        }
    }
    return result;
}

void rotateSensors(Vector3& gyro, Vector3& accel, Vector3& mag, Vector3 theta) {
    Matrix3x3 rotationMatrix = createRotationMatrix(theta);

    gyro = rotateVector(rotationMatrix, gyro);
    accel = rotateVector(rotationMatrix, accel);
    mag = rotateVector(rotationMatrix, mag);
}

void rotateSensors(Vector3& gyro, Vector3& accel, Vector3& mag,  Vector3& attitude, Matrix3x3 rotationMatrix) {
    gyro = rotateVector(rotationMatrix, gyro);
    accel = rotateVector(rotationMatrix, accel);
    mag = rotateVector(rotationMatrix, mag);
    attitude = rotateVector(rotationMatrix, attitude);
}


void rotateSensors(Vector3& gyro, Vector3& accel, Vector3& mag, Matrix3x3 rotationMatrix) {
    gyro = rotateVector(rotationMatrix, gyro);
    accel = rotateVector(rotationMatrix, accel);
    mag = rotateVector(rotationMatrix, mag);
}

void printVector(const Vector3& vector) {
    std::cout << "[" << vector[0] << ", " << vector[1] << ", " << vector[2] << "]\n";
}

/*
int main() {
    double pitch =  90.0 * DEG_TO_RAD;  // 90-degree rotation in radians
    double roll  =  0.0 * DEG_TO_RAD;  // 90-degree rotation in radians
    double yaw   =  0.0 * DEG_TO_RAD;  // 90-degree rotation in radians

    // Define sensor vectors
    Vector3 rotate= { pitch, roll, yaw};
    Vector3 gyro  = { 1.0f, 2.0f, 3.0f };   // Gyroscope vector

    //                up    hor   right
    Vector3 accel = { 1.0f, 2.0f, 3.0f };   // Accelerometer vector
    Vector3 mag   = { 1.0f, 2.0f, 3.0f };   // Magnetometer vector

    std::cout << "Original Gyro Vector: ";
    printVector(gyro);

    std::cout << "Original Accel Vector: ";
    printVector(accel);

    std::cout << "Original Mag Vector: ";
    printVector(mag);

    // NED Rotate all sensor vectors
    rotateSensors(gyro, accel, mag, rotate);

    std::cout << "\nRotated Gyro Vector: ";
    printVector(gyro);

    std::cout << "Rotated Accel Vector: ";
    printVector(accel);

    std::cout << "Rotated Mag Vector: ";
    printVector(mag);

    return 0;
}
*/
