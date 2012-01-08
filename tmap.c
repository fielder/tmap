#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#include "cdefs.h"
#include "render.h"

static bool quit = false;


void
RunInput (void)
{
	SDL_Event sdlev;

	while (SDL_PollEvent(&sdlev))
	{
		switch (sdlev.type)
		{
			case SDL_KEYDOWN:
				break;

			case SDL_KEYUP:
				if (sdlev.key.keysym.sym == 27)
					quit = true;
				break;

			case SDL_QUIT:
				quit = true;
				break;

			default:
				break;
		}
	}
}


int
main (int argc, const char *argv[])
{
	unsigned int last, duration, now;

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

		RunInput ();

		R_Refresh ();
	}

	R_Shutdown ();

	SDL_Quit ();

	return EXIT_SUCCESS;
}
