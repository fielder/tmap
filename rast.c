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

static struct outvert_s p_outverts[MAX_SURF_VERTS + 1];
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
	int i;
	float transformed[3];
	float out[3];

	for (i = 0; i < 4; i++)
	{
		const struct plane_s *p = &view.planes[i];
		if (Vec_Dot(p->normal, v) - p->dist < 0.0)
			return;
	}

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
DrawPolyOutline (int color, float verts[MAX_SURF_VERTS + 1][3], int numverts)
{
	int i;
	float local[3], v[3];
	struct outvert_s *ov;

	num_outverts = 0;
	for (i = 0; i < numverts; i++)
	{
		Vec_Subtract (verts[i], view.pos, local);
		TransformVec (local, v);

		ov = p_outverts + num_outverts++;

		ov->zi = 1.0 / v[2];
		ov->u = view.center_x + view.dist * ov->zi * v[0];
		ov->v = view.center_y - view.dist * ov->zi * v[1];
		ov->s = 0;
		ov->t = 0;
	}
	p_outverts[num_outverts].zi = p_outverts[0].zi;
	p_outverts[num_outverts].u = p_outverts[0].u;
	p_outverts[num_outverts].v = p_outverts[0].v;
	p_outverts[num_outverts].s = p_outverts[0].s;
	p_outverts[num_outverts].t = p_outverts[0].t;

	for (i = 0; i < numverts; i++)
	{
		struct outvert_s *a, *b;
		a = &p_outverts[i];
		b = &p_outverts[i + 1];
		R_Line (a->u, a->v, b->u, b->v, color);
	}
}


static void
DrawFilledPoly (int color, float verts[MAX_SURF_VERTS + 1][3], int numverts)
{
	int i;
	float local[3], v[3];
	struct outvert_s *ov;
	float min_v, max_v;
	struct span_s *span;

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

	for (span = spans; span != p_span; span++)
		memset (r_buf + span->v * r_w + span->u, color, span->count);
}


static int
ClipPoly (	float verts[MAX_SURF_VERTS + 1][3],
		float outverts[MAX_SURF_VERTS + 1][3],
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
	dists[i] = dists[0];
	Vec_Copy (verts[0], verts[i]);

	numout = 0;
	for (i = 0; i < count; i++)
	{
		if (dists[i] >= 0.0)
			Vec_Copy (verts[i], outverts[numout++]);

		if (dists[i] == 0.0 || dists[i + 1] == 0.0)
			continue;

		if ((dists[i] > 0.0) == (dists[i + 1] > 0.0))
			continue;

		frac = dists[i] / (dists[i] - dists[i + 1]);
		outverts[numout][0] = verts[i][0] + frac * (verts[i + 1][0] - verts[i][0]);
		outverts[numout][1] = verts[i][1] + frac * (verts[i + 1][1] - verts[i][1]);
		outverts[numout][2] = verts[i][2] + frac * (verts[i + 1][2] - verts[i][2]);
		numout++;
	}

	return numout;
}


static void
DrawFlatSurf (struct msurf_s *s)
{
	const unsigned int *edgenums;
	int i;
	int numverts;
	float verts[2][MAX_SURF_VERTS + 1][3];
	int clipidx = 0;

	if (Vec_Dot(s->normal, view.pos) - s->dist < PLANE_DIST_EPSILON)
		return;

	/* copy vertices */
	edgenums = g_surfedges + s->firstedge;
	for (i = 0; i < s->numedges; i++)
	{
		unsigned int edgenum = edgenums[i];
		int vnum;

		if (edgenum & 0x80000000)
			vnum = g_edges[edgenum & 0x7fffffff].v[1];
		else
			vnum = g_edges[edgenum].v[0];

		Vec_Copy (g_verts[vnum], verts[clipidx][i]);
	}
	Vec_Copy (verts[clipidx][0], verts[clipidx][i]);

	/* clip against view planes */
	numverts = s->numedges;
	for (i = 0; i < 4; i++)
	{
		numverts = ClipPoly (	verts[clipidx],
					verts[!clipidx],
					numverts,
					&view.planes[i]);
		clipidx = !clipidx;
	}

	if (numverts >= 3)
		DrawFilledPoly ((uintptr_t)s >> 4, verts[clipidx], numverts);
}


