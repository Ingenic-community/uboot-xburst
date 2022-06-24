#ifndef __TEST_NKU_H__
#define __TEST_NKU_H__

#define RSA_BIT	(992)
#define RSA_BYTE_LEN	(RSA_BIT/8)
#define RSA_WORD_LEN	(RSA_BYTE_LEN/4)

#define MD5_BIT	(128)
#define MD5_BYTE_LEN	(MD5_BIT/8)
#define MD5_WORD_LEN	(MD5_BYTE_LEN/4)

extern unsigned int rsakr[RSA_WORD_LEN];

extern unsigned int rsaku[RSA_WORD_LEN];

extern unsigned int rsakuckenc[RSA_WORD_LEN];

extern unsigned int rsakuukenc[RSA_WORD_LEN];

extern unsigned int rsan[RSA_WORD_LEN];

extern unsigned int rsanckenc[RSA_WORD_LEN];

extern unsigned int rsanukenc[RSA_WORD_LEN];

extern unsigned int nku1sig[MD5_WORD_LEN];

extern unsigned int nku1sigrkenc[MD5_WORD_LEN];

extern unsigned int nku1[RSA_WORD_LEN*2];



extern unsigned int rsan128[4];
extern unsigned int rsakr128[4];
extern unsigned int rsaku128[4];




#endif
