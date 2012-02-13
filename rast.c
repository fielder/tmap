#include <string.h>
#include <math.h>

#include "cdefs.h"
#include "vec.h"
#include "geom.h"
#include "render.h"


struct span_s
{
	int u, v;
	int count;
};

struct outvert_s
{
	float u, v; /* screen coords */
	float s, t; /* texel coords */
	float zi;
};

static struct outvert_s p_outverts[MAX_SURF_VERTS];
static int num_outverts;

static struct span_s spans[MAX_H];
static struct span_s *p_span;

static int p_topidx, p_bottomidx;

//TODO: Ecounter zero-width spans ?
//TODO: When starting to scan out an edge we ceil the v, but don't match
//	up the u with the same adjustment. Is it necessary?


static void
TransformVec (const float v[3], float out[3])
{
	int i;

	for (i = 0; i < 3; i++)
		out[i] = Vec_Dot (view.xform[i], v);
}


static void
DrawPoint (const float v[3], int c)
{
	float transformed[3];
	float out[3];

	Vec_Subtract (v, view.pos, transformed);
	TransformVec (transformed, out);

	if (out[2] > 0.0)
	{
		float zi;
		int u, v;

		zi = 1.0 / out[2];
		u = ceil(view.center_x + view.dist * zi * out[0]);
		v = ceil(view.center_y - view.dist * zi * out[1]);

		if (	u >= 0 && u < r_w &&
			v >= 0 && v < r_h )
		{
			r_buf[v * r_w + u] = c;
		}
	}
}


static void
Draw3DLine (const float v1[3], const float v2[3], int count, int color)
{
	int j;
	float step;
	float v[3], inc[3];

	Vec_Subtract (v2, v1, inc);
	step = Vec_Length(inc) / count;
	Vec_Normalize (inc);
	Vec_Scale (inc, step);
	Vec_Copy (v1, v);
	for (j = 0; j < count; j++)
	{
		Vec_Add (v, inc, v);
		DrawPoint (v, color);
	}
}


static void
FinishSpans (void)
{
	const struct outvert_s *ov, *nv;
	int i, nexti;
	int itop, ibot;
	int u, step_u;

	p_span = spans;

	i = p_topidx;
	ov = &p_outverts[i];
	itop = ceil(ov->v);
	while (i != p_bottomidx)
	{
		/* find next vert */
		if ((nexti = i - 1) == -1)
			nexti = num_outverts - 1;
		nv = &p_outverts[nexti];
		ibot = ceil(nv->v);

		if (itop < ibot)
		{
			step_u = ((nv->u - ov->u) / (nv->v - ov->v)) * 0x10000;
			u = ov->u * 0x10000;
			while (itop < ibot)
			{
				p_span->count = (u >> 16) - p_span->u;
//FIXME: we see -1 length spans when the view is nearly right on the plane
				if (p_span->count < 0)
					p_span->count = 0;
				u += step_u;
				p_span++;
				itop++;
			}
		}

		/* go to next edge */
		itop = ibot;
		ov = nv;
		i = nexti;
	}
}


static void
BeginSpans (void)
{
	const struct outvert_s *ov, *nv;
	int i, nexti;
	int itop, ibot;
	int u, step_u;

	p_span = spans;

	i = p_topidx;
	ov = &p_outverts[i];
	itop = ceil(ov->v);
	while (i != p_bottomidx)
	{
		/* find next vert */
		if ((nexti = i + 1) == num_outverts)
			nexti = 0;
		nv = &p_outverts[nexti];
		ibot = ceil(nv->v);

		if (itop < ibot)
		{
			step_u = ((nv->u - ov->u) / (nv->v - ov->v)) * 0x10000;
			u = ov->u * 0x10000;
			while (itop < ibot)
			{
				p_span->u = u >> 16;
				p_span->v = itop;
				u += step_u;
				p_span++;
				itop++;
			}
		}

		/* go to next edge */
		itop = ibot;
		ov = nv;
		i = nexti;
	}
}


