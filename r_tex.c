#include <stdlib.h>
#include <string.h>

#include "cdefs.h"
#include "pcx.h"
#include "r_tex.h"

static struct tex_s *textures = NULL;


struct tex_s *
R_LoadTex (const char *path)
{
	struct tex_s *t;

	for (t = textures; t != NULL; t = t->next)
	{
		if (strcmp(t->path, path) == 0)
			return t;
	}

	t = malloc (sizeof(*t));
	memset (t, 0, sizeof(*t));

	t->next = textures;
	textures = t;

	t->path = path;
	t->pixels = LoadPCX (t->path, &t->w, &t->h, NULL);

	return t;
}
