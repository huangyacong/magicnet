#ifndef __SE_SHA1_H__
#define __SE_SHA1_H__

#ifdef	__cplusplus
extern "C" {
#endif

// pcOut len == 41, include '\0'
void SeSHA1(const char* pcText, size_t sz, char* pcOut);

// pcOut len == 41, include '\0'
void SeMacSHA1(const char* pcKey, size_t key_sz, const char* pcText, size_t text_sz, char* pcOut);

#ifdef	__cplusplus
}
#endif

#endif
