#include <math.h>
#include <string.h>

#include "cdefs.h"
#include "pcx.h"
#include "render.h"

struct tex_s
{
	uint8_t *pixels;
	int w, h;
};

static struct tex_s tex_floor;
static struct tex_s tex_side;
static struct tex_s tex_front;

static uint8_t r_pal[768];

struct edge_s
{
	unsigned short v[2];
};

struct surf_s
{
	int firstedge;
	int numedges;

	struct tex_s *tex;

	uint8_t *cache;
	int cache_w, cache_h;
};

static float r_verts[128][3];
static int num_verts;

static struct edge_s r_edges[128];
static int num_edges;

static struct surf_s r_surfs[16];
static int num_surfs;

static unsigned int r_surfedges[128];
static int num_surfedges;


static void
AddVertex (float x, float y, float z)
{
	r_verts[num_verts][0] = x;
	r_verts[num_verts][1] = y;
	r_verts[num_verts][2] = z;
	num_verts++;
}


static void
AddEdge (int v1, int v2)
{
	r_edges[num_edges].v[0] = v1;
	r_edges[num_edges].v[1] = v2;
	num_edges++;
}


static void
AddSurfEdge (int v1, int v2)
{
	const struct edge_s *e;
	int i;

	for (i = 0, e = r_edges; i < num_edges; i++, e++)
	{
		if (e->v[0] == v1 && e->v[1] == v2)
		{
			r_surfedges[num_surfedges++] = i;
			break;
		}
		else if (e->v[0] == v2 && e->v[1] == v1)
		{
			r_surfedges[num_surfedges++] = i | 0x80000000;
			break;
		}
	}
}


static void
AddQuad (struct tex_s *tex, int v1, int v2, int v3, int v4)
{
	struct surf_s *s;

	s = &r_surfs[num_surfs++];
	memset (s, 0, sizeof(*s));

	s->tex = tex;
	s->firstedge = num_surfedges;
	AddSurfEdge (v1, v2);
	AddSurfEdge (v2, v3);
	AddSurfEdge (v3, v4);
	AddSurfEdge (v4, v1);
	s->numedges = num_surfedges - s->firstedge;
}


static void
LoadTex (struct tex_s *tex, const char *path)
{
	tex->pixels = LoadPCX (path, &tex->w, &tex->h, r_pal);
}


static void
AdjustGeometry (float x, float y, float z)
{
	int i;

	for (i = 0; i < num_verts; i++)
	{
		r_verts[i][0] += x;
		r_verts[i][1] += y;
		r_verts[i][2] += z;
	}
}


void
R_SetupGeometry (void)
{
	float x, y, z;

	LoadTex (&tex_floor, "CEIL5_2.pcx");
	LoadTex (&tex_side, "WALL42_3.pcx");
	LoadTex (&tex_front, "WALL42_1.pcx");

	Vid_SetPalette (r_pal);

	num_verts = 0;
	num_edges = 0;
	num_surfs = 0;
	num_surfedges = 0;

	x = tex_front.w / 2.0;
	y = tex_front.h / 2.0;
	z = tex_side.w / 2.0;
	AddVertex ( x,  y,  z); /* top verts */
	AddVertex (-x,  y,  z);
	AddVertex (-x,  y, -z);
	AddVertex ( x,  y, -z);
	AddVertex ( x, -y,  z); /* bottom verts */
	AddVertex (-x, -y,  z);
	AddVertex (-x, -y, -z);
	AddVertex ( x, -y, -z);

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

	AddQuad (&tex_front, 3, 2, 6, 7); /* front */
	AddQuad (&tex_front, 1, 0, 4, 5); /* back */
	AddQuad (&tex_side, 0, 3, 7, 4); /* right */
	AddQuad (&tex_side, 2, 1, 5, 6); /* left */
	AddQuad (&tex_floor, 0, 1, 2, 3); /* top */
	AddQuad (&tex_floor, 7, 6, 5, 4); /* bottom */

	AdjustGeometry (-48, 0, 128);
}

/* ================================================================== */

#define FOV_X 90.0

struct
{
	bool inited;

	float center_x;
	float center_y;

	/* radians */
	float fov_x;
	float fov_y;

	float dist;
	float scale_y;
} view;


static void
SetupView (void)
{
	if (view.inited)
		return;
	view.inited = true;

	view.center_x = r_w / 2.0 - 0.5;
	view.center_y = r_h / 2.0 - 0.5;

	view.fov_x = FOV_X * (M_PI / 180.0);
	view.dist = (r_w / 2.0) / tan(view.fov_x / 2.0);
	view.fov_y = 2.0 * atan((r_h / 2.0) / view.dist);
}


void
R_DrawGeometry (void)
{
	int i;

	SetupView ();

	for (i = 0; i < num_verts; i++)
	{
		const float *v = r_verts[i];
		int x, y;
		x = view.center_x + view.dist * (v[0] / v[2]);
		y = view.center_y + view.dist * (v[1] / v[2]);
		r_buf[y * r_w + x] = 4;
	}
}
