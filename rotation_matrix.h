#ifndef ROTATION_MATRIX_H
#define ROTATION_MATRIX_H

#include <QCoreApplication>
#include <array>     // For std::array
#include <cmath>     // For sin, cos, M_PI
#include <iostream>  // For std::cout
#include <array>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Geometry> // for AngleAxis
#include <vector>
#include <cmath>

using namespace Eigen;

// Define Vector3x as a 3-element array to represent 3D vectors (x, y, z)
using Vector3x = std::array<double, 3>;
using Vector9 = std::array<double, 9>;

#define DEG_TO_RAD (3.141592/180)
#define RAD_TO_DEG (180.0/3.141592765)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

using Matrix3x3 = std::array<std::array<double, 3>, 3>;
using Matrix3x6 = std::array<std::array<double, 6>, 3>;
using Matrix9x9 = std::array<std::array<double, 9>, 9>;

void printVector(const Vector3x& vector);
void rotateSensors(Vector3x& gyro, Vector3x& accel, Vector3x& mag, double theta);
void rotateSensors(Vector3x& gyro, Vector3x& accel, Vector3x& mag, Matrix3x3 rotationMatrix);

Vector3x rotateVector(const Matrix3x3& rotationMatrix, const Vector3x& vector);
Matrix3x3 multiplyMatrix(const Matrix3x3& mat1, const Matrix3x3& mat2);
Matrix3x3 createRotationMatrix(Vector3x theta);
Matrix3x3 createRotationMatrixZ(double theta);
void rotateSensors(Vector3x& gyro, Vector3x& accel, Vector3x& mag, Vector3x& attitude, Matrix3x3 rotationMatrix);

#endif // ROTATION_MATRIX_H
