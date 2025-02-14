#ifndef ROTATION_MATRIX_H
#define ROTATION_MATRIX_H

#include <QCoreApplication>
#include <array>     // For std::array
#include <cmath>     // For sin, cos, M_PI
#include <iostream>  // For std::cout
#include <array>

// Define Vector3 as a 3-element array to represent 3D vectors (x, y, z)
using Vector3 = std::array<double, 3>;
using Vector9 = std::array<double, 9>;

#define DEG_TO_RAD (3.141592/180)
#define RAD_TO_DEG (180.0/3.141592765)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using Matrix3x3 = std::array<std::array<double, 3>, 3>;
using Matrix9x9 = std::array<std::array<double, 9>, 9>;

void printVector(const Vector3& vector);
void rotateSensors(Vector3& gyro, Vector3& accel, Vector3& mag, double theta);
void rotateSensors(Vector3& gyro, Vector3& accel, Vector3& mag, Matrix3x3 rotationMatrix);

Vector3 rotateVector(const Matrix3x3& rotationMatrix, const Vector3& vector);
Matrix3x3 multiplyMatrix(const Matrix3x3& mat1, const Matrix3x3& mat2);
Matrix3x3 createRotationMatrix(Vector3 theta);
Matrix3x3 createRotationMatrixZ(double theta);
void rotateSensors(Vector3& gyro, Vector3& accel, Vector3& mag, Vector3& attitude, Matrix3x3 rotationMatrix);

#endif // ROTATION_MATRIX_H
