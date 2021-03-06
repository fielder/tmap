#include <string.h>

#include "vec.h"
#include "render.h"
#include "geom.h"

float g_verts[MAX_VERTEXES][3];
int g_numverts;

struct medge_s g_edges[MAX_EDGES];
int g_numedges;

struct msurf_s g_surfs[MAX_SURFACES];
int g_numsurfs;

unsigned int g_surfedges[MAX_EDGES];
int g_numsurfedges;


static void
AddVertex (float x, float y, float z)
{
	g_verts[g_numverts][0] = x;
	g_verts[g_numverts][1] = y;
	g_verts[g_numverts][2] = z;
	g_numverts++;
}


static void
AddEdge (int v1, int v2)
{
	g_edges[g_numedges].v[0] = v1;
	g_edges[g_numedges].v[1] = v2;
	g_numedges++;
}


static void
AddSurfEdge (int v1, int v2)
{
	const struct medge_s *e;
	int i;

	for (i = 0, e = g_edges; i < g_numedges; i++, e++)
	{
		if (e->v[0] == v1 && e->v[1] == v2)
		{
			g_surfedges[g_numsurfedges++] = i;
			break;
		}
		else if (e->v[0] == v2 && e->v[1] == v1)
		{
			g_surfedges[g_numsurfedges++] = i | 0x80000000;
			break;
		}
	}
}


static void
AddQuad (const char *texpath, int v1, int v2, int v3, int v4)
{
	struct msurf_s *s;

	s = &g_surfs[g_numsurfs++];
	memset (s, 0, sizeof(*s));

	s->firstedge = g_numsurfedges;
	AddSurfEdge (v1, v2);
	AddSurfEdge (v2, v3);
	AddSurfEdge (v3, v4);
	AddSurfEdge (v4, v1);
	s->numedges = g_numsurfedges - s->firstedge;

	Vec_MakeNormal (g_verts[v1],
			g_verts[v2],
			g_verts[v3],
			s->normal,
			&s->dist);

	s->texpath = texpath;
}


void
Geom_Setup (void)
{
	float x, y, z;
	float dx, dy, dz;

	g_numverts = 0;
	g_numedges = 0;
	g_numsurfs = 0;
	g_numsurfedges = 0;

	/* ================================ */

	dx = -64;
	dy = 16;
	dz = 128;

	x = 20;
	y = 64;
	z = 12;
	AddVertex (dx +  x, dy +  y, dz +  z); /* top verts */
	AddVertex (dx + -x, dy +  y, dz +  z);
	AddVertex (dx + -x, dy +  y, dz + -z);
	AddVertex (dx +  x, dy +  y, dz + -z);
	AddVertex (dx +  x, dy + -y, dz +  z); /* bottom verts */
	AddVertex (dx + -x, dy + -y, dz +  z);
	AddVertex (dx + -x, dy + -y, dz + -z);
	AddVertex (dx +  x, dy + -y, dz + -z);

	AddEdge (0, 1); /* top */
	AddEdge (1, 2);
	AddEdge (2, 3);
	AddEdge (3, 0);
	AddEdge (0, 4); /* sides */
	AddEdge (1, 5);
	AddEdge (2, 6);
	AddEdge (3, 7);
	AddEdge (4, 5); /* bottom */
	AddEdge (5, 6);
	AddEdge (6, 7);
	AddEdge (7, 4);

	AddQuad ("WALL42_1.pcx", 3, 7, 6, 2); /* front */
	AddQuad ("WALL42_1.pcx", 1, 5, 4, 0); /* back */
	AddQuad ("WALL42_3.pcx", 0, 4, 7, 3); /* right */
	AddQuad ("WALL42_3.pcx", 2, 6, 5, 1); /* left */
	AddQuad ("CEIL5_2.pcx", 0, 3, 2, 1); /* top */
	AddQuad ("CEIL5_2.pcx", 7, 4, 5, 6); /* bottom */

	/* ================================ */

	dx = 128;
	dy = 128;
	dz = 256;

	x = 32;
	y = 32;
	z = 32;
	AddVertex (dx +  x, dy +  y, dz +  z); /* top verts */
	AddVertex (dx + -x, dy +  y, dz +  z);
	AddVertex (dx + -x, dy +  y, dz + -z);
	AddVertex (dx +  x, dy +  y, dz + -z);
	AddVertex (dx +  x, dy + -y, dz +  z); /* bottom verts */
	AddVertex (dx + -x, dy + -y, dz +  z);
	AddVertex (dx + -x, dy + -y, dz + -z);
	AddVertex (dx +  x, dy + -y, dz + -z);

	AddEdge (8, 9); /* top */
	AddEdge (9, 10);
	AddEdge (10, 11);
	AddEdge (11, 8);
	AddEdge (8, 12); /* sides */
	AddEdge (9, 13);
	AddEdge (10, 14);
	AddEdge (11, 15);
	AddEdge (12, 13); /* bottom */
	AddEdge (13, 14);
	AddEdge (14, 15);
	AddEdge (15, 12);

	AddQuad ("W28_5.pcx", 11, 15, 14, 10); /* front */
	AddQuad ("W28_5.pcx", 9, 13, 12, 8); /* back */
	AddQuad ("W28_5.pcx", 8, 12, 15, 11); /* right */
	AddQuad ("W28_5.pcx", 10, 14, 13, 9); /* left */
	AddQuad ("W28_5.pcx", 8, 11, 10, 9); /* top */
	AddQuad ("W28_5.pcx", 15, 12, 13, 14); /* bottom */

	/* ================================ */

	AddVertex (8, 0, 512);
	AddVertex (8, -128, 512);
	AddVertex (0, -128, 512);
	AddVertex (0, 0, 512);

	AddEdge (16, 17);
	AddEdge (17, 18);
	AddEdge (18, 19);
	AddEdge (19, 16);

	AddQuad ("AGB128_1.pcx", 16, 17, 18, 19); /* front */
}
