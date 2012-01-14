#ifndef __VEC_H__
#define __VEC_H__

#define PLANE_DIST_EPSILON 0.01
#define PLANE_NORMAL_EPSILON 0.00001

extern void
Vec_Clear (float v[3]);

extern void
Vec_Copy (const float src[3], float out[3]);

extern void
Vec_Add (const float a[3], const float b[3], float out[3]);

extern void
Vec_Subtract (const float a[3], const float b[3], float out[3]);

extern float
Vec_Dot (const float a[3], const float b[3]);

extern void
Vec_Cross (const float a[3], const float b[3], float out[3]);

extern void
Vec_Normalize (float v[3]);

extern void
Vec_SnapPlane (float normal[3], float *dist);

#endif /* __VEC_H__ */
