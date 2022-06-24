#include <common.h>
#include <nand.h>
#include <linux/types.h>
#include <linux/mtd/mtd.h>
#include <cloner/cloner.h>
#include <errno.h>

#define FMW_SIZE_MAX	512

struct sn_config {

	uint32_t sn_len;
	uint32_t crc_val;
};


struct mac_config {
	uint32_t mac_len;
	uint32_t crc_val;
};

static int32_t firmware_buf_compare(uint8_t *wbuf, uint8_t *rbuf, uint32_t len) {

	int32_t i = 0;
	for(i = 0; i < len; i++) {
		if(wbuf[i] != rbuf[i]) {
			printf("compare err:wbuf = 0x%02x, rbuf= 0x%02x\n",
				wbuf[i], rbuf[i]);
			return -EIO;
		}
	}
	return 0;
}

static int32_t flash_write_blk(struct mtd_info *mtd, uint32_t off, uint32_t len, uint8_t *wbuf) {

        uint32_t retlen = 0;
	int32_t ret = 0;
	int32_t retry_count = 5;
	uint8_t *rbuf;

w_retry:
	ret = mtd->_write(mtd, off, len, &retlen, wbuf);
	if(ret < 0) {
	    if(retry_count--)
		    goto w_retry;
	    if(retry_count < 0) {
		    printf("%s %s %d:write flash failed! ret = %d\n",
		    __FILE__, __func__, __LINE__, ret);
		    return -EIO;
	    }
	}

	retry_count = 5;
	retlen = 0;
	rbuf = calloc(len, sizeof(uint8_t));

r_retry:
	ret = mtd->_read(mtd, off, len, &retlen, rbuf);
	if(ret < 0) {
	    if(retry_count--)
		    goto r_retry;
	    if(retry_count < 0) {
		    printf("%s %s %d:read flash failed! ret = %d\n",
		    __FILE__, __func__, __LINE__, ret);
		    goto failed;
	    }
	}

	if(firmware_buf_compare(wbuf, rbuf, len)) {
		printf("%s %s %d: buf compare err!\n",
			__FILE__, __func__, __LINE__);
		ret = -EIO;
		goto failed;
	}

	free(rbuf);
	return 0;

failed:
	free(rbuf);
	return ret;
}

static int32_t flash_read_blk(struct mtd_info *mtd, uint32_t off, uint32_t len, uint8_t *rbuf) {

	uint32_t retlen = 0;
	int32_t ret = 0;
	int32_t count = 5;

retry_count:
	ret = mtd->_read(mtd, off, len, &retlen, rbuf);
	if(ret < 0 && count--)
		goto retry_count;

	if(count < 0) {
		printf("%s %s %d: flash read error off = 0x%x, len = %x, ret = %d\n",
			__FILE__, __func__, __LINE__, off, len, ret);
		return ret;
	}

	return 0;
}
static int32_t spinand_firmware_write(struct mtd_info *mtd, uint32_t flash_offs,
	uint32_t flash_size, void *buf, uint32_t buf_size) {

	int32_t ret = 0;
	uint8_t i = 0;
	uint8_t errcount = 0;

	struct erase_info instr = {
		.addr = flash_offs,
		.len = mtd->erasesize,
	};

	for(i = 0; i < flash_size / mtd->erasesize; i++) {
		mtd->_erase(mtd, &instr);
		instr.addr += instr.len;
	}

	for(i = 0; i < flash_size / mtd->erasesize; i++) {
		ret = flash_write_blk(mtd, flash_offs, buf_size, buf);
		if(ret) {
			printf("%s %s %d:write data failed! errcount = %d\n",
				__FILE__, __func__, __LINE__, errcount++);
		}
		flash_offs += mtd->erasesize;
	}

	if(errcount == flash_size / mtd->erasesize) {
		printf("all blk write failed!\n");
		return -EIO;
	}

	return 0;
}

int32_t spinand_sn_program(struct cloner *cloner) {

	struct mtd_info *mtd = (void *)nand_info;
	struct sn_config sn = {
		.sn_len = cloner->cmd->write.length,
		.crc_val = cloner->cmd->write.crc,
	};
	int32_t ret = 0;
	void *buf = calloc(sizeof(sn) + sn.sn_len, sizeof(uint8_t));
	if(!buf) {
		printf("alloc mem failed!\n");
		return -ENOMEM;
	}

	memcpy(buf, &sn, sizeof(sn));
	memcpy(buf + sizeof(sn), (void *)cloner->write_req->buf, sn.sn_len);

	if(spinand_firmware_write(mtd, mtd->size + CONFIG_MAC_SIZE, CONFIG_SN_SIZE,
		    buf, sizeof(sn) + sn.sn_len)) {
		printf("#########burner sn firware failed!\n");
		ret = -EIO;
	}

	free(buf);
	return ret;
}


