#include <math.h>

#include "vec.h"
#include "view.h"

struct view_s view;


void
View_Setup (int w, int h, float fov_x)
{
	view.center_x = w / 2.0 - 0.5;
	view.center_y = h / 2.0 - 0.5;

	view.fov_x = fov_x;
	view.dist = (w / 2.0) / tan(view.fov_x / 2.0);
	view.fov_y = 2.0 * atan((h / 2.0) / view.dist);

	Vec_Clear (view.pos);
	Vec_Clear (view.angles);

	view.angles[1] = M_PI; /* start off looking to +z axis */
}


void
View_SetupFrustum (void)
{
	int i;
	float n[3];
	float xform[3][3];
	float ang, adj;
	struct plane_s *p;

	Vec_AnglesMatrix (view.angles, xform, ROT_MATRIX_ORDER_ZYX);

	/* DEBUG: adjust view pyramid inward */
	adj = 2.0 * (M_PI / 180.0);

	/* left plane */
	p = &view.planes[VPLANE_LEFT];
	ang = (view.fov_x / 2.0) - adj;
	n[0] = cos (ang);
	n[1] = 0.0;
	n[2] = -sin (ang);
	for (i = 0; i < 3; i++) /* rotate with view angles */
		p->normal[i] = Vec_Dot (xform[i], n);
	p->dist = Vec_Dot (p->normal, view.pos);

	/* right plane */
	p = &view.planes[VPLANE_RIGHT];
	ang = M_PI - ((view.fov_x / 2.0) - adj);
	n[0] = cos (ang);
	n[1] = 0.0;
	n[2] = -sin (ang);
	for (i = 0; i < 3; i++) /* rotate with view angles */
		p->normal[i] = Vec_Dot (xform[i], n);
	p->dist = Vec_Dot (p->normal, view.pos);

	/* bottom plane */
	p = &view.planes[VPLANE_BOTTOM];
	ang = (M_PI / 2.0) - ((view.fov_y / 2.0) - adj);
	n[0] = 0.0;
	n[1] = sin (ang);
	n[2] = -cos (ang);
	for (i = 0; i < 3; i++) /* rotate with view angles */
		p->normal[i] = Vec_Dot (xform[i], n);
	p->dist = Vec_Dot (p->normal, view.pos);

	/* top plane */
	p = &view.planes[VPLANE_TOP];
	ang = -(M_PI / 2.0) + ((view.fov_y / 2.0) - adj);
	n[0] = 0.0;
	n[1] = sin (ang);
	n[2] = -cos (ang);
	for (i = 0; i < 3; i++) /* rotate with view angles */
		p->normal[i] = Vec_Dot (xform[i], n);
	p->dist = Vec_Dot (p->normal, view.pos);
}
