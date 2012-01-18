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

float r_viewpos[3];

static struct outvert_s p_outverts[MAX_SURF_VERTS];
static int num_outverts;

static struct span_s spans[MAX_H];
static struct span_s *p_span;

static int p_topidx, p_bottomidx;

//TODO: Ecounter zero-width spans ?
//TODO: When starting to scan out an edge we ceil the v, but don't match
//	up the u with the same adjustment. Is it necessary?

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
				p_span->count = (u >> 16) - p_span->u;
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
	float v[3];
	struct outvert_s *ov;
	float min_v, max_v;

	if (Vec_Dot(s->normal, r_viewpos) - s->dist < PLANE_DIST_EPSILON)
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
		Vec_Subtract (g_verts[vnum], r_viewpos, v);

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


void
R_DrawGeometry (void)
{
	int i;

	for (i = 0; i < g_numsurfs; i++)
		DrawSurf (&g_surfs[i]);
}
