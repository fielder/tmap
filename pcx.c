#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "cdefs.h"
#include "pcx.h"


struct pcxheader_s
{
	uint8_t		manufacturer;
	uint8_t		version;
	uint8_t		encoding;
	uint8_t		bpp;
	uint16_t	minx, miny;
	uint16_t	maxx, maxy;
	uint16_t	hres, vres;
	uint8_t		ega_palette[48];
	uint8_t		reserved;
	uint8_t		num_layers;
	uint16_t	bytes_per_line;
	uint16_t	pal_type;
	uint8_t		unused[58];
};


static short
LittleShort (short s)
{
	static int endianess = -1;

	if (endianess == -1)
	{
		union
		{
			int i;
			short s;
			char c;
		} u;
		u.i = 0;
		u.c = 1;
		if (u.s == 1)
			endianess = 0;
		else
			endianess = 1;
	}

	if (endianess == 1)
	{
		int b1, b2;
		b1 = s & 0xff;
		b2 = (s >> 8) & 0xff;
		s = (b1 << 8) | b2;
	}

	return s;
}


uint8_t *
ReadPCX (	const uint8_t *data,
		int datalen,
		int *width,
		int *height,
		uint8_t palette[768] )
{
	struct pcxheader_s header;
	int w, h;
	uint8_t *pixels;
	uint8_t *dest;
	int x, y;
	int p, c;

	/* check minimum size */
	if (datalen < (int)sizeof(struct pcxheader_s) + 1 + 1 + 768)
		return NULL;

	memcpy (&header, data, sizeof(header));
	header.minx = LittleShort (header.minx);
	header.miny = LittleShort (header.miny);
	header.maxx = LittleShort (header.maxx);
	header.maxy = LittleShort (header.maxy);
	header.hres = LittleShort (header.hres);
	header.vres = LittleShort (header.vres);
	header.bytes_per_line = LittleShort (header.bytes_per_line);
	header.pal_type = LittleShort (header.pal_type);

	w = header.maxx - header.minx + 1;
	h = header.maxy - header.miny + 1;

	if (	header.manufacturer != 0x0a ||
		header.version != 5 ||
		header.encoding != 1 ||
		header.bpp != 8 ||
		w <= 0 || h <= 0 ||
		w > 2048 || h > 2048)
	{
		return NULL;
	}

	if (palette != NULL)
		memcpy (palette, data + datalen - 768, 768);

	if ((pixels = malloc(w * h)) == NULL)
		return NULL;

/* TODO: bounds checking */
	data += sizeof(header);
	dest = pixels;
	for (y = 0; y < h; y++)
	{
		x = 0;
		while (x < header.bytes_per_line)
		{
			p = *data++;
			if ((p & 0xc0) == 0xc0)
			{
				c = p & 0x3f;
				p = *data++;
			}
			else
				c = 1;
			while (c-- > 0)
			{
				if (x < w)
					*dest++ = p;
				x++;
			}
		}
	}

	if (*data != 0xc)
	{
		free (pixels);
		return NULL;
	}

	if (width != NULL)
		*width = w;
	if (height != NULL)
		*height = h;

	return pixels;
}


static void *
LoadFile (const char *path, int *size)
{
	void *ret = NULL;
	int fd;

	fd = open (path, O_RDONLY);
	if (fd != -1)
	{
		int sz = lseek (fd, 0, SEEK_END);
		if (sz != -1)
		{
			ret = malloc (sz + 1);
			if (ret != NULL)
			{
				lseek (fd, 0, SEEK_SET);
				if (read(fd, ret, sz) != sz)
				{
					free (ret);
					ret = NULL;
				}
				else
				{
					((char *)ret)[sz] = '\0';
					if (size != NULL)
						*size = sz;
				}
			}
		}
		close (fd);
		fd = -1;
	}

	return ret;
}


uint8_t *
LoadPCX (	const char *path,
		int *width,
		int *height,
		uint8_t palette[768] )
{
	uint8_t *filedata, *pixels;
	int filesize;

	if ((filedata = LoadFile (path, &filesize)) == NULL)
		return NULL;
	pixels = ReadPCX (filedata, filesize, width, height, palette);
	free (filedata);
	filedata = NULL;

	return pixels;
}
