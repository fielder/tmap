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
	if (len == 0)
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
