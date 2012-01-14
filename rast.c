#include "cdefs.h"
#include "vec.h"
#include "geom.h"
#include "render.h"


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


/*
static struct outvert_s p_outverts[32];
static int num_outverts;

static struct outedge_s p_outedges[32];
static int num_outedges;
static struct outedge_s edges_l, edges_r;

static float p_min_v, p_max_v;
static int p_topi;
*/


static void
GenerateSpans (void)
{
}


//FIXME: We're going to have to snap u/v to integers eventually
//	Will require adjusting non-integer values to match the shift
static void
ScanEdges (void)
{
#if 0
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
#endif
}


static void
DrawSurf (struct msurf_s *s)
{
#if 0
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
#endif

	ScanEdges ();
	GenerateSpans ();
}


void
R_DrawGeometry (void)
{
	int i;

	for (i = 0; i < g_numsurfs; i++)
		DrawSurf (g_surfs + i);
}
