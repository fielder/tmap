#include <SDL.h>

#include "cdefs.h"
#include "pcx.h"
#include "render.h"

static void
R_SetupGeometry (void);
static void
R_DrawGeometry (void);

static int r_w = 512;
static int r_h = 384;

static SDL_Surface *sdl_surf = NULL;
static uint8_t *r_buf = NULL;


void
R_Init (void)
{
	R_SetupGeometry ();

	SDL_InitSubSystem (SDL_INIT_VIDEO);

	sdl_surf = SDL_SetVideoMode (r_w, r_h, 8,
				SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_HWPALETTE);
	r_buf = malloc (r_w * r_h);
}


void
R_Shutdown (void)
{
	free (r_buf);
	r_buf = NULL;

	SDL_FreeSurface (sdl_surf);
	sdl_surf = NULL;
}


static void
Vid_SetPalette (const uint8_t palette[768])
{
	int i;
	SDL_Color sdlpal[256];

	for (i = 0; i < 256; i++)
	{
		sdlpal[i].r = palette[0];
		sdlpal[i].g = palette[1];
		sdlpal[i].b = palette[2];
		sdlpal[i].unused = 0;
		palette += 3;
	}

	SDL_SetColors (sdl_surf, sdlpal, 0, 256);
}


static bool vid_locked = false;

static bool
Vid_Lock (void)
{
	if (SDL_MUSTLOCK(sdl_surf) && !vid_locked)
	{
		if (SDL_LockSurface(sdl_surf) != 0)
			return false;
		vid_locked = true;
	}

	return true;
}


static void
Vid_Unlock (void)
{
	if (vid_locked)
	{
		SDL_UnlockSurface (sdl_surf);
		vid_locked = false;
	}
}


static void
Vid_Swap (void)
{
	if (!Vid_Lock())
		return;

	if (sdl_surf->pitch == sdl_surf->w)
	{
		memcpy (sdl_surf->pixels, r_buf, sdl_surf->w * sdl_surf->h);
	}
	else
	{
		int y;
		uint8_t *dest = sdl_surf->pixels;
		const uint8_t *src = r_buf;
		for (y = 0; y < sdl_surf->h; y++)
		{
			memcpy (dest, src, sdl_surf->w);
			dest += sdl_surf->pitch;
			src += r_w;
		}
	}

	Vid_Unlock ();

	SDL_Flip (sdl_surf);
}


static void
R_DrawPal (void)
{
	int x, y;
	uint8_t *dest = r_buf;
	for (y = 0; y < 128 && y < r_h; y++)
	{
		for (x = 0; x < 128 && x < r_w; x++)
			dest[x] = ((y << 1) & 0xf0) + (x >> 3);
		dest += r_w;
	}
}


void
R_Refresh (void)
{
	if (0)
		R_DrawPal ();

	R_DrawGeometry ();

	Vid_Swap ();
}

/* ================================================================== */

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
	struct edge_s *e;
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

	AddEdge (0, 1);
	AddEdge (1, 2);
	AddEdge (2, 3);
	AddEdge (3, 0);
	AddEdge (0, 4);
	AddEdge (1, 5);
	AddEdge (2, 6);
	AddEdge (3, 7);
	AddEdge (4, 5);
	AddEdge (5, 6);
	AddEdge (6, 7);
	AddEdge (7, 4);

	AddQuad (&tex_front, 3, 2, 6, 7);
	AddQuad (&tex_front, 1, 0, 4, 5);
	AddQuad (&tex_side, 0, 3, 7, 4);
	AddQuad (&tex_side, 2, 1, 5, 6);
	AddQuad (&tex_floor, 0, 1, 2, 3);
	AddQuad (&tex_floor, 7, 6, 5, 4);
}


static void
R_DrawGeometry (void)
{
}
