#ifndef __SE_BASE64_H__
#define __SE_BASE64_H__

#ifdef	__cplusplus
extern "C" {
#endif

#define BASE64_ENCODE_OUT_SIZE(s) ((unsigned int)((((s) + 2) / 3) * 4 + 1))
#define BASE64_DECODE_OUT_SIZE(s) ((unsigned int)((((s) + 3) / 4) * 3) + 1)

/*
 * out is double in len
 * out is null-terminated encode string.
 * return values is out length, exclusive terminating `\0'
 */
unsigned int SeBase64Encode(const unsigned char *in, unsigned int inlen, char *out);

/*
 * out is double in len
 * return values is out length
 */
unsigned int SeBase64Decode(const char *in, unsigned int inlen, unsigned char *out);

#ifdef	__cplusplus
}
#endif

#endif
