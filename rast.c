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

	float normal[3];
	float dist;

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

/* ================================================================== */

static void
Vec_Copy (const float src[3], float out[3])
{
	out[0] = src[0];
	out[1] = src[1];
	out[2] = src[2];
}


static float
Vec_Dot (const float a[3], const float b[3])
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}


static void
Vec_Cross (const float a[3], const float b[3], float out[3])
{
	out[0] = a[1] * b[2] - a[2] * b[1];
	out[1] = a[2] * b[0] - a[0] * b[2];
	out[2] = a[0] * b[1] - a[1] * b[0];
}


static void
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


static void
Vec_Subtract (const float a[3], const float b[3], float out[3])
{
	out[0] = a[0] - b[0];
	out[1] = a[1] - b[1];
	out[2] = a[2] - b[2];
}


static void
Vec_Add (const float a[3], const float b[3], float out[3])
{
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
	out[2] = a[2] + b[2];
}

/* ================================================================== */

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
	float a[3], b[3];

	s = &r_surfs[num_surfs++];
	memset (s, 0, sizeof(*s));

	s->tex = tex;
	s->firstedge = num_surfedges;
	AddSurfEdge (v1, v2);
	AddSurfEdge (v2, v3);
	AddSurfEdge (v3, v4);
	AddSurfEdge (v4, v1);
	s->numedges = num_surfedges - s->firstedge;

	Vec_Subtract (r_verts[v2], r_verts[v1], a);
	Vec_Subtract (r_verts[v3], r_verts[v1], b);
	Vec_Cross (a, b, s->normal);
	Vec_Normalize (s->normal);
	s->dist = Vec_Dot (s->normal, r_verts[v1]);
}


static void
LoadTex (struct tex_s *tex, const char *path)
{
	tex->pixels = LoadPCX (path, &tex->w, &tex->h, r_pal);
}


void
R_SetupGeometry (void)
{
	float x, y, z;
	float dx, dy, dz;

	LoadTex (&tex_floor, "CEIL5_2.pcx");
	LoadTex (&tex_side, "WALL42_3.pcx");
	LoadTex (&tex_front, "WALL42_1.pcx");

	Vid_SetPalette (r_pal);

	num_verts = 0;
	num_edges = 0;
	num_surfs = 0;
	num_surfedges = 0;

	dx = -48;
	dy = 0;
	dz = 128;

	x = tex_front.w / 2.0;
	y = tex_front.h / 2.0;
	z = tex_side.w / 2.0;
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

	AddQuad (&tex_front, 3, 7, 6, 2); /* front */
	AddQuad (&tex_front, 1, 5, 4, 0); /* back */
	AddQuad (&tex_side, 0, 4, 7, 3); /* right */
	AddQuad (&tex_side, 2, 6, 5, 1); /* left */
	AddQuad (&tex_floor, 0, 3, 2, 1); /* top */
	AddQuad (&tex_floor, 7, 4, 5, 6); /* bottom */
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

	float forward[3];
	float right[3];
	float up[3];
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

	view.right[0] = 1.0;
	view.right[1] = 0.0;
	view.right[2] = 0.0;
	view.up[0] = 0.0;
	view.up[1] = 1.0;
	view.up[2] = 0.0;
	view.forward[0] = 0.0;
	view.forward[1] = 0.0;
	view.forward[2] = 1.0;
}


static void
DrawPoint (const float v[3])
{
	int x, y;
	x = view.center_x + view.dist * (v[0] / v[2]);
	y = view.center_y + view.dist * (v[1] / v[2]);
	r_buf[y * r_w + x] = 4;
}


static void
DrawSurf (struct surf_s *s)
{
	int i;

	if (Vec_Dot(s->normal, view.forward) - s->dist <= 0)
		return;

	for (i = 0; i < s->numedges; i++)
	{
		unsigned int edgenum = r_surfedges[s->firstedge + i];
		int v1, v2;
		float *a, *b;
		float x1, y1, x2, y2;

		if (edgenum & 0x80000000)
		{
			edgenum &= 0x7fffffff;
			v1 = r_edges[edgenum].v[1];
			v2 = r_edges[edgenum].v[0];
		}
		else
		{
			v1 = r_edges[edgenum].v[0];
			v2 = r_edges[edgenum].v[1];
		}

		a = r_verts[v1];
		b = r_verts[v2];

		x1 = view.center_x + view.dist * (a[0] / a[2]);
		y1 = view.center_y + view.dist * (a[1] / a[2]);

		x2 = view.center_x + view.dist * (b[0] / b[2]);
		y2 = view.center_y + view.dist * (b[1] / b[2]);

		R_Line (x1, y1, x2, y2, 4);
	}
}


void
R_DrawGeometry (void)
{
	int i;

	SetupView ();

/*
	for (i = 0; i < num_verts; i++)
		DrawPoint (r_verts[i]);
*/

	for (i = 0; i < num_surfs; i++)
		DrawSurf (r_surfs + i);
}
