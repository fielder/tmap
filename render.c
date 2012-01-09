#include <stdlib.h>

#include <SDL.h>

#include "cdefs.h"
#include "render.h"

static SDL_Surface *sdl_surf = NULL;

int r_w = 512;
int r_h = 384;
uint8_t *r_buf = NULL;


void
R_Init (void)
{
	SDL_InitSubSystem (SDL_INIT_VIDEO);

	sdl_surf = SDL_SetVideoMode (r_w, r_h, 8,
				SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_HWPALETTE);
	r_buf = malloc (r_w * r_h);

	R_SetupGeometry ();
}


void
R_Shutdown (void)
{
	free (r_buf);
	r_buf = NULL;

	SDL_FreeSurface (sdl_surf);
	sdl_surf = NULL;
}


void
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


void
R_Line (int x1, int y1, int x2, int y2, int c)
{
	int x, y;
	int dx, dy;
	int sx, sy;
	int ax, ay;
	int d;

	if (0)
	{
		if (	x1 < 0 || x1 >= r_w ||
			x2 < 0 || x2 >= r_w ||
			y1 < 0 || y1 >= r_h ||
			y2 < 0 || y2 >= r_h )
		{
			return;
		}
	}

	dx = x2 - x1;
	ax = 2 * (dx < 0 ? -dx : dx);
	sx = dx < 0 ? -1 : 1;

	dy = y2 - y1;
	ay = 2 * (dy < 0 ? -dy : dy);
	sy = dy < 0 ? -1 : 1;

	x = x1;
	y = y1;

	if (ax > ay)
	{
		d = ay - ax / 2;
		while (1)
		{
			r_buf[y * r_w + x] = c;
			if (x == x2)
				break;
			if (d >= 0)
			{
				y += sy;
				d -= ax;
			}
			x += sx;
			d += ay;
		}
	}
	else
	{
		d = ax - ay / 2;
		while (1)
		{
			r_buf[y * r_w + x] = c;
			if (y == y2)
				break;
			if (d >= 0)
			{
				x += sx;
				d -= ay;
			}
			y += sy;
			d += ax;
		}
	}
}


static void
DrawPal (void)
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
		DrawPal ();

	R_DrawGeometry ();

	Vid_Swap ();
}
