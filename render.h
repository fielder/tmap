#ifndef __RENDER_H__
#define __RENDER_H__

#include "cdefs.h"

/* render.c */
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
R_Refresh (void);

/* rast.c */
extern void
R_SetupGeometry (void);
extern void
R_DrawGeometry (void);

#endif /* __RENDER_H__ */
