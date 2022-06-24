#ifdef CONFIG_JZ_SPI
extern unsigned int ssi_rate;

struct spi_param *spi_args;

int spi_erase()
{
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	struct spi_flash *flash;
	spi.rate  = spi_args->rate ;
	ssi_rate  = spi.rate;

#ifdef CONFIG_JZ_SPI
	spi_init();
#endif

	if(flash == NULL){
		flash = spi_flash_probe(bus, cs, spi.rate, mode);
		if (!flash) {
			printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
			return 1;
		}
	}
	jz_erase_all(flash);
	printf("spi erase ok\n");
	return 0;
}



int spi_program(struct cloner *cloner)
{
	unsigned int bus = CONFIG_SF_DEFAULT_BUS;
	unsigned int cs = CONFIG_SF_DEFAULT_CS;
	unsigned int speed = CONFIG_SF_DEFAULT_SPEED;
	unsigned int mode = CONFIG_SF_DEFAULT_MODE;
	u32 offset = cloner->cmd->write.partition + cloner->cmd->write.offset;
	u32 length = cloner->cmd->write.length;
	int blk_size = spi_args->spi_erase_block_siz;
	void *addr = (void *)cloner->write_req->buf;
	unsigned int ret;
	int len = 0;
	struct spi_flash *flash;
	spi.rate  = spi_args->rate ;
	ssi_rate = spi.rate;

#ifdef CONFIG_JZ_SPI
	spi_init();
#endif

#ifdef CONFIG_INGENIC_SOFT_SPI
	spi_init_jz(&spi);
#endif

	if(flash == NULL){
		flash = spi_flash_probe(bus, cs, spi.rate, mode);
		if (!flash) {
			printf("Failed to initialize SPI flash at %u:%u\n", bus, cs);
			return -1;
		}
	}

	BURNNER_PRI("the offset = %x\n",offset);
	BURNNER_PRI("the length = %x\n",length);


	if (length < blk_size || length%blk_size == 0){
		len = length;
		BURNNER_PRI("the length = %x,blk_size = %x\n",length,blk_size);
	}
	else{
		BURNNER_PRI("the length = %x, is no enough %x\n",length,blk_size);
		len = (length/blk_size)*blk_size + blk_size;
	}

	if (spi_args->spi_erase == SPI_NO_ERASE) {
		ret = spi_flash_erase(flash, offset, len);
		BURNNER_PRI("SF: %zu bytes @ %#x Erased: %s\n", (size_t)len, (u32)offset,
				ret ? "ERROR" : "OK");
	}

	ret = spi_flash_write(flash, offset, len, addr);
	BURNNER_PRI("SF: %zu bytes @ %#x write: %s\n", (size_t)len, (u32)offset,
			ret ? "ERROR" : "OK");


	if (debug_args->write_back_chk) {
		spi_flash_read(flash, offset,len, addr);

		uint32_t tmp_crc = local_crc32(0xffffffff,addr,cloner->cmd->write.length);
		debug_cond(BURNNER_DEBUG,"%d blocks check: %s\n",len,(cloner->cmd->write.crc == tmp_crc) ? "OK" : "ERROR");
		if (cloner->cmd->write.crc != tmp_crc) {
			printf("src_crc32 = %08x , dst_crc32 = %08x\n",cloner->cmd->write.crc,tmp_crc);
			return -EIO;
		}
	}

#if debug
	int buf_debug[8*1024*1024];
	if (spi_flash_read(flash, 1024, /*len*/2048, buf_debug)) {
		printf("read failed\n");
		return -1;
	}
	int i = 0;
	for(i=0;i<4096;i++){
		printf("the debug[%d] = %x\n",i,buf_debug[i]);
	}

#endif

	return 0;
}
#endif
