#ifndef SE_CRYPT_H
#define SE_CRYPT_H

#ifdef	__cplusplus
extern "C" {
#endif

void SeEnCrypt(const unsigned char *input, unsigned char *output, int length, const unsigned char *key, int keylen);

void SeDeCrypt(const unsigned char *input, unsigned char *output, int length, const unsigned char *key, int keylen);

#ifdef	__cplusplus
}
#endif

#endif
