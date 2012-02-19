#ifndef __VIEW_H__
#define __VIEW_H__

#include "vec.h"

enum
{
	VPLANE_LEFT,
	VPLANE_TOP,
	VPLANE_RIGHT,
	VPLANE_BOTTOM
};

struct view_s
{
	float center_x;
	float center_y;

	float fov_x; /* radians */
	float fov_y; /* radians */

	float dist;

	float pos[3];

	float angles[3]; /* radians */

	float right[3];
	float up[3];
	float forward[3];

	float xform[3][3]; /* world-to-camera */

	struct plane_s planes[4];
};

extern struct view_s view;

extern void
View_Setup (int w, int h, float fov_x);

extern void
View_SetupFrustum (void);

#endif /* __VIEW_H__ */
