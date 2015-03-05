#ifndef SE_CRYPT_H
#define SE_CRYPT_H

// º”√‹
void SeEnCrypt(const unsigned char *input, unsigned char *output, int length, const unsigned char *key, int keylen);

// Ω‚√‹
void SeDeCrypt(const unsigned char *input, unsigned char *output, int length, const unsigned char *key, int keylen);

#endif
