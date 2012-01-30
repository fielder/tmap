#ifndef __RENDER_H__
#define __RENDER_H__

#include "cdefs.h"

struct tex_s
{
	uint8_t *pixels;
	int w, h;
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
};

/* render.c */
#define MAX_W 1600
#define MAX_H 1200

extern int r_w;
extern int r_h;
extern uint8_t *r_buf;
extern struct view_s view;

extern void
R_Init (void);
extern void
R_Shutdown (void);

extern void
R_LoadTex (struct tex_s *tex, const char *path);

extern void
Vid_SetPalette (const uint8_t palette[768]);

extern void
R_Line (int x1, int y1, int x2, int y2, int c);

extern void
R_Refresh (void);

/* rast.c */

extern void
R_DrawGeometry (void);

#endif /* __RENDER_H__ */
