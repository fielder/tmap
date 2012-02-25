#include <string.h>
#include <math.h>

#include "cdefs.h"
#include "vec.h"
#include "geom.h"
#include "view.h"
#include "r_tex.h"
#include "render.h"

struct span_s
{
	int u, v; /* screen coords */
	float si, ti, zi;
	float end_si, end_ti, end_zi;
	int count;
};

struct outvert_s
{
	float u, v; /* screen coords */
	float si, ti; /* texel coords */
	float zi;
};

static struct outvert_s p_outverts[MAX_SURF_VERTS + 1];
static int num_outverts;

static struct span_s spans[MAX_H];
static struct span_s *p_span;

static int p_topidx, p_bottomidx;

//TODO: Ecounter zero-width spans ?
//TODO: When starting to scan out an edge we ceil the v, but don't match
//	up the u with the same adjustment. Is it necessary?

#define AFFINE 0

static void
TransformVec (const float v[3], float out[3])
{
	int i;

	for (i = 0; i < 3; i++)
		out[i] = Vec_Dot (view.xform[i], v);
}


static void
FinishSpans (void)
{
	const struct outvert_s *ov, *nv;
	int i, nexti;
	int itop, ibot;
	int u, step_u;
	float fstep_u;
	float fstep_si, fstep_ti, fstep_zi;
	float si, ti, zi;

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
			fstep_u = (nv->u - ov->u) / (nv->v - ov->v);
			step_u = fstep_u * 0x10000;

			fstep_si = (nv->si - ov->si) / (nv->v - ov->v);
			fstep_ti = (nv->ti - ov->ti) / (nv->v - ov->v);
			fstep_zi = (nv->zi - ov->zi) / (nv->v - ov->v);

			si = ov->si;
			ti = ov->ti;
			zi = ov->zi;
			u = ov->u * 0x10000;
			while (itop < ibot)
			{
				p_span->count = (u >> 16) - p_span->u;

//FIXME: we see -1 length spans when the view is nearly right on the plane
				if (p_span->count < 0)
					p_span->count = 0;

				p_span->end_si = si;
				p_span->end_ti = ti;
				p_span->end_zi = zi;

				u += step_u;
				si += fstep_si;
				ti += fstep_ti;
				zi += fstep_zi;
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
	float fstep_u;
	float fstep_si, fstep_ti, fstep_zi;
	float si, ti, zi;

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
			fstep_u = (nv->u - ov->u) / (nv->v - ov->v);
			step_u = fstep_u * 0x10000;

			fstep_si = (nv->si - ov->si) / (nv->v - ov->v);
			fstep_ti = (nv->ti - ov->ti) / (nv->v - ov->v);
			fstep_zi = (nv->zi - ov->zi) / (nv->v - ov->v);

			si = ov->si;
			ti = ov->ti;
			zi = ov->zi;
			u = ov->u * 0x10000;
			while (itop < ibot)
			{
				p_span->u = u >> 16;
				p_span->v = itop;
				p_span->si = si;
				p_span->ti = ti;
				p_span->zi = zi;

				u += step_u;
				si += fstep_si;
				ti += fstep_ti;
				zi += fstep_zi;
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
DrawTexturedSpans (const struct tex_s *tex)
{
	struct span_s *span;

	for (span = spans; span != p_span; span++)
	{
		int i;
		uint8_t *dest = r_buf + span->v * r_w + span->u;
		float si_inc = (span->end_si - span->si) / span->count;
		float ti_inc = (span->end_ti - span->ti) / span->count;
		float zi_inc = (span->end_zi - span->zi) / span->count;
		float si, ti, zi;

		for (	i = 0, si = span->si, ti = span->ti, zi = span->zi;
			i < span->count;
			i++, si += si_inc, ti += ti_inc, zi += zi_inc)
		{
#if AFFINE
			int s = si;
			int t = ti;
#else
			int s = si / zi;
			int t = ti / zi;
#endif
			dest[i] = tex->pixels[t * tex->w + s];
		}
	}
}


static void
DrawFlatSpans (int color)
{
	struct span_s *span;

	for (span = spans; span != p_span; span++)
		memset (r_buf + span->v * r_w + span->u, color, span->count);
}


static void
DrawTexturedPoly (	struct msurf_s *s,
			float verts[MAX_SURF_VERTS + 1][5],
			int numverts)
{
	int color = (uintptr_t)s >> 4;
	int i;
	float local[3], v[3];
	struct outvert_s *ov;
	float min_v, max_v;

	min_v = 99999.0;
	max_v = -99999.0;

	num_outverts = 0;

	for (i = 0; i < numverts; i++)
	{
		Vec_Subtract (verts[i], view.pos, local);
		TransformVec (local, v);

		ov = p_outverts + num_outverts++;

		ov->zi = 1.0 / v[2];
		ov->u = view.center_x + view.dist * ov->zi * v[0];
		ov->v = view.center_y - view.dist * ov->zi * v[1];
#if AFFINE
		ov->si = verts[i][3];
		ov->ti = verts[i][4];
#else
		ov->si = verts[i][3] * ov->zi;
		ov->ti = verts[i][4] * ov->zi;
#endif

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
		DrawTexturedSpans (s->tex);
	else
		DrawFlatSpans (color);
}


static int
ClipPoly (	float verts[MAX_SURF_VERTS + 1][5],
		float outverts[MAX_SURF_VERTS + 1][5],
		int count,
		const struct plane_s *p)
{
	float dists[MAX_SURF_VERTS + 1];
	float frac;
	int i;
	int numout;

	if (count < 3)
		return 0;

	for (i = 0; i < count; i++)
		dists[i] = Vec_Dot(verts[i], p->normal) - p->dist;

	/* make wrap-around edge easier to handle */
	dists[i] = dists[0];
	verts[i][0] = verts[0][0];
	verts[i][1] = verts[0][1];
	verts[i][2] = verts[0][2];
	verts[i][3] = verts[0][3];
	verts[i][4] = verts[0][4];

	numout = 0;
	for (i = 0; i < count; i++)
	{
		float *cur = verts[i];
		float *next = verts[i + 1];

		if (dists[i] >= 0.0)
		{
			outverts[numout][0] = cur[0];
			outverts[numout][1] = cur[1];
			outverts[numout][2] = cur[2];
			outverts[numout][3] = cur[3];
			outverts[numout][4] = cur[4];
			numout++;
		}

		if (dists[i] == 0.0 || dists[i + 1] == 0.0)
			continue;

		if ((dists[i] > 0.0) == (dists[i + 1] > 0.0))
			continue;

		frac = dists[i] / (dists[i] - dists[i + 1]);
		outverts[numout][0] = cur[0] + frac * (next[0] - cur[0]);
		outverts[numout][1] = cur[1] + frac * (next[1] - cur[1]);
		outverts[numout][2] = cur[2] + frac * (next[2] - cur[2]);
		outverts[numout][3] = cur[3] + frac * (next[3] - cur[3]);
		outverts[numout][4] = cur[4] + frac * (next[4] - cur[4]);
		numout++;
	}

	return numout;
}


static int
ExtractSurfaceVerts (	const struct msurf_s *s,
			float outverts[MAX_SURF_VERTS + 1][5])
{
	const unsigned int *edgenums;
	unsigned int edgenum;
	int vnum;
	int i;

	edgenums = g_surfedges + s->firstedge;
	for (i = 0; i < s->numedges; i++)
	{
		edgenum = edgenums[i];

		if (edgenum & 0x80000000)
			vnum = g_edges[edgenum & 0x7fffffff].v[1];
		else
			vnum = g_edges[edgenum].v[0];

		outverts[i][0] = g_verts[vnum][0];
		outverts[i][1] = g_verts[vnum][1];
		outverts[i][2] = g_verts[vnum][2];
	}

	return s->numedges;
}


static void
DrawTexturedSurf (struct msurf_s *s)
{
	int i;
	int numverts;
	float verts[2][MAX_SURF_VERTS + 1][5];
	int clipidx = 0;

	if (Vec_Dot(s->normal, view.pos) - s->dist < PLANE_DIST_EPSILON)
		return;

	numverts = ExtractSurfaceVerts (s, verts[clipidx]);

	{
		float adj = 0.01; /* adjust inward pixel count */
		verts[clipidx][0][3] = 0.0 + adj;
		verts[clipidx][0][4] = 0.0 + adj;

		verts[clipidx][1][3] = 0.0 + adj;
		verts[clipidx][1][4] = s->tex->h - adj;

		verts[clipidx][2][3] = s->tex->w - adj;
		verts[clipidx][2][4] = s->tex->h - adj;

		verts[clipidx][3][3] = s->tex->w - adj;
		verts[clipidx][3][4] = 0.0 + adj;
	}

	/* clip against view planes */
	for (i = 0; i < 4; i++)
	{
		numverts = ClipPoly (	verts[clipidx],
					verts[!clipidx],
					numverts,
					&view.planes[i]);
		clipidx = !clipidx;
	}

	if (numverts >= 3)
		DrawTexturedPoly (s, verts[clipidx], numverts);
}


void
R_DrawGeometry (void)
{
	int i;

	for (i = 0; i < g_numsurfs; i++)
	{
		struct msurf_s *s = &g_surfs[i];

		if (s->tex == NULL)
			s->tex = R_LoadTex (s->texpath);

		DrawTexturedSurf (s);
	}
}
