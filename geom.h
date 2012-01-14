#ifndef __GEOM_H__
#define __GEOM_H__

#include "cdefs.h"
#include "render.h"

struct medge_s
{
	unsigned short v[2];
};

struct msurf_s
{
	int firstedge; /* in g_surfedges[] */
	int numedges;

	float normal[3];
	float dist;

	struct tex_s tex;

	uint8_t *cache;
	int cache_w, cache_h;
};

#define MAX_VERTEXES	512
#define MAX_EDGES	512
#define MAX_SURF_VERTS	32
#define MAX_SURFACES	16

extern float g_verts[MAX_VERTEXES][3];
extern int g_numverts;

extern struct medge_s g_edges[MAX_EDGES];
extern int g_numedges;

extern struct msurf_s g_surfs[MAX_SURFACES];
extern int g_numsurfs;

extern unsigned int g_surfedges[MAX_EDGES];
extern int g_numsurfedges;

extern void
Geom_Setup (void);

#endif /* __GEOM_H__ */
