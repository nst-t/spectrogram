// Copyright (c) 2017, Michael Boyle
// See LICENSE file for details: <https://github.com/moble/quaternion/blob/master/LICENSE>

#ifndef __QUATERNION_H__
#define __QUATERNION_H__

#include "../math3d/math_3d.h"
#ifdef __cplusplus
extern "C" {
#endif

  #if defined(_MSC_VER)
    #include "math_msvc_compatibility.h"
  #else
    #include <math.h>
  #endif

  #define _QUATERNION_EPS 1e-14

  #if defined(_MSC_VER)
    #define NPY_INLINE __inline
  #elif defined(__GNUC__)
    #if defined(__STRICT_ANSI__)
      #define NPY_INLINE __inline__
    #else
      #define NPY_INLINE inline
    #endif
  #else
    #define NPY_INLINE
  #endif

  typedef struct {
    float w;
    float x;
    float y;
    float z;
  } quaternion_t;


  // Constructor-ish
  quaternion_t quaternion(float x, float y, float z, float w);
  quaternion_t quaternion_create_from_spherical_coords(float vartheta, float varphi);
  quaternion_t quaternion_create_from_euler_angles(float alpha, float beta, float gamma);

  // Unary bool returners
  static NPY_INLINE int quaternion_isnan(quaternion_t q) {
    return isnan(q.w) || isnan(q.x) || isnan(q.y) || isnan(q.z);
  }
  static NPY_INLINE int quaternion_nonzero(quaternion_t q) {
    if(quaternion_isnan(q)) { return 1; }
    return ! (q.w == 0 && q.x == 0 && q.y == 0 && q.z == 0);
  }
  static NPY_INLINE int quaternion_isinf(quaternion_t q) {
    return isinf(q.w) || isinf(q.x) || isinf(q.y) || isinf(q.z);
  }
  static NPY_INLINE int quaternion_isfinite(quaternion_t q) {
    return isfinite(q.w) && isfinite(q.x) && isfinite(q.y) && isfinite(q.z);
  }

  // Binary bool returners
  static NPY_INLINE int quaternion_equal(quaternion_t q1, quaternion_t q2) {
    return
      !quaternion_isnan(q1) &&
      !quaternion_isnan(q2) &&
      q1.w == q2.w &&
      q1.x == q2.x &&
      q1.y == q2.y &&
      q1.z == q2.z;
  }
  static NPY_INLINE int quaternion_not_equal(quaternion_t q1, quaternion_t q2) {
    return !quaternion_equal(q1, q2);
  }
  static NPY_INLINE int quaternion_less(quaternion_t q1, quaternion_t q2) {
    return
      (!quaternion_isnan(q1) && !quaternion_isnan(q2))
      &&
      (q1.w != q2.w ? q1.w < q2.w :
       q1.x != q2.x ? q1.x < q2.x :
       q1.y != q2.y ? q1.y < q2.y :
       q1.z != q2.z ? q1.z < q2.z : 0);
  }
  static NPY_INLINE int quaternion_greater(quaternion_t q1, quaternion_t q2) {
    return
      (!quaternion_isnan(q1) && !quaternion_isnan(q2))
      &&
      (q1.w != q2.w ? q1.w > q2.w :
       q1.x != q2.x ? q1.x > q2.x :
       q1.y != q2.y ? q1.y > q2.y :
       q1.z != q2.z ? q1.z > q2.z : 0);
  }
  static NPY_INLINE int quaternion_less_equal(quaternion_t q1, quaternion_t q2) {
    return
      (!quaternion_isnan(q1) && !quaternion_isnan(q2))
      &&
      (q1.w != q2.w ? q1.w < q2.w :
       q1.x != q2.x ? q1.x < q2.x :
       q1.y != q2.y ? q1.y < q2.y :
       q1.z != q2.z ? q1.z < q2.z : 1);
    // Note that the final possibility is 1, whereas in
    // `quaternion_less` it was 0.  This distinction correctly
    // accounts for equality.
  }
  static NPY_INLINE int quaternion_greater_equal(quaternion_t q1, quaternion_t q2) {
    return
      (!quaternion_isnan(q1) && !quaternion_isnan(q2))
      &&
      (q1.w != q2.w ? q1.w > q2.w :
       q1.x != q2.x ? q1.x > q2.x :
       q1.y != q2.y ? q1.y > q2.y :
       q1.z != q2.z ? q1.z > q2.z : 1);
    // Note that the final possibility is 1, whereas in
    // `quaternion_greater` it was 0.  This distinction correctly
    // accounts for equality.
  }

  // Unary float returners
  quaternion_t quaternion_log(quaternion_t q); // Pre-declare; declared again below, in its rightful place
  static NPY_INLINE float quaternion_norm(quaternion_t q) {
    return q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z;
  }
  static NPY_INLINE float quaternion_absolute(quaternion_t q) {
    return sqrt(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
  }
  static NPY_INLINE float quaternion_angle(quaternion_t q) {
    return 2 * quaternion_absolute( quaternion_log( q ) );
  }

  // Unary quaternion_t returners
  quaternion_t quaternion_sqrt(quaternion_t q);
  quaternion_t quaternion_log(quaternion_t q);
  quaternion_t quaternion_exp(quaternion_t q);
  static NPY_INLINE quaternion_t quaternion_normalized(quaternion_t q) {
    float q_abs = quaternion_absolute(q);
    quaternion_t r = {q.w/q_abs, q.x/q_abs, q.y/q_abs, q.z/q_abs};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_x_parity_conjugate(quaternion_t q) {
    quaternion_t r = {q.w, q.x, -q.y, -q.z};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_x_parity_symmetric_part(quaternion_t q) {
    quaternion_t r = {q.w, q.x, 0.0, 0.0};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_x_parity_antisymmetric_part(quaternion_t q) {
    quaternion_t r = {0.0, 0.0, q.y, q.z};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_y_parity_conjugate(quaternion_t q) {
    quaternion_t r = {q.w, -q.x, q.y, -q.z};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_y_parity_symmetric_part(quaternion_t q) {
    quaternion_t r = {q.w, 0.0, q.y, 0.0};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_y_parity_antisymmetric_part(quaternion_t q) {
    quaternion_t r = {0.0, q.x, 0.0, q.z};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_z_parity_conjugate(quaternion_t q) {
    quaternion_t r = {q.w, -q.x, -q.y, q.z};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_z_parity_symmetric_part(quaternion_t q) {
    quaternion_t r = {q.w, 0.0, 0.0, q.z};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_z_parity_antisymmetric_part(quaternion_t q) {
    quaternion_t r = {0.0, q.x, q.y, 0.0};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_parity_conjugate(quaternion_t q) {
    quaternion_t r = {q.w, q.x, q.y, q.z};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_parity_symmetric_part(quaternion_t q) {
    quaternion_t r = {q.w, q.x, q.y, q.z};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_parity_antisymmetric_part(quaternion_t q) {
    quaternion_t r = {0.0, 0.0, 0.0, 0.0};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_negative(quaternion_t q) {
    quaternion_t r = {-q.w, -q.x, -q.y, -q.z};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_conjugate(quaternion_t q) {
    quaternion_t r = {q.w, -q.x, -q.y, -q.z};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_inverse(quaternion_t q) {
    float norm = quaternion_norm(q);
    quaternion_t r = {q.w/norm, -q.x/norm, -q.y/norm, -q.z/norm};
    return r;
  }

  // Quaternion-quaternion_t binary quaternion_t returners
  static NPY_INLINE quaternion_t quaternion_copysign(quaternion_t q1, quaternion_t q2) {
    quaternion_t r = {
      (float)copysign(q1.w, q2.w),
      (float)copysign(q1.x, q2.x),
      (float)copysign(q1.y, q2.y),
      (float)copysign(q1.z, q2.z)
    };
    return r;
  }

  // Quaternion-vector binary void returner
//  static inline void _cross(float a[], float b[], float c[]) {
//    c[0] = a[1]*b[2] - a[2]*b[1];
//    c[1] = a[2]*b[0] - a[0]*b[2];
//    c[2] = a[0]*b[1] - a[1]*b[0];
//    return;
//  }
//  static inline void _cross_times_scalar(float s, float a[], float b[], float c[]) {
//    c[0] = s*(a[1]*b[2] - a[2]*b[1]);
//    c[1] = s*(a[2]*b[0] - a[0]*b[2]);
//    c[2] = s*(a[0]*b[1] - a[1]*b[0]);
//    return;
//  }
  static NPY_INLINE void _sv_plus_rxv(quaternion_t q, float v[], float w[]) {
    w[0] = q.w * v[0] + q.y*v[2] - q.z*v[1];
    w[1] = q.w * v[1] + q.z*v[0] - q.x*v[2];
    w[2] = q.w * v[2] + q.x*v[1] - q.y*v[0];
    return;
  }
  static NPY_INLINE void _v_plus_2rxvprime_over_m(quaternion_t q, float v[], float w[], float two_over_m, float vprime[]) {
    vprime[0] = v[0] + two_over_m * (q.y*w[2] - q.z*w[1]);
    vprime[1] = v[1] + two_over_m * (q.z*w[0] - q.x*w[2]);
    vprime[2] = v[2] + two_over_m * (q.x*w[1] - q.y*w[0]);
    return;
  }
  static NPY_INLINE void quaternion_rotate_vector(quaternion_t q, float v[], float vprime[]) {
    // The most efficient formula I know of for rotating a vector by a quaternion_t is
    //     v' = v + 2 * r x (s * v + r x v) / m
    // where x represents the cross product, s and r are the scalar and vector parts of the quaternion, respectively,
    // and m is the sum of the squares of the components of the quaternion.  This requires 22 multiplications and
    // 14 additions, as opposed to 32 and 24 for naive application of `q*v*q.conj()`.  In this function, I will further
    // reduce the operation count to 18 and 12 by skipping the normalization by `m`.  The full version will be
    // implemented in another function.
    float w[3];
    _sv_plus_rxv(q, v, w);
    _v_plus_2rxvprime_over_m(q, v, w, 2, vprime);
    return;
  }
  static NPY_INLINE void quaternion_rotate_vec3(quaternion_t q, const vec3_t v_in, vec3_t *v_out) {
    // The most efficient formula I know of for rotating a vector by a quaternion_t is
    //     v' = v + 2 * r x (s * v + r x v) / m
    // where x represents the cross product, s and r are the scalar and vector parts of the quaternion, respectively,
    // and m is the sum of the squares of the components of the quaternion.  This requires 22 multiplications and
    // 14 additions, as opposed to 32 and 24 for naive application of `q*v*q.conj()`.  In this function, I will further
    // reduce the operation count to 18 and 12 by skipping the normalization by `m`.  The full version will be
    // implemented in another function.
    float w[3],v[3],vprime[3];
    v[0] = v_in.x;
    v[1] = v_in.y;
    v[2] = v_in.z;
    _sv_plus_rxv(q, v, w);
    _v_plus_2rxvprime_over_m(q, v, w, 2, vprime);
    v_out->x = vprime[0];
    v_out->y = vprime[1];
    v_out->z = vprime[2];
    return;
  }
  static NPY_INLINE void quaternion_rotate_vector_and_normalize(quaternion_t q, float v[], float vprime[]) {
    // This applies the algorithm described above, but also includes normalization of the quaternion.
    float w[3];
    float m = q.x*q.x+q.y*q.y+q.z*q.z;
    _sv_plus_rxv(q, v, w);
    _v_plus_2rxvprime_over_m(q, v, w, 2/m, vprime);
    return;
  }

  // Quaternion-quaternion/quaternion-scalar binary quaternion_t returners
  static NPY_INLINE quaternion_t quaternion_add(quaternion_t q1, quaternion_t q2) {
    quaternion_t r = {
      q1.w+q2.w,
      q1.x+q2.x,
      q1.y+q2.y,
      q1.z+q2.z,
    };
    return r;
  }
  static NPY_INLINE void quaternion_inplace_add(quaternion_t* q1, quaternion_t q2) {
    q1->w += q2.w;
    q1->x += q2.x;
    q1->y += q2.y;
    q1->z += q2.z;
    return;
  }
  static NPY_INLINE quaternion_t quaternion_scalar_add(float s, quaternion_t q) {
    quaternion_t r = {s+q.w, q.x, q.y, q.z};
    return r;
  }
  static NPY_INLINE void quaternion_inplace_scalar_add(float s, quaternion_t* q) {
    q->w += s;
    return;
  }
  static NPY_INLINE quaternion_t quaternion_add_scalar(quaternion_t q, float s) {
    quaternion_t r = {s+q.w, q.x, q.y, q.z};
    return r;
  }
  static NPY_INLINE void quaternion_inplace_add_scalar(quaternion_t* q, float s) {
    q->w += s;
    return;
  }
  static NPY_INLINE quaternion_t quaternion_subtract(quaternion_t q1, quaternion_t q2) {
    quaternion_t r = {
      q1.w-q2.w,
      q1.x-q2.x,
      q1.y-q2.y,
      q1.z-q2.z,
    };
    return r;
  }
  static NPY_INLINE void quaternion_inplace_subtract(quaternion_t* q1, quaternion_t q2) {
    q1->w -= q2.w;
    q1->x -= q2.x;
    q1->y -= q2.y;
    q1->z -= q2.z;
    return;
  }
  static NPY_INLINE quaternion_t quaternion_scalar_subtract(float s, quaternion_t q) {
    quaternion_t r = {s-q.w, -q.x, -q.y, -q.z};
    return r;
  }
  static NPY_INLINE quaternion_t quaternion_subtract_scalar(quaternion_t q, float s) {
    quaternion_t r = {q.w-s, q.x, q.y, q.z};
    return r;
  }
  static NPY_INLINE void quaternion_inplace_subtract_scalar(quaternion_t* q, float s) {
    q->w -= s;
    return;
  }
  static NPY_INLINE quaternion_t quaternion_multiply(quaternion_t q1, quaternion_t q2) {
    quaternion_t r = {
      q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z,
      q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y,
      q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x,
      q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w,
    };
    return r;
  }
  static NPY_INLINE void quaternion_inplace_multiply(quaternion_t* q1a, quaternion_t q2) {
    quaternion_t q1 = {q1a->w, q1a->x, q1a->y, q1a->z};
    q1a->w = q1.w*q2.w - q1.x*q2.x - q1.y*q2.y - q1.z*q2.z;
    q1a->x = q1.w*q2.x + q1.x*q2.w + q1.y*q2.z - q1.z*q2.y;
    q1a->y = q1.w*q2.y - q1.x*q2.z + q1.y*q2.w + q1.z*q2.x;
    q1a->z = q1.w*q2.z + q1.x*q2.y - q1.y*q2.x + q1.z*q2.w;
    return;
  }
  static NPY_INLINE quaternion_t quaternion_scalar_multiply(float s, quaternion_t q) {
    quaternion_t r = {s*q.w, s*q.x, s*q.y, s*q.z};
    return r;
  }
  static NPY_INLINE void quaternion_inplace_scalar_multiply(float s, quaternion_t* q) {
    q->w *= s;
    q->x *= s;
    q->y *= s;
    q->z *= s;
    return;
  }
  static NPY_INLINE quaternion_t quaternion_multiply_scalar(quaternion_t q, float s) {
    quaternion_t r = {s*q.w, s*q.x, s*q.y, s*q.z};
    return r;
  }
  static NPY_INLINE void quaternion_inplace_multiply_scalar(quaternion_t* q, float s) {
    q->w *= s;
    q->x *= s;
    q->y *= s;
    q->z *= s;
    return;
  }
  static NPY_INLINE quaternion_t quaternion_divide(quaternion_t q1, quaternion_t q2) {
    float q2norm = q2.w*q2.w + q2.x*q2.x + q2.y*q2.y + q2.z*q2.z;
    quaternion_t r = {
      (  q1.w*q2.w + q1.x*q2.x + q1.y*q2.y + q1.z*q2.z) / q2norm,
      (- q1.w*q2.x + q1.x*q2.w - q1.y*q2.z + q1.z*q2.y) / q2norm,
      (- q1.w*q2.y + q1.x*q2.z + q1.y*q2.w - q1.z*q2.x) / q2norm,
      (- q1.w*q2.z - q1.x*q2.y + q1.y*q2.x + q1.z*q2.w) / q2norm
    };
    return r;
  }
  static NPY_INLINE void quaternion_inplace_divide(quaternion_t* q1a, quaternion_t q2) {
    float q2norm;
    quaternion_t q1 = *q1a;
    q2norm = q2.w*q2.w + q2.x*q2.x + q2.y*q2.y + q2.z*q2.z;
    q1a->w = ( q1.w*q2.w + q1.x*q2.x + q1.y*q2.y + q1.z*q2.z)/q2norm;
    q1a->x = (-q1.w*q2.x + q1.x*q2.w - q1.y*q2.z + q1.z*q2.y)/q2norm;
    q1a->y = (-q1.w*q2.y + q1.x*q2.z + q1.y*q2.w - q1.z*q2.x)/q2norm;
    q1a->z = (-q1.w*q2.z - q1.x*q2.y + q1.y*q2.x + q1.z*q2.w)/q2norm;
    return;
  }
  static NPY_INLINE quaternion_t quaternion_scalar_divide(float s, quaternion_t q) {
    float qnorm = q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z;
    quaternion_t r = {
      ( s*q.w) / qnorm,
      (-s*q.x) / qnorm,
      (-s*q.y) / qnorm,
      (-s*q.z) / qnorm
    };
    return r;
  }
  /* The following function is impossible, but listed for completeness: */
  /* static NPY_INLINE void quaternion_inplace_scalar_divide(float* sa, quaternion_t q2) { } */
  static NPY_INLINE quaternion_t quaternion_divide_scalar(quaternion_t q, float s) {
    quaternion_t r = {q.w/s, q.x/s, q.y/s, q.z/s};
    return r;
  }
  static NPY_INLINE void quaternion_inplace_divide_scalar(quaternion_t* q, float s) {
    q->w /= s;
    q->x /= s;
    q->y /= s;
    q->z /= s;
    return;
  }
  static NPY_INLINE quaternion_t quaternion_power(quaternion_t q, quaternion_t p) {
    /* Note that the following is just my chosen definition of the power. */
    /* Other definitions may disagree due to non-commutativity. */
    if(! quaternion_nonzero(q)) { /* log(q)=-inf */
      if(! quaternion_nonzero(p)) {
        quaternion_t r = {1.0, 0.0, 0.0, 0.0}; /* consistent with python */
        return r;
      } else {
        quaternion_t r = {0.0, 0.0, 0.0, 0.0}; /* consistent with python */
        return r;
      }
    }
    return quaternion_exp(quaternion_multiply(quaternion_log(q), p));
  }
  static NPY_INLINE void quaternion_inplace_power(quaternion_t* q1, quaternion_t q2) {
    /* Not overly useful as an in-place operator, but here for completeness. */
    quaternion_t q3 = quaternion_power(*q1,q2);
    *q1 = q3;
    return;
  }
  quaternion_t quaternion_scalar_power(float s, quaternion_t q);
  static NPY_INLINE void quaternion_inplace_scalar_power(float s, quaternion_t* q) {
    /* Not overly useful as an in-place operator, but here for completeness. */
    quaternion_t q2 = quaternion_scalar_power(s, *q);
    *q = q2;
    return;
  }
  static NPY_INLINE quaternion_t quaternion_power_scalar(quaternion_t q, float s) {
    /* Unlike the quaternion^quaternion_t power, this is unambiguous. */
    if(! quaternion_nonzero(q)) { /* log(q)=-inf */
      if(s==0) {
        quaternion_t r = {1.0, 0.0, 0.0, 0.0}; /* consistent with python */
        return r;
      } else {
        quaternion_t r = {0.0, 0.0, 0.0, 0.0}; /* consistent with python */
        return r;
      }
    }
    return quaternion_exp(quaternion_multiply_scalar(quaternion_log(q), s));
  }
  static NPY_INLINE void quaternion_inplace_power_scalar(quaternion_t* q, float s) {
    /* Not overly useful as an in-place operator, but here for completeness. */
    quaternion_t q2 = quaternion_power_scalar(*q, s);
    *q = q2;
    return;
  }

  // Associated functions
  static NPY_INLINE float rotor_intrinsic_distance(quaternion_t q1, quaternion_t q2) {
    return 2*quaternion_absolute(quaternion_log(quaternion_divide(q1,q2)));
  }
  static NPY_INLINE float rotor_chordal_distance(quaternion_t q1, quaternion_t q2) {
    return quaternion_absolute(quaternion_subtract(q1,q2));
  }
  static NPY_INLINE float rotation_intrinsic_distance(quaternion_t q1, quaternion_t q2) {
    if(rotor_chordal_distance(q1,q2)<=1.414213562373096) {
      return 2*quaternion_absolute(quaternion_log(quaternion_divide(q1,q2)));
    } else {
      return 2*quaternion_absolute(quaternion_log(quaternion_divide(q1,quaternion_negative(q2))));
    }
  }
  static NPY_INLINE float rotation_chordal_distance(quaternion_t q1, quaternion_t q2) {
    if(rotor_chordal_distance(q1,q2)<=1.414213562373096) {
      return quaternion_absolute(quaternion_subtract(q1,q2));
    } else {
      return quaternion_absolute(quaternion_add(q1,q2));
    }
  }
  static NPY_INLINE quaternion_t slerp(quaternion_t q1, quaternion_t q2, float tau) {
    if(rotor_chordal_distance(q1,q2)<=1.414213562373096) {
      return quaternion_multiply( quaternion_power_scalar(quaternion_divide(q2,q1), tau), q1);
    } else {
      return quaternion_multiply( quaternion_power_scalar(quaternion_divide(quaternion_negative(q2),q1), tau), q1);
    }
  }
  static NPY_INLINE quaternion_t squad_evaluate(float tau_i, quaternion_t q_i, quaternion_t a_i, quaternion_t b_ip1, quaternion_t q_ip1) {
    return slerp(slerp(q_i, q_ip1, tau_i),
                 slerp(a_i, b_ip1, tau_i),
                 2*tau_i*(1-tau_i));
  }


#ifdef __cplusplus
}
#endif

#endif // __QUATERNION_H__
