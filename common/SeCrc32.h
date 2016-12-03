#ifndef __SE_CRC32_H__
#define	__SE_CRC32_H__

#ifdef	__cplusplus
extern "C" {
#endif

unsigned int SeCrc32(const unsigned char *buf, int len, unsigned int init /*= 100*/);

#ifdef	__cplusplus
}
#endif

#endif