static void
DrawSurf (struct msurf_s *s)
{
	const unsigned int *edgenums;
	unsigned int edgenum;
	int i;
	int vnum;
	float transformed[3], v[3];
	struct outvert_s *ov;
	float min_v, max_v;

	if (Vec_Dot(s->normal, view.pos) - s->dist < PLANE_DIST_EPSILON)
		return;

	min_v = 99999.0;
	max_v = -99999.0;

	num_outverts = 0;

	edgenums = g_surfedges + s->firstedge;
	for (i = 0; i < s->numedges; i++)
	{
		edgenum = edgenums[i];
		if (edgenum & 0x80000000)
			vnum = g_edges[edgenum & 0x7fffffff].v[1];
		else
			vnum = g_edges[edgenum].v[0];

		Vec_Subtract (g_verts[vnum], view.pos, transformed);
		TransformVec (transformed, v);

		ov = p_outverts + num_outverts++;

		ov->zi = 1.0 / v[2];
		ov->u = view.center_x + view.dist * ov->zi * v[0];
		ov->v = view.center_y - view.dist * ov->zi * v[1];
		ov->s = 0;
		ov->t = 0;

		/* find top & bottom verts */
		if (ov->v < min_v)
		{
			min_v = ov->v;
			p_topidx = i;
		}
		if (ov->v > max_v)
		{
			max_v = ov->v;
			p_bottomidx = i;
		}
	}

	if (min_v >= max_v)
		return;

	BeginSpans ();
	FinishSpans ();

	if (1)
	{
		struct span_s *span;
		for (span = spans; span != p_span; span++)
			memset (r_buf + span->v * r_w + span->u,
				(uintptr_t)s >> 4,
				span->count);
	}
}


static void
DrawSurfEdges (const struct msurf_s *s)
{
	const unsigned int *edgenums;
	unsigned int edgenum;
	int i;
	const float *v1, *v2;
	struct medge_s *medge;

	if (Vec_Dot(s->normal, view.pos) - s->dist < PLANE_DIST_EPSILON)
		return;

	edgenums = g_surfedges + s->firstedge;
	for (i = 0; i < s->numedges; i++)
	{
		edgenum = edgenums[i];
		if (edgenum & 0x80000000)
		{
			medge = &g_edges[edgenum & 0x7fffffff];
			v1 = g_verts[medge->v[1]];
			v2 = g_verts[medge->v[0]];
		}
		else
		{
			medge = &g_edges[edgenum];
			v1 = g_verts[medge->v[0]];
			v2 = g_verts[medge->v[1]];
		}

		Draw3DLine (v1, v2, 256, (uintptr_t)s >> 4);
	}
}


static void
SetupFrustum (void)
{
	int i;
	float dx, dy;
	float v[3];
	float xform[3][3];
	float origin[3];
	float tl[3], tr[3], bl[3], br[3];

	Vec_Clear (origin);

	Vec_AnglesMatrix (view.angles, xform, ROT_MATRIX_ORDER_ZYX);

	dx = view.dist * tan (view.fov_x / 2.0);
	dy = view.dist * tan (view.fov_y / 2.0);

	Vec_Clear (v);
	v[0] = -dx;
	v[1] = dy;
	v[2] = -view.dist;
	for (i = 0; i < 3; i++) /* rotate with view angles */
		tl[i] = Vec_Dot (xform[i], v);
	Draw3DLine (origin, tl, 256, 4);

	Vec_Clear (v);
	v[0] = dx;
	v[1] = dy;
	v[2] = -view.dist;
	for (i = 0; i < 3; i++) /* rotate with view angles */
		tr[i] = Vec_Dot (xform[i], v);
	Draw3DLine (origin, tr, 256, 4);

	Vec_Clear (v);
	v[0] = -dx;
	v[1] = -dy;
	v[2] = -view.dist;
	for (i = 0; i < 3; i++) /* rotate with view angles */
		bl[i] = Vec_Dot (xform[i], v);
	Draw3DLine (origin, bl, 256, 4);

	Vec_Clear (v);
	v[0] = dx;
	v[1] = -dy;
	v[2] = -view.dist;
	for (i = 0; i < 3; i++) /* rotate with view angles */
		br[i] = Vec_Dot (xform[i], v);
	Draw3DLine (origin, br, 256, 4);

	Draw3DLine (tl, tr, 256, 4);
	Draw3DLine (tr, br, 256, 4);
	Draw3DLine (br, bl, 256, 4);
	Draw3DLine (bl, tl, 256, 4);
}


void
R_DrawGeometry (void)
{
	int i;

	SetupFrustum ();

	for (i = 0; i < g_numsurfs; i++)
	{
//		DrawSurf (&g_surfs[i]);
		DrawSurfEdges (&g_surfs[i]);
	}

#if 1
	{
		float v[3];

		for (i = 0; i < 128; i++)
		{
			Vec_Clear (v); v[0] = i;
			DrawPoint (v, 176);

			Vec_Clear (v); v[1] = i;
			DrawPoint (v, 112);

			Vec_Clear (v); v[2] = i;
			DrawPoint (v, 198);
		}
	}
#endif
}
