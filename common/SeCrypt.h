#ifndef SE_CRYPT_H
#define SE_CRYPT_H

void SeEnCrypt(const unsigned char *input, unsigned char *output, int length, const unsigned char *key, int keylen);

void SeDeCrypt(const unsigned char *input, unsigned char *output, int length, const unsigned char *key, int keylen);

#endif
