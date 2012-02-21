#ifndef __R_TEX_H__
#define __R_TEX_H__

#include "cdefs.h"

struct tex_s
{
	struct tex_s *next;

	const char *path;
	uint8_t *pixels;
	int w, h;
};

extern struct tex_s *
R_LoadTex (const char *path);

#endif /* __R_TEX_H__ */
