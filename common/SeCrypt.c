/**********************************************************************
 *
 * SeCrypt.h
 * rc4
 * NOTE: This is a C version RC4 operation wrapper
 * 
 *
 **********************************************************************/

#include "SeCrypt.h"

typedef struct
{
	int x;						/*!< permutation index */
	int y;						/*!< permutation index */
	unsigned char m[256];		/*!< permutation table */
}arc4_context;

void arc4_setup(arc4_context *ctx, const unsigned char *key, int keylen)
{
	int i, j, k, a;
	unsigned char *m;

	ctx->x = 0;
	ctx->y = 0;
	m = ctx->m;

	for(i = 0; i < 256; i++)
	{
		m[i] = (unsigned char) i;
	}

	j = k = 0;

	for(i = 0; i < 256; i++, k++)
	{
		if(k >= keylen)
		{
			k = 0;
		}

		a = m[i];
		j = (j + a + key[k]) & 0xFF;
		m[i] = m[j];
		m[j] = (unsigned char)a;
	}
}

void arc4_crypt(arc4_context *ctx, int length, const unsigned char *input, unsigned char *output)
{
	int i, x, y, a, b;
	unsigned char *m;

	x = ctx->x;
	y = ctx->y;
	m = ctx->m;

	for(i = 0; i < length; i++)
	{
		x = ( x + 1 ) & 0xFF; a = m[x];
		y = ( y + a ) & 0xFF; b = m[y];

		m[x] = (unsigned char) b;
		m[y] = (unsigned char) a;

		output[i] = (unsigned char)(input[i] ^ m[(unsigned char)( a + b )]);
	}

	ctx->x = x;
	ctx->y = y;
}

void SeCrypt(const unsigned char *input, unsigned char *output, int length, const unsigned char *key, int keylen)
{
	arc4_context ctx;
	arc4_setup(&ctx, key, keylen);
	arc4_crypt(&ctx, length, input, output);
}

// º”√‹
void SeEnCrypt(const unsigned char *input, unsigned char *output, int length, const unsigned char *key, int keylen)
{
	SeCrypt(input, output, length, key, keylen);
}

// Ω‚√‹
void SeDeCrypt(const unsigned char *input, unsigned char *output, int length, const unsigned char *key, int keylen)
{
	SeCrypt(input, output, length, key, keylen);
}
