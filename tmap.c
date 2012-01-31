#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#include "cdefs.h"
#include "vec.h"
#include "render.h"

static bool quit = false;
static float moves[3];
static bool dragging = false;
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
			case SDL_MOUSEBUTTONDOWN:
				dragging = true;
				break;

			case SDL_MOUSEBUTTONUP:
				dragging = false;
				break;

			case SDL_KEYDOWN:
				if (sdlev.key.keysym.sym == SDLK_UP)
					moves[2]++;
				else if (sdlev.key.keysym.sym == SDLK_DOWN)
					moves[2]--;
				else if (sdlev.key.keysym.sym == SDLK_LEFT)
					moves[0]--;
				else if (sdlev.key.keysym.sym == SDLK_RIGHT)
					moves[0]++;
				break;

			case SDL_KEYUP:
				if (sdlev.key.keysym.sym == SDLK_ESCAPE)
					quit = true;
				else if (sdlev.key.keysym.sym == SDLK_UP)
					moves[2]--;
				else if (sdlev.key.keysym.sym == SDLK_DOWN)
					moves[2]++;
				else if (sdlev.key.keysym.sym == SDLK_LEFT)
					moves[0]++;
				else if (sdlev.key.keysym.sym == SDLK_RIGHT)
					moves[0]--;
				else if (sdlev.key.keysym.sym == 'v')
				{
					printf("right: (%g, %g, %g)\n",
						view.right[0],
						view.right[1],
						view.right[2]);
					printf("up:    (%g, %g, %g)\n",
						view.up[0],
						view.up[1],
						view.up[2]);
					printf("fwd  : (%g, %g, %g)\n",
						view.forward[0],
						view.forward[1],
						view.forward[2]);
				}
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

	if (dragging)
	{
		float rads;

		rads = view.fov_x * (mouse[0] / (float)r_w);
		view.angles[YAW] += rads;

		rads = view.fov_y * (mouse[1] / (float)r_h);
		view.angles[PITCH] += rads;
	}

	{
		float v[3];
		Vec_Copy (view.forward, v);
		Vec_Scale (v, moves[2] * SPEED * frametime);
		Vec_Add (view.pos, v, view.pos);

		Vec_Copy (view.right, v);
		Vec_Scale (v, moves[0] * SPEED * frametime);
		Vec_Add (view.pos, v, view.pos);
	}

	Vec_AnglesVectors (view.angles, view.right, view.up, view.forward);
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
