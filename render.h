#ifndef __RENDER_H__
#define __RENDER_H__

#include "cdefs.h"
#include "vec.h"

/* render.c */
#define MAX_W 1600
#define MAX_H 1200

extern int r_w;
extern int r_h;
extern uint8_t *r_buf;

extern void
R_Init (void);
extern void
R_Shutdown (void);

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
