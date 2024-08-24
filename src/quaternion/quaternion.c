// Copyright (c) 2017, Michael Boyle
// See LICENSE file for details: <https://github.com/moble/quaternion/blob/master/LICENSE>

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_MSC_VER)
#include "math_msvc_compatibility.h"
#else
#include <math.h>
#endif

#include <stdio.h>
#include <float.h>

#include "quaternion.h"

  quaternion_t quaternion(float x, float y, float z, float w)
  {
    quaternion_t r = {w, x, y, z};
    return r;
  }

  quaternion_t quaternion_create_from_spherical_coords(float vartheta, float varphi)
  {
    float ct = cos(vartheta / 2.);
    float cp = cos(varphi / 2.);
    float st = sin(vartheta / 2.);
    float sp = sin(varphi / 2.);
    quaternion_t r = {cp * ct, -sp * st, st * cp, sp * ct};
    return r;
  }

  quaternion_t quaternion_create_from_euler_angles(float alpha, float beta, float gamma)
  {
    float ca = cos(alpha / 2.);
    float cb = cos(beta / 2.);
    float cc = cos(gamma / 2.);
    float sa = sin(alpha / 2.);
    float sb = sin(beta / 2.);
    float sc = sin(gamma / 2.);
    quaternion_t r = {ca * cb * cc - sa * cb * sc, ca * sb * sc - sa * sb * cc, ca * sb * cc + sa * sb * sc, sa * cb * cc + ca * cb * sc};
    return r;
  }

  quaternion_t quaternion_sqrt(quaternion_t q)
  {
    float absolute = quaternion_norm(q); // pre-square-root
    if (absolute <= DBL_MIN)
    {
      quaternion_t r = {0.0, 0.0, 0.0, 0.0};
      return r;
    }
    absolute = sqrt(absolute);
    if (fabs(absolute + q.w) < _QUATERNION_EPS * absolute)
    {
      quaternion_t r = {0.0, sqrt(absolute), 0.0, 0.0};
      return r;
    }
    else
    {
      float c = sqrt(0.5 / (absolute + q.w));
      quaternion_t r = {(absolute + q.w) * c, q.x * c, q.y * c, q.z * c};
      return r;
    }
  }

  quaternion_t quaternion_log(quaternion_t q)
  {
    float b = sqrt(q.x * q.x + q.y * q.y + q.z * q.z);
    if (fabs(b) <= _QUATERNION_EPS * fabs(q.w))
    {
      if (q.w < 0.0)
      {
        // fprintf(stderr, "Input quaternion(%.15g, %.15g, %.15g, %.15g) has no unique logarithm; returning one arbitrarily.", q.w, q.x, q.y, q.z);
        if (fabs(q.w + 1) > _QUATERNION_EPS)
        {
          quaternion_t r = {log(-q.w), M_PI, 0., 0.};
          return r;
        }
        else
        {
          quaternion_t r = {0., M_PI, 0., 0.};
          return r;
        }
      }
      else
      {
        quaternion_t r = {log(q.w), 0., 0., 0.};
        return r;
      }
    }
    else
    {
      float v = atan2(b, q.w);
      float f = v / b;
      quaternion_t r = {log(q.w * q.w + b * b) / 2.0, f * q.x, f * q.y, f * q.z};
      return r;
    }
  }

  float
  _quaternion_scalar_log(float s) { return log(s); }

  quaternion_t quaternion_scalar_power(float s, quaternion_t q)
  {
    /* Unlike the quaternion^quaternion_t power, this is unambiguous. */
    if (s == 0.0)
    { /* log(s)=-inf */
      if (!quaternion_nonzero(q))
      {
        quaternion_t r = {1.0, 0.0, 0.0, 0.0}; /* consistent with python */
        return r;
      }
      else
      {
        quaternion_t r = {0.0, 0.0, 0.0, 0.0}; /* consistent with python */
        return r;
      }
    }
    else if (s < 0.0)
    { /* log(s)=nan */
      // fprintf(stderr, "Input scalar (%.15g) has no unique logarithm; returning one arbitrarily.", s);
      quaternion_t t = {log(-s), M_PI, 0, 0};
      return quaternion_exp(quaternion_multiply(q, t));
    }
    return quaternion_exp(quaternion_multiply_scalar(q, log(s)));
  }

  quaternion_t quaternion_exp(quaternion_t q)
  {
    float vnorm = sqrt(q.x * q.x + q.y * q.y + q.z * q.z);
    if (vnorm > _QUATERNION_EPS)
    {
      float s = sin(vnorm) / vnorm;
      float e = exp(q.w);
      quaternion_t r = {e * cos(vnorm), e * s * q.x, e * s * q.y, e * s * q.z};
      return r;
    }
    else
    {
      quaternion_t r = {exp(q.w), 0, 0, 0};
      return r;
    }
  }

#ifdef __cplusplus
}
#endif
