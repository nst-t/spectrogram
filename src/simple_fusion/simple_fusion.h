#ifndef SIMPLE_FUSION_H
#define SIMPLE_FUSION_H

typedef struct
{
    float w, x, y, z;
} Quaternion;

typedef struct
{
    float x, y, z;
} Vector3;

typedef struct
{
    float roll, pitch, yaw;
} EulerAngles;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

float deg2rad(float deg);

EulerAngles quaternionToEulerAngles(Quaternion q);

Vector3 rotateVector(Vector3 v, Quaternion q);

Quaternion eulerAnglesToQuaternion(EulerAngles angles);

void normalizeQuaternion(Quaternion *q);

Quaternion multiplyQuaternions(Quaternion q1, Quaternion q2);

Quaternion integrateGyroscope(Quaternion q, float gx, float gy, float gz, float dt);

Vector3 crossProduct(Vector3 v1, Vector3 v2);

Quaternion errorCorrection(Quaternion q, Vector3 accel, Vector3 mag, Vector3 accel_ref, Vector3 mag_ref, float k_a, float k_m);

Quaternion sensorFusion(Quaternion q, float gx, float gy, float gz, Vector3 accel, Vector3 mag, Vector3 accel_ref, Vector3 mag_ref, float dt, float k_a, float k_m);

#endif // SIMPLE_FUSION_H