static void
DrawSurfEdges (const struct msurf_s *s)
{
	const unsigned int *edgenums;
	int i;
	int numverts;
	float verts[2][MAX_SURF_VERTS + 1][3];
	int clipidx = 0;

	if (Vec_Dot(s->normal, view.pos) - s->dist < PLANE_DIST_EPSILON)
		return;

	/* copy vertices */
	edgenums = g_surfedges + s->firstedge;
	for (i = 0; i < s->numedges; i++)
	{
		unsigned int edgenum = edgenums[i];
		int vnum;

		if (edgenum & 0x80000000)
			vnum = g_edges[edgenum & 0x7fffffff].v[1];
		else
			vnum = g_edges[edgenum].v[0];

		Vec_Copy (g_verts[vnum], verts[clipidx][i]);
	}
	Vec_Copy (verts[clipidx][0], verts[clipidx][i]);

	/* clip against view planes */
	numverts = s->numedges;
	for (i = 0; i < 4; i++)
	{
		numverts = ClipPoly (	verts[clipidx],
					verts[!clipidx],
					numverts,
					&view.planes[i]);
		clipidx = !clipidx;
	}

	if (numverts >= 3)
		DrawPolyOutline ((uintptr_t)s >> 4, verts[clipidx], numverts);
}


static void
SetupFrustum (void)
{
	int i;
	float n[3];
	float xform[3][3];
	float ang, adj;

	Vec_AnglesMatrix (view.angles, xform, ROT_MATRIX_ORDER_ZYX);

	/* DEBUG: adjust view pyramid inward */
	adj = 2.0 * (M_PI / 180.0);

	/* left plane */
	ang = (view.fov_x / 2.0) - adj;
	n[0] = cos (ang);
	n[1] = 0.0;
	n[2] = -sin (ang);
	for (i = 0; i < 3; i++) /* rotate with view angles */
		view.planes[VPLANE_LEFT].normal[i] = Vec_Dot (xform[i], n);
	view.planes[VPLANE_LEFT].dist = Vec_Dot (view.planes[VPLANE_LEFT].normal, view.pos);

	/* right plane */
	ang = M_PI - ((view.fov_x / 2.0) - adj);
	n[0] = cos (ang);
	n[1] = 0.0;
	n[2] = -sin (ang);
	for (i = 0; i < 3; i++) /* rotate with view angles */
		view.planes[VPLANE_RIGHT].normal[i] = Vec_Dot (xform[i], n);
	view.planes[VPLANE_RIGHT].dist = Vec_Dot (view.planes[VPLANE_RIGHT].normal, view.pos);

	/* bottom plane */
	ang = (M_PI / 2.0) - ((view.fov_y / 2.0) - adj);
	n[0] = 0.0;
	n[1] = sin (ang);
	n[2] = -cos (ang);
	for (i = 0; i < 3; i++) /* rotate with view angles */
		view.planes[VPLANE_BOTTOM].normal[i] = Vec_Dot (xform[i], n);
	view.planes[VPLANE_BOTTOM].dist = Vec_Dot (view.planes[VPLANE_BOTTOM].normal, view.pos);

	/* top plane */
	ang = -(M_PI / 2.0) + ((view.fov_y / 2.0) - adj);
	n[0] = 0.0;
	n[1] = sin (ang);
	n[2] = -cos (ang);
	for (i = 0; i < 3; i++) /* rotate with view angles */
		view.planes[VPLANE_TOP].normal[i] = Vec_Dot (xform[i], n);
	view.planes[VPLANE_TOP].dist = Vec_Dot (view.planes[VPLANE_TOP].normal, view.pos);
}


void
R_DrawGeometry (void)
{
	int i;

	SetupFrustum ();

	for (i = 0; i < g_numsurfs; i++)
	{
		if (1)
			DrawFlatSurf (&g_surfs[i]);
		if (0)
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
