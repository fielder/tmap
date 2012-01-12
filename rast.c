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

struct medge_s
{
	unsigned short v[2];
};

struct msurf_s
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

static struct medge_s r_edges[128];
static int num_edges;

static struct msurf_s r_surfs[16];
static int num_surfs;

static unsigned int r_surfedges[128];
static int num_surfedges;

/* ================================================================== */

#define PLANE_DIST_EPSILON 0.01
#define PLANE_NORMAL_EPSILON 0.00001

static void
Vec_Clear (float v[3])
{
	v[0] = v[1] = v[2] = 0.0;
}


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
		Vec_Clear (v);
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


static void
Vec_SnapPlane (float normal[3], float *dist)
{
	int i;
	float idist;

	for (i = 0; i < 3; i++)
	{
		if (fabs(normal[i] - 1.0) < PLANE_NORMAL_EPSILON)
		{
			Vec_Clear (normal);
			normal[i] = 1.0;
			break;
		}
		else if (fabs(normal[i] - -1.0) < PLANE_NORMAL_EPSILON)
		{
			Vec_Clear (normal);
			normal[i] = -1.0;
			break;
		}
	}

	idist = floor (*dist + 0.5);
	if (fabs(*dist - idist) < PLANE_DIST_EPSILON)
		*dist = idist;
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
	const struct medge_s *e;
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
	struct msurf_s *s;
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
	Vec_SnapPlane (s->normal, &s->dist);
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
	dy = -15;
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

struct outvert_s
{
	float u, v; /* screen coords */
	float s, t; /* texel coords */
	float zi;
};


struct outedge_s
{
	struct outedge_s *next;
	int u, u_step;
}; 


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


static struct outvert_s p_outverts[32];
static int num_outverts;

static struct outedge_s p_outedges[32];
static int num_outedges;
static struct outedge_s edges_l, edges_r;

static float p_min_v, p_max_v;
static int p_topi;


static void
GenerateSpans (void)
{
}


//FIXME: We're going to have to snap u/v to integers eventually
//	Will require adjusting non-integer values to match the shift
static void
ScanEdges (void)
{
	struct outvert_s *ov, *nv;
	int i;
	int bottomi;

	struct outedge_s *prev_le, *prev_re;
	struct outedge_s *oe;
	int nexti;

	float u_step;

	/* find top and bottom verts */
	p_min_v = 99999.0;
	p_max_v = -99999.0;
	for (i = 0, ov = p_outverts; i < num_outverts; i++, ov++)
	{
		if (ov->v < p_min_v)
		{
			p_min_v = ov->v;
			p_topi = i;
		}
		if (ov->v > p_max_v)
		{
			p_max_v = ov->v;
			bottomi = i;
		}
	}

	num_outedges = 0;

	/* generate left edges */
	edges_l.next = NULL;
	prev_le = &edges_l;
	i = p_topi;
	ov = &p_outverts[i];
	while (i != bottomi)
	{
		/* find next vert */
		if ((nexti = i - 1) == -1)
			nexti = num_outverts - 1;
		nv = &p_outverts[nexti];

		/* get a new edge & link in */
		oe = &p_outedges[num_outedges++];
		oe->next = NULL;
		prev_le->next = oe;
		prev_le = oe;

		/* calculate gradient */
		oe->u = ov->u * 0x100000;
		u_step = (nv->u - ov->u) / (nv->v - ov->v);
		oe->u_step = u_step * 0x100000;

		/* go to next vert */
		i = nexti;
		ov = nv;
	}

	/* generate right edges */
	edges_r.next = NULL;
	prev_re = &edges_r;
	i = p_topi;
	ov = &p_outverts[i];
	while (i != bottomi)
	{
		/* find next vert */
		if ((nexti = i + 1) == num_outverts)
			nexti = 0;
		nv = &p_outverts[nexti];

		/* get a new edge & link in */
		oe = &p_outedges[num_outedges++];
		oe->next = NULL;
		prev_re->next = oe;
		prev_re = oe;

		/* calculate gradient */
		oe->u = ov->u * 0x100000;
		u_step = (nv->u - ov->u) / (nv->v - ov->v);
		oe->u_step = u_step * 0x100000;

		/* go to next vert */
		i = nexti;
		ov = nv;
	}
}


static void
DrawSurf (struct msurf_s *s)
{
	const unsigned int *edgenums;
	unsigned int edgenum;
	int i;
	int vnum;
	const float *v;
	struct outvert_s *ov;

	if (Vec_Dot(s->normal, view.forward) - s->dist < PLANE_DIST_EPSILON)
		return;

	num_outverts = 0;

	edgenums = r_surfedges + s->firstedge;
	for (i = 0; i < s->numedges; i++)
	{
		edgenum = edgenums[i];
		if (edgenum & 0x80000000)
			vnum = r_edges[edgenum & 0x7fffffff].v[1];
		else
			vnum = r_edges[edgenum].v[0];
		v = r_verts[vnum];

		ov = p_outverts + num_outverts++;

		ov->zi = 1.0 / v[2];
		ov->u = view.center_x + view.dist * ov->zi * v[0];
		ov->v = view.center_y + view.dist * ov->zi * v[1];
		ov->s = 0;
		ov->t = 0;
	}

	ScanEdges ();
	GenerateSpans ();
}


void
R_DrawGeometry (void)
{
	int i;

	SetupView ();

	for (i = 0; i < num_surfs; i++)
		DrawSurf (r_surfs + i);
}
