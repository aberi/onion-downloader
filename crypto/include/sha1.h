#ifndef _SHA_1_
#define _SHA_1_

#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>

/* Initial 32-bit words */
#define A 0x67452301
#define B 0xEFCDAB89
#define C 0x98BADCFE
#define D 0x10325476
#define E 0xC3D2E1F0

/* 20-round keys */
#define K1 0x5A827999
#define K2 0x6ED9EBA1
#define K3 0x8F1BBCDC
#define K4 0xCA62C1D6

struct sha1_block
{
	uint32_t h[5];
};

typedef struct sha1_block Hash;

uint32_t f1 (uint32_t a, uint32_t b, uint32_t c);
uint32_t f2 (uint32_t a, uint32_t b, uint32_t c);
uint32_t f3 (uint32_t a, uint32_t b, uint32_t c);

typedef uint32_t (*F) (uint32_t, uint32_t, uint32_t);

Hash *Round (uint32_t a, uint32_t *b, uint32_t c, uint32_t d, uint32_t *e, F f, uint32_t key, uint32_t word);
Hash *Transform_Block (Hash *in); /* Modify the input parameter according to the algorithm and return
									 the modified version */
char *pad(char *data, uint32_t len);

#endif
