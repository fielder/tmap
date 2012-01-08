#ifndef __CDEFS_H__
#define __CDEFS_H__

#include <stdint.h>

typedef enum { false, true } bool;

#ifndef NULL
#define NULL ((void *)0)
#endif

#define ARRAY_COUNT(__ARR) (sizeof(__ARR) / sizeof((__ARR)[0]))
#define MEMBER_SIZE(__T, __M) (sizeof(((__T *)0)->__M))

#endif /* __CDEFS_H__ */
