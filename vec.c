#include <math.h>

#include "vec.h"


void
Vec_Clear (float v[3])
{
	v[0] = 0.0;
	v[1] = 0.0;
	v[2] = 0.0;
}


void
Vec_Copy (const float src[3], float out[3])
{
	out[0] = src[0];
	out[1] = src[1];
	out[2] = src[2];
}


void
Vec_Scale (float v[3], float s)
{
	v[0] *= s;
	v[1] *= s;
	v[2] *= s;
}


void
Vec_Add (const float a[3], const float b[3], float out[3])
{
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
	out[2] = a[2] + b[2];
}


void
Vec_Subtract (const float a[3], const float b[3], float out[3])
{
	out[0] = a[0] - b[0];
	out[1] = a[1] - b[1];
	out[2] = a[2] - b[2];
}


float
Vec_Dot (const float a[3], const float b[3])
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}


void
Vec_Cross (const float a[3], const float b[3], float out[3])
{
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}


void
Vec_Normalize (float v[3])
{
	float len;

	len = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	if (len == 0.0)
	{
		v[0] = 0.0;
		v[1] = 0.0;
		v[2] = 0.0;
	}
	else
	{
		len = 1.0 / len;
		v[0] *= len;
		v[1] *= len;
		v[2] *= len;
	}
}


void
Vec_SnapPlane (float normal[3], float *dist)
{
	int i;
	float idist;

	for (i = 0; i < 3; i++)
	{
		if (fabs(normal[i] - 1.0) < PLANE_NORMAL_EPSILON)
		{
			Vec_Clear (normal);
			normal[i] = 1.0;
			break;
		}
		else if (fabs(normal[i] - -1.0) < PLANE_NORMAL_EPSILON)
		{
			Vec_Clear (normal);
			normal[i] = -1.0;
			break;
		}
	}

	idist = floor (*dist + 0.5);
	if (fabs(*dist - idist) < PLANE_DIST_EPSILON)
		*dist = idist;
}


void
Vec_MakeNormal (const float v1[3],
		const float v2[3],
		const float v3[3],
		float normal[3],
		float *dist)
{
	float a[3], b[3];

	Vec_Subtract (v2, v1, a);
	Vec_Subtract (v3, v1, b);

	Vec_Cross (a, b, normal);

	Vec_Normalize (normal);

	*dist = Vec_Dot (normal, v1);

	Vec_SnapPlane (normal, dist);
}


int
Vec_BoxPlaneSide (const struct plane_s *plane, float mins[3], float maxs[3])
{
	if (plane->type < PLANE_NEAR_X)
	{
		float a = plane->normal[plane->type] * plane->dist;
		if (a <= mins[plane->type])
			return (plane->signbits >> plane->type) & 0x1;
		else if (a >= maxs[plane->type])
			return ((plane->signbits >> plane->type) & 0x1) ^ 0x1;
		else
			return SIDE_CROSS;
	}
	else
	{
		//TODO: ...
	}
}


void
Vec_AnglesVectors (	const float angles[3],
			float right[3],
			float up[3],
			float forward[3])
{
	double cr, sr;
	double cp, sp;
	double cy, sy;

	cr = cos (angles[ROLL]);
	sr = sin (angles[ROLL]);

	cp = cos (angles[PITCH]);
	sp = sin (angles[PITCH]);

	cy = cos (angles[YAW]);
	sy = sin (angles[YAW]);

	//TODO: any common sub-expressions here ?

	/* X * Y * Z */
/*
	right[0] = cy * cr;
	right[1] = sp * sy * cr - cp * sr;
	right[2] = cp * sy * cr + sp * sr;

	up[0] = cy * sr;
	up[1] = sp * sy * sr + cp * cr;
	up[2] = cp * sy * sr - sp * cr;

	forward[0] = -sy;
	forward[1] = sp * cy;
	forward[2] = cp * cy;
*/

	/* Z * Y * X */
	right[0] = cr * cy;
	right[1] = sr * cy;
	right[2] = -sy;

	up[0] = cr * sy * sp - sr * cp;
	up[1] = cr * cp + sr * sy * sp;
	up[2] = cy * sp;

	forward[0] = sr * sp + cr * sy * cp;
	forward[1] = sr * sy * cp - cr * sp;
	forward[2] = cy * cp;
}
