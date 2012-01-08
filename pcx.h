#ifndef __PCX_H__
#define __PCX_H__

#include "cdefs.h"

extern uint8_t *
ReadPCX (	const uint8_t *data,
		int datalen,
		int *width,
		int *height,
		uint8_t palette[768] );

extern uint8_t *
LoadPCX (	const char *path,
		int *width,
		int *height,
		uint8_t palette[768] );

#endif /* __PCX_H__ */