int32_t spinand_sn_read(struct cloner *cloner) {

	struct mtd_info *mtd = (void *)nand_info;
	struct sn_config sn;
	uint32_t read_off = mtd->size + CONFIG_MAC_SIZE;
	int32_t ret = 0, i = 0;
	void *buf = cloner->read_req->buf;

	for(i = 0; i < CONFIG_SN_SIZE / mtd->erasesize; i++) {
		memset(&sn, 0, sizeof(sn));
		ret = flash_read_blk(mtd, read_off, sizeof(sn), &sn);
		if(!ret && sn.sn_len != 0 && sn.crc_val != 0)
			break;
		printf("%s %s %d: read sn config failed!, retrycount = %d\n",
			__FILE__, __func__, __LINE__, i);
		read_off += mtd->erasesize;
	}

	if(i == CONFIG_SN_SIZE / mtd->erasesize) {
		printf("%s %s %d: read sn config failed!\n",
			__FILE__, __func__, __LINE__);
		return -EIO;
	}

	if(sn.sn_len == -1 ||
	    sn.crc_val == -1 ||
	    sn.sn_len >= FMW_SIZE_MAX) {
		printf("sn data error!\n");
		return -EINVAL;
	}

	memcpy(buf, &sn, sizeof(sn));
	buf += sizeof(sn);

	for(; i < CONFIG_SN_SIZE / mtd->erasesize; i++) {
		if(!flash_read_blk(mtd, read_off + sizeof(sn), sn.sn_len, buf)) {
			if(local_crc32(0xffffffff, buf, sn.sn_len) == sn.crc_val)
				break;
		}
		printf("%s %s %d: read sn buf failed!, retrycount = %d\n",
			__FILE__, __func__, __LINE__, i);
		read_off += mtd->erasesize;
	}

	if(i == CONFIG_SN_SIZE / mtd->erasesize) {
		printf("%s %s %d: read sn failed!\n",
			__FILE__, __func__, __LINE__);
		return -EIO;
	}
	return 0;
}

int32_t spinand_mac_program(struct cloner *cloner) {

	struct mtd_info *mtd = (void *)nand_info;
	struct mac_config mac = {
		.mac_len = cloner->cmd->write.length,
		.crc_val = cloner->cmd->write.crc,
	};
	int32_t ret = 0;
	void *buf = calloc(sizeof(mac) + mac.mac_len, sizeof(uint8_t));
	if(!buf) {
		printf("alloc mem failed!\n");
		return -ENOMEM;
	}

	memcpy(buf, &mac, sizeof(mac));
	memcpy(buf + sizeof(mac), (void *)cloner->write_req->buf, mac.mac_len);

	if(spinand_firmware_write(mtd, mtd->size, CONFIG_MAC_SIZE,
				buf, sizeof(mac) + mac.mac_len)) {
		printf("#########burner mac firmware failed!\n");
		ret = -EIO;
	}
	free(buf);
	return ret;
}
int32_t spinand_mac_read(struct cloner *cloner) {

	struct mtd_info *mtd = (void *)nand_info;
	struct mac_config mac;
	uint32_t read_off = mtd->size;
	int32_t ret = 0, i = 0;
	void *buf = cloner->read_req->buf;

	for(i = 0; i < CONFIG_MAC_SIZE / mtd->erasesize; i++) {
		memset(&mac, 0, sizeof(mac));
		ret = flash_read_blk(mtd, read_off, sizeof(mac), &mac);
		if(!ret && mac.mac_len == 12 && mac.crc_val != 0)
			break;
		printf("%s %s %d: read mac config failed!, retrycount = %d\n",
			__FILE__, __func__, __LINE__, i);
		read_off += mtd->erasesize;
	}

	if(i == CONFIG_MAC_SIZE / mtd->erasesize) {
		printf("%s %s %d: read mac config failed!\n",
			__FILE__, __func__, __LINE__);
		return -EIO;
	}

	if(mac.mac_len != 12 ||
		mac.crc_val == -1 ||
		mac.mac_len >= FMW_SIZE_MAX) {
		printf("mac data error!\n");
		return -EINVAL;
	}

	memcpy(buf, &mac, sizeof(mac));
	buf += sizeof(mac);

	for(; i < CONFIG_MAC_SIZE / mtd->erasesize; i++) {
		if(!flash_read_blk(mtd, read_off + sizeof(mac), mac.mac_len, buf)) {
			if(local_crc32(0xffffffff, buf, mac.mac_len) == mac.crc_val)
				break;
		}
		printf("%s %s %d: read mac buf failed!, retrycount = %d\n",
			__FILE__, __func__, __LINE__, i);
		read_off += mtd->erasesize;
	}

	if(i == CONFIG_SN_SIZE / mtd->erasesize) {
		printf("%s %s %d: read mac failed!\n",
			__FILE__, __func__, __LINE__);
		return -EIO;
	}
	return 0;
}


