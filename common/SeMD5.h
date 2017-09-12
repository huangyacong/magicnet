#ifndef __SE_MD5_H__
#define __SE_MD5_H__

#ifdef	__cplusplus
extern "C" {
#endif

// pcOut len == 33, include '\0'
void SeMD5(char *pcOut, const char* pcBuffer, unsigned int uiLen);

#ifdef	__cplusplus
}
#endif

#endif
