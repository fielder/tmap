#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <SDL.h>

#include "cdefs.h"
#include "vec.h"
#include "render.h"

static bool quit = false;
static float moves[3];
static bool dragging = false;
static float speed_mult = 1.0;

#define SPEED 32.0

#if 1
	#define KEY_FORWARD	'.'
	#define KEY_BACK	'e'
	#define KEY_LEFT	'o'
	#define KEY_RIGHT	'u'
#else
	#define KEY_FORWARD	SDLK_UP
	#define KEY_BACK	SDLK_DOWN
	#define KEY_LEFT	SDLK_LEFT
	#define KEY_RIGHT	SDLK_RIGHT
#endif

void
RunInput (float frametime)
{
	SDL_Event sdlev;
	int mouse_delt[2];
	float v[3];
	int i;

	mouse_delt[0] = mouse_delt[1] = 0;

	while (SDL_PollEvent(&sdlev))
	{
		switch (sdlev.type)
		{
			case SDL_MOUSEBUTTONDOWN:
				if (sdlev.button.button == 1)
					dragging = true;
				else if (sdlev.button.button == 3)
					moves[1]++;
				break;

			case SDL_MOUSEBUTTONUP:
				if (sdlev.button.button == 1)
					dragging = false;
				else if (sdlev.button.button == 3)
					moves[1]--;
				break;

			case SDL_KEYDOWN:
				if (sdlev.key.keysym.sym == KEY_FORWARD)
					moves[2]++;
				else if (sdlev.key.keysym.sym == KEY_BACK)
					moves[2]--;
				else if (sdlev.key.keysym.sym == KEY_LEFT)
					moves[0]--;
				else if (sdlev.key.keysym.sym == KEY_RIGHT)
					moves[0]++;
				else if (sdlev.key.keysym.sym == SDLK_LSHIFT)
					speed_mult = 2.0;
				break;

			case SDL_KEYUP:
				if (sdlev.key.keysym.sym == SDLK_ESCAPE)
					quit = true;
				else if (sdlev.key.keysym.sym == KEY_FORWARD)
					moves[2]--;
				else if (sdlev.key.keysym.sym == KEY_BACK)
					moves[2]++;
				else if (sdlev.key.keysym.sym == KEY_LEFT)
					moves[0]++;
				else if (sdlev.key.keysym.sym == KEY_RIGHT)
					moves[0]--;
				else if (sdlev.key.keysym.sym == SDLK_LSHIFT)
					speed_mult = 1.0;
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
				else if (sdlev.key.keysym.sym == 'p')
				{
					printf (" %g, %g, %g\n",
						view.pos[0],
						view.pos[1],
						view.pos[2]);
				}
				else if (sdlev.key.keysym.sym == 'a')
				{
					printf (" %g, %g, %g\n",
						view.angles[0],
						view.angles[1],
						view.angles[2]);
				}
				break;

			case SDL_MOUSEMOTION:
				mouse_delt[0] = sdlev.motion.xrel;
				mouse_delt[1] = sdlev.motion.yrel;
				break;

			case SDL_QUIT:
				quit = true;
				break;

			default:
				break;
		}
	}

	/* rotate camera with mouse */
	if (dragging)
	{
		float rads;

		rads = view.fov_x * (mouse_delt[0] / (float)r_w);
		view.angles[YAW] += rads;

		rads = view.fov_y * (mouse_delt[1] / (float)r_h);
		view.angles[PITCH] += rads;
	}

	/* restrict camera angles */
	if (view.angles[PITCH] > M_PI / 2.0)
		view.angles[PITCH] = M_PI / 2.0;
	if (view.angles[PITCH] < -M_PI / 2.0)
		view.angles[PITCH] = -M_PI / 2.0;

	/* move camera */
	Vec_Clear (v);

	Vec_Copy (view.right, v);
	Vec_Scale (v, moves[0] * SPEED * speed_mult * frametime);
	Vec_Add (view.pos, v, view.pos);

	Vec_Copy (view.up, v);
	Vec_Scale (v, moves[1] * SPEED * speed_mult * frametime);
	Vec_Add (view.pos, v, view.pos);

	Vec_Copy (view.forward, v);
	Vec_Scale (v, moves[2] * SPEED * speed_mult * frametime);
	Vec_Add (view.pos, v, view.pos);

	/* make view vectors from camera angles */
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
