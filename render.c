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
