#ifndef __AES_H__
#define __AES_H__

#define AES_CRYPT	0x1  //0:en  de:1
#define AES_MODE	0x2   //0:cpu   1:dma
#define AES_CBC		0x4

#define AES_BY_RKEY (0x1 << 4)
#define AES_BY_CKEY (0x1 << 5)
#define AES_BY_UKEY (0x1 << 6)

#define AES_LEN		(3<<12) //0:128bit, 1:192bit, 2:256bit
#define AES_128BIT	0
#define AES_192BIT	1
#define AES_256BIT	2

#define AES_KEYDONE 0x1
#define AES_DONE 0x2
#define AES_DMADONE 0x4

#define AES_ONETIME_MAX	512	/* byte */

#define ASCR_EN_SFT_BIT (0)
#define ASCR_INITIV_SFT_BIT (1)
#define ASCR_KEYS_SFT_BIT (2)
#define ASCR_AESS_SFT_BIT (3)
#define ASCR_DECE_SFT_BIT (4)
#define ASCR_MODE_SFT_BIT (5)
#define ASCR_KEYL_SFT_BIT (6)
#define ASCR_DMAE_SFT_BIT (8)
#define ASCR_DMAS_SFT_BIT (9)
#define ASCR_CLR_SFT_BIT (10)
#define AES_REG_ASCR 0x13430000

#define AES_REG_ASSR 0x13430004
#define AES_REG_ASINTM 0x13430008
#define AES_REG_ASSA 0x1343000C
#define AES_REG_ASDA 0x13430010
#define AES_REG_ASTC 0x13430014
#define AES_REG_ASDI 0x13430018
#define AES_REG_ASDO 0x1343001C
#define AES_REG_ASKY 0x13430020
#define AES_REG_ASIV 0x13430024
int aes(void *bininput,void *binoutput,unsigned int len,unsigned int key,unsigned int crypt);
int do_aes(void *binaddr,int len,int aeskey,int aescrypt);
void do_aes_dma(void *in_paaddr, void *out_paaddr, int len, int aeskey, int aescrypt);

#endif
