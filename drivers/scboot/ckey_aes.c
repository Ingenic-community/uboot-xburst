#include <common.h>
#include <asm/io.h>
#include <asm/arch/cpm.h>
#include <asm/reboot.h>
#include <asm/spl.h>
#include "secall.h"
#include "pdma.h"
#include "otp.h"
#include "aes.h"

/*
 * security boot.
 * 1. prepare a bin encrypted.
 *		|---------------|-------------------|					|
 *		| SC KEY(2048)	|  CODE encrypted   |
 *		|---------------|-------------------|					|
 * 2.
 *
 * */

#define SC_MAX_SIZE_PERTIME		(2048)
#define SC_MAGIC_SIZE			(512)
#define SC_KEY_SIZE				(1536)

#define SECURE_SCBOOT_MAGIC		0x54424353

#define SPL_KENOFFSET    ((SC_MAGIC_SIZE + 16) / 4)
#define SPL_CENOFFSET    ((SC_MAGIC_SIZE + 4) / 4)
#define SPL_NLENOFFSET   ((SC_MAGIC_SIZE + 8) / 4)
#define SPL_ULENOFFSET   ((SC_MAGIC_SIZE + 12) / 4)
#define KN_OFFSET        (256)
#define KU_OFFSET        (KN_OFFSET + 256)
#define SCKEY_INFO_LEN   (256)

#define UBOOT_KENOFFSET  ((CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE + 16) / 4)
#define UBOOT_CENOFFSET  ((CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE + 4) / 4)
#define UBOOT_NLENOFFSET ((CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE + 8) / 4)
#define UBOOT_ULENOFFSET ((CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE + 12) / 4)
#define UBOOT_LENOFFSET  ((CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE) / 4)

#define IMAGE_START       0x0
#define SPL_SCKEY_START   0x00000200
#define UBOOT_SCKEY_START 0x00004200

#define CRC_POSITION        9		/* 9th bytes */
#define SPL_LENGTH_POSITION 12	/* 11th */

#define USERKEY_ENCRYPT  2
#define CHIPKEY_ENCRYPT  1

extern void flush_cache_all(void);
extern void read_flash(unsigned int from, unsigned int len, unsigned char *buf);
extern void write_flash(unsigned int from, unsigned int len, unsigned char *buf);
extern inline phys_addr_t virt_to_phys(volatile void * address);

