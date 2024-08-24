#include "simple_fusion.h"
#include <math.h>

void normalizeQuaternion(Quaternion *q)
{
    float norm = sqrtf(q->w * q->w + q->x * q->x + q->y * q->y + q->z * q->z);
    q->w /= norm;
    q->x /= norm;
    q->y /= norm;
    q->z /= norm;
}

Quaternion multiplyQuaternions(Quaternion q1, Quaternion q2)
{
    Quaternion result;
    result.w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    result.x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    result.y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    result.z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
    return result;
}

Quaternion integrateGyroscope(Quaternion q, float gx, float gy, float gz, float dt)
{
    Quaternion q_omega;
    q_omega.w = 0;
    q_omega.x = gx * dt / 2.0f;
    q_omega.y = gy * dt / 2.0f;
    q_omega.z = gz * dt / 2.0f;

    Quaternion delta_q = multiplyQuaternions(q, q_omega);
    q.w += delta_q.w;
    q.x += delta_q.x;
    q.y += delta_q.y;
    q.z += delta_q.z;

    normalizeQuaternion(&q);
    return q;
}

Vector3 crossProduct(Vector3 v1, Vector3 v2)
{
    Vector3 result;
    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.x;
    return result;
}

float deg2rad(float deg)
{
    return deg * M_PI / 180.0f;
}

Vector3 rotateVector(Vector3 v, Quaternion q)
{
    Quaternion v_q = {0, v.x, v.y, v.z};
    Quaternion q_conj = {q.w, -q.x, -q.y, -q.z};
    Quaternion result = multiplyQuaternions(multiplyQuaternions(q, v_q), q_conj);
    return (Vector3){result.x, result.y, result.z};
}

EulerAngles quaternionToEulerAngles(Quaternion q)
{
    EulerAngles angles;

    // Roll (x-axis rotation)
    angles.roll = atan2f(2.0f * (q.w * q.x + q.y * q.z), 1.0f - 2.0f * (q.x * q.x + q.y * q.y));

    // Pitch (y-axis rotation)
    float sinp = 2.0f * (q.w * q.y - q.z * q.x);
    if (fabsf(sinp) >= 1)
        angles.pitch = copysignf(M_PI / 2.0f, sinp); // Use 90 degrees if out of range
    else
        angles.pitch = asinf(sinp);

    // Yaw (z-axis rotation)
    angles.yaw = atan2f(2.0f * (q.w * q.z + q.x * q.y), 1.0f - 2.0f * (q.y * q.y + q.z * q.z));

    return angles;
}

Quaternion eulerAnglesToQuaternion(EulerAngles angles)
{
    Quaternion q;

    // Calculate the half angles
    float cy = cosf(angles.yaw * 0.5f);
    float sy = sinf(angles.yaw * 0.5f);
    float cp = cosf(angles.pitch * 0.5f);
    float sp = sinf(angles.pitch * 0.5f);
    float cr = cosf(angles.roll * 0.5f);
    float sr = sinf(angles.roll * 0.5f);

    // Compute the quaternion components
    q.w = cr * cp * cy + sr * sp * sy;
    q.x = sr * cp * cy - cr * sp * sy;
    q.y = cr * sp * cy + sr * cp * sy;
    q.z = cr * cp * sy - sr * sp * cy;

    return q;
}

Quaternion errorCorrection(Quaternion q, Vector3 accel, Vector3 mag, Vector3 accel_ref, Vector3 mag_ref, float k_a, float k_m)
{
    // Normalize inputs
    float accel_norm = sqrtf(accel.x * accel.x + accel.y * accel.y + accel.z * accel.z);
    accel.x /= accel_norm;
    accel.y /= accel_norm;
    accel.z /= accel_norm;

    float mag_norm = sqrtf(mag.x * mag.x + mag.y * mag.y + mag.z * mag.z);
    mag.x /= mag_norm;
    mag.y /= mag_norm;
    mag.z /= mag_norm;

    // Calculate estimated vectors
    Quaternion q_conj = {q.w, -q.x, -q.y, -q.z};
    Quaternion accel_q = multiplyQuaternions(multiplyQuaternions(q, (Quaternion){0, accel_ref.x, accel_ref.y, accel_ref.z}), q_conj);
    Quaternion mag_q = multiplyQuaternions(multiplyQuaternions(q, (Quaternion){0, mag_ref.x, mag_ref.y, mag_ref.z}), q_conj);

    Vector3 accel_est = {accel_q.x, accel_q.y, accel_q.z};
    Vector3 mag_est = {mag_q.x, mag_q.y, mag_q.z};

    // Calculate error vectors
    Vector3 accel_error = crossProduct(accel, accel_est);
    Vector3 mag_error = crossProduct(mag, mag_est);

    // Total error
    Vector3 total_error = {
        k_a * accel_error.x + k_m * mag_error.x,
        k_a * accel_error.y + k_m * mag_error.y,
        k_a * accel_error.z + k_m * mag_error.z};

    Quaternion error_q = {0, total_error.x, total_error.y, total_error.z};

    // Apply correction
    Quaternion corrected_q = multiplyQuaternions(q, error_q);
    normalizeQuaternion(&corrected_q);
    return corrected_q;
}

// accel_ref and mag_ref are reference vectors (assuming they are [0, 0, 1] and [1, 0, 0] in world frame)
Quaternion sensorFusion(Quaternion q, float gx, float gy, float gz, Vector3 accel, Vector3 mag, Vector3 accel_ref, Vector3 mag_ref, float dt, float k_a, float k_m)
{
    // Integrate gyroscope data
    q = integrateGyroscope(q, gx, gy, gz, dt);

    // Correct orientation based on accelerometer and magnetometer
    q = errorCorrection(q, accel, mag, accel_ref, mag_ref, k_a, k_m);

    return q;
}