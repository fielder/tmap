#ifndef __VEC_H__
#define __VEC_H__

#define PITCH	0
#define YAW	1
#define ROLL	2

struct plane_s
{
	float normal[3];
	float dist;
	unsigned char type;
	unsigned char signbits;
	unsigned char pad[2];
};

enum
{
	PLANE_X,	/* normal lies on x axis */
	PLANE_Y,	/* normal lies on y axis */
	PLANE_Z,	/* normal lies on z axis */
	PLANE_NEAR_X,	/* normal is closest to PLANE_X */
	PLANE_NEAR_Y,	/* normal is closest to PLANE_Y */
	PLANE_NEAR_Z	/* normal is closest to PLANE_Z */
};

enum
{
	SIDE_FRONT,
	SIDE_BACK,
	SIDE_CROSS
};

#define PLANE_DIST_EPSILON 0.01
#define PLANE_NORMAL_EPSILON 0.00001

extern void
Vec_Clear (float v[3]);

extern void
Vec_Copy (const float src[3], float out[3]);

extern void
Vec_Scale (float v[3], float s);

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

extern void
Vec_MakeNormal (const float v1[3],
		const float v2[3],
		const float v3[3],
		float normal[3],
		float *dist);

extern int
Vec_BoxPlaneSide (const struct plane_s *plane, float mins[3], float maxs[3]);

#endif /* __VEC_H__ */