void ckey_aes(void)
{
	char *read_buf = NULL;
	int n_paaddr, ku_paaddr, code_paaddr;
	read_buf = (char *)malloc(SCKEY_INFO_LEN);
	memset(read_buf, 0xff , SCKEY_INFO_LEN);
	read_flash(SPL_SCKEY_START, SCKEY_INFO_LEN, read_buf);

	int spl_kencrypt = *((int *)read_buf + 4);
	int spl_cencrypt = *((int *)read_buf + 1);
	int spl_len = *((int *)read_buf);
	int spl_nlen = *((int *)read_buf + 2);
	int spl_ulen = *((int *)read_buf + 3);

	memset(read_buf, 0xff , SCKEY_INFO_LEN);

	read_flash(UBOOT_SCKEY_START, SCKEY_INFO_LEN, read_buf);

	int uboot_kencrypt = *((int *)read_buf + 4);
	int uboot_cencrypt = *((int *)read_buf + 1);
	int uboot_len = *((int *)read_buf);
	int uboot_nlen = *((int *)read_buf + 2);
	int uboot_ulen = *((int *)read_buf + 3);

	int image_len = uboot_len + CONFIG_UBOOT_OFFSET + SC_MAX_SIZE_PERTIME;

	if (((spl_kencrypt == -1) || (spl_cencrypt == -1) ||
		(uboot_kencrypt == -1) || (uboot_cencrypt == -1))) {
		printf("memory read failed!!\n");
		hang();
	}

	if (!((spl_kencrypt == USERKEY_ENCRYPT) || (spl_cencrypt == USERKEY_ENCRYPT) ||
		(uboot_kencrypt == USERKEY_ENCRYPT) || (uboot_cencrypt == USERKEY_ENCRYPT)))
		return 0;

	free(read_buf);
	*read_buf = NULL;

	read_buf = (char *)malloc(image_len);
	memset(read_buf, 0xff, image_len);

	read_flash(IMAGE_START, image_len, read_buf);
	flush_cache_all();

	if (spl_kencrypt == USERKEY_ENCRYPT) {
		spl_nlen = (spl_nlen / 8 + 15) & 0xFFFFFFF0;
		spl_ulen = (spl_ulen / 8 + 15) & 0xFFFFFFF0;

		n_paaddr = virt_to_phys(read_buf + SC_MAGIC_SIZE + KN_OFFSET);
		ku_paaddr = virt_to_phys(read_buf + SC_MAGIC_SIZE + KU_OFFSET);

		do_aes_dma((int *)n_paaddr, (int *)n_paaddr, spl_nlen, AES_BY_UKEY, 1);
		do_aes_dma((int *)ku_paaddr, (int *)ku_paaddr, spl_ulen, AES_BY_UKEY, 1);
		*((int *)read_buf + SPL_KENOFFSET) = CHIPKEY_ENCRYPT;
	}
	if (spl_cencrypt == USERKEY_ENCRYPT) {
		code_paaddr = virt_to_phys(read_buf + SC_MAX_SIZE_PERTIME);
		do_aes_dma((int *)code_paaddr, (int *)code_paaddr, spl_len, AES_BY_UKEY, 1);
		*((int *)read_buf + SPL_CENOFFSET) = CHIPKEY_ENCRYPT;

		u8 crc = sec_crc(read_buf + SC_MAX_SIZE_PERTIME, spl_len);
		memcpy(read_buf + CRC_POSITION, &crc, 1);
	}

	if (uboot_kencrypt == USERKEY_ENCRYPT) {
		uboot_nlen = (uboot_nlen / 8 + 15) & 0xFFFFFFF0;
		uboot_ulen = (uboot_ulen / 8 + 15) & 0xFFFFFFF0;

		n_paaddr = virt_to_phys(read_buf + CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE + KN_OFFSET);
		ku_paaddr = virt_to_phys(read_buf + CONFIG_UBOOT_OFFSET + SC_MAGIC_SIZE + KU_OFFSET);

		do_aes_dma((int *)n_paaddr, (int *)n_paaddr, uboot_nlen, AES_BY_UKEY, 1);
		do_aes_dma((int *)ku_paaddr, (int *)ku_paaddr, uboot_ulen, AES_BY_UKEY, 1);
		*((int *)read_buf + UBOOT_KENOFFSET) = CHIPKEY_ENCRYPT;
	}

	if (uboot_cencrypt == USERKEY_ENCRYPT) {
		int ulen = (uboot_len + 15) & 0xFFFFFFF0;

		code_paaddr = virt_to_phys(read_buf + CONFIG_UBOOT_OFFSET + SC_MAX_SIZE_PERTIME);
		do_aes_dma((int *)code_paaddr, (int *)code_paaddr, ulen, AES_BY_UKEY, 1);

		*((int *)read_buf + UBOOT_CENOFFSET) = CHIPKEY_ENCRYPT;
	}

	write_flash(IMAGE_START, image_len, read_buf);

	memset(read_buf, 0xff , SCKEY_INFO_LEN);

	read_flash(SPL_SCKEY_START, SCKEY_INFO_LEN, read_buf);
	flush_cache_all();

	spl_kencrypt = *((int *)read_buf + 4);
	spl_cencrypt = *((int *)read_buf + 1);

	free(read_buf);
	read_buf = NULL;

	if ((spl_cencrypt == 1) && (spl_kencrypt == 1)) {
		printf("Spl chipkey aes success\n");
		_machine_restart();
	}else {
		printf("## ERROR ## ckey aes failed!! ##\n");
		hang();
	}


}
