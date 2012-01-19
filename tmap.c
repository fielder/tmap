#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#include "cdefs.h"
#include "vec.h"
#include "render.h"

static bool quit = false;
static int movedir[3];
static bool space = false;
#define SPEED 32


void
RunInput (float frametime)
{
	SDL_Event sdlev;
	int mouse[2];

	mouse[0] = mouse[1] = 0;

	while (SDL_PollEvent(&sdlev))
	{
		switch (sdlev.type)
		{
			case SDL_KEYDOWN:
				if (sdlev.key.keysym.sym == SDLK_UP)
					movedir[2]++;
				else if (sdlev.key.keysym.sym == SDLK_DOWN)
					movedir[2]--;
				else if (sdlev.key.keysym.sym == SDLK_LEFT)
					movedir[0]--;
				else if (sdlev.key.keysym.sym == SDLK_RIGHT)
					movedir[0]++;
				else if (sdlev.key.keysym.sym == SDLK_SPACE)
					space = true;
				break;

			case SDL_KEYUP:
				if (sdlev.key.keysym.sym == SDLK_ESCAPE)
					quit = true;
				else if (sdlev.key.keysym.sym == SDLK_UP)
					movedir[2]--;
				else if (sdlev.key.keysym.sym == SDLK_DOWN)
					movedir[2]++;
				else if (sdlev.key.keysym.sym == SDLK_LEFT)
					movedir[0]++;
				else if (sdlev.key.keysym.sym == SDLK_RIGHT)
					movedir[0]--;
				else if (sdlev.key.keysym.sym == SDLK_SPACE)
					space = false;
				break;

			case SDL_MOUSEMOTION:
				mouse[0] = sdlev.motion.xrel;
				mouse[1] = sdlev.motion.yrel;
				break;

			case SDL_QUIT:
				quit = true;
				break;

			default:
				break;
		}
	}

	if (space)
	{
		float rads;

		rads = view.fov_x * (mouse[0] / (float)r_w);
		view.angles[YAW] += rads;

		rads = view.fov_y * (mouse[1] / (float)r_h);
		view.angles[PITCH] += rads;
	}

	{
		float v[3];
		Vec_Clear (v);
		v[0] = movedir[0];
		v[1] = movedir[1];
		v[2] = movedir[2];
		Vec_Scale (v, SPEED * frametime);
		Vec_Add (view.pos, v, view.pos);
	}
}


int
main (int argc, const char *argv[])
{
	unsigned int last, duration, now;
	float frametime;

	if (SDL_Init(SDL_INIT_TIMER) != 0)
	{
		fprintf (stderr, "Error: failed initializing SDL\n");
		return EXIT_FAILURE;
	}

	R_Init ();

	last = SDL_GetTicks ();
	while (!quit)
	{
		do
		{
			now = SDL_GetTicks ();
			duration = now - last;
		} while (duration < 1);
		last = now;
		frametime = duration / 1000.0;

		RunInput (frametime);

		R_Refresh ();
	}

	R_Shutdown ();

	SDL_Quit ();

	return EXIT_SUCCESS;
}
