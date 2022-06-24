#include <config.h>
#include <common.h>
#include <ddr/ddr_common.h>
#include <generated/ddr_reg_values.h>

#include <asm/io.h>
#include <asm/arch/clk.h>


static unsigned int * test_data;
static unsigned int * test_data_uncache;
#define REG32(addr) *(volatile unsigned int *)(addr)
static inline void set_gpio_func(int gpio, int type) {
	int i;
	int port = gpio / 32;
	int pin = gpio & 0x1f;
	int addr = 0xb0010010 + port * 0x100;

	for(i = 0;i < 4;i++){
		REG32(addr + 0x10 * i) &= ~(1 << pin);
		REG32(addr + 0x10 * i) |= (((type >> (3 - i)) & 1) << pin);
	}
}

void dump_reg()
{
	printf("DDRC_DLP: %x\n",ddr_readl(DDRC_DLP));
	printf("DDRP_DSGCR: %x\n",ddr_readl(DDRP_DSGCR));
	printf("DDRP_ACDLLCR: %x\n",ddr_readl(DDRP_ACDLLCR));

	printf("DDRP_DX0DLLCR: %x\n",ddr_readl(DDRP_DXDLLCR(0)));
	printf("DDRP_DX1DLLCR: %x\n",ddr_readl(DDRP_DXDLLCR(1)));
	printf("DDRP_DX2DLLCR: %x\n",ddr_readl(DDRP_DXDLLCR(2)));
	printf("DDRP_DX3DLLCR: %x\n",ddr_readl(DDRP_DXDLLCR(3)));
}
#define TCSM_DELAY(x)					\
	do{							\
		register unsigned int i = x;			\
	while(i--)					\
		__asm__ volatile(".set mips32\n\t"	\
				 "nop\n\t"		\
					 ".set mips32");	\
	}while(0)

#define TEST_SIZE (4 * 1024 * 1024)
void ddr_selfresh_test()
{
	unsigned int val;
	unsigned int run = 1;
	int result = 0;
	int timeout = 500000;

	test_data = 0x80600000;
	test_data_uncache = (unsigned int *)((unsigned int)test_data | 0xa0000000);
	printf("%s %s",__FILE__,__DATE__);
	for(val = 0; val < TEST_SIZE / 4;val++)
		test_data_uncache[val] = (unsigned int)&test_data_uncache[val];
	dump_reg();
//-----------------------------------------
	ddr_writel(0xf003,DDRC_DLP);
	dump_reg();
//-----------------------------------------
/*
 *   ddr selrefret ok;
 */
	/* val = ddr_readl(DDRP_DSGCR); */
	/* val &= ~(1 << 4); */
	/* ddr_writel(val,DDRP_DSGCR); */
	/* dump_reg(); */
//-----------------------------------------
	while(run) {
		val = ddr_readl(DDRC_CTRL);
		val |= 1 << 5;
		ddr_writel(val, DDRC_CTRL);
		printf("ddr self-refresh\n");
		printf("ddr self-refresh\n");
		printf("ddr self-refresh\n");
		printf("ddr self-refresh\n");
		printf("ddr self-refresh\n");
		printf("ddr self-refresh\n");
		printf("ddr self-refresh\n");
		mdelay(1000);

//---------------------------------------------------
		/* val = ddr_readl(DDRP_ACDLLCR); */
		/* val &= ~(1 << 30); */
		/* ddr_writel(val,DDRP_ACDLLCR); */
		/* mdelay(1); */
		/* val = ddr_readl(DDRP_ACDLLCR); */
		/* val |= (1 << 30); */
		/* ddr_writel(val,DDRP_ACDLLCR); */
		/* mdelay(200); */

		/* val = ddr_readl(DDRP_DXDLLCR(0)); */
		/* val &= ~(1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(0)); */
		/* mdelay(1); */
		/* val = ddr_readl(DDRP_DXDLLCR(0)); */
		/* val |= (1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(0)); */
		/* mdelay(1); */


		/* val = ddr_readl(DDRP_DXDLLCR(1)); */
		/* val &= ~(1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(1)); */
		/* mdelay(1); */
		/* val = ddr_readl(DDRP_DXDLLCR(1)); */
		/* val |= (1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(1)); */
		/* mdelay(1); */


		/* val = ddr_readl(DDRP_DXDLLCR(2)); */
		/* val &= ~(1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(2)); */
		/* mdelay(1); */
		/* val = ddr_readl(DDRP_DXDLLCR(2)); */
		/* val |= (1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(2)); */
		/* mdelay(1); */


		/* val = ddr_readl(DDRP_DXDLLCR(3)); */
		/* val &= ~(1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(3)); */
		/* mdelay(1); */
		/* val = ddr_readl(DDRP_DXDLLCR(3)); */
		/* val |= (1 << 30); */
		/* ddr_writel(val,DDRP_DXDLLCR(3)); */
		/* mdelay(200); */

		/* dump_reg(); */
//---------------------------------------------------

		printf("1\n");
		val = ddr_readl(DDRC_CTRL);
		val &= ~(1 << 5);
		ddr_writel(val, DDRC_CTRL);
		printf("2\n");
		TCSM_DELAY(10);
//		mdelay(10);
		*(volatile unsigned int *)0xb301102c |= (1 << 4);
		TCSM_DELAY(10);
//		mdelay(1);

		{
			unsigned int lcr;
			unsigned int opcr;
			unsigned int opcr_save;

			set_gpio_func(32*1+31, 10);
			/* init opcr and lcr for idle */
			lcr = REG32(0xb0000004);
			lcr &= ~(0x3);		/* LCR.SLEEP.DS=0'b0,LCR.LPM=1'b00*/
			lcr |= 0xff << 8;	/* power stable time */
			REG32(0xb0000004) = lcr;

			opcr = REG32(0xb0000024);
			opcr |= 0xfff << 8;	/* EXCLK stable time */
			opcr &= ~(1 << 4);	/* EXCLK stable time */
			REG32(0xb0000024) = opcr;

			lcr = REG32(0xb0000004);
			lcr &= ~3;
			lcr |= 1;
			REG32(0xb0000004) = lcr;
	/* OPCR.MASK_INT bit30*/
	/* set Oscillator Stabilize Time bit8*/
	/* disable externel clock Oscillator in sleep mode bit4*/
	/* select 32K crystal as RTC clock in sleep mode bit2*/
			/* printf("#####opcr:%08x\n", *(volatile unsigned int *)0xb0000024); */

	/* set Oscillator Stabilize Time bit8*/
	/* disable externel clock Oscillator in sleep mode bit4*/
	/* select 32K crystal as RTC clock in sleep mode bit2*/
			opcr = REG32(0xb0000024);
			opcr_save = opcr;
			opcr &= ~((1 << 7) | (1 << 6) | (1 << 4));
			opcr |= (1 << 30) | (1 << 28) | (1 << 25) | (1 << 23) | (0xff << 8) | (1 << 2);
			REG32(0xb0000024) = opcr;

//			*(volatile unsigned int *)(0xb0000020) = 0x0fdefffe;

	/* sysfs */
			/* load_func_to_tcsm((unsigned int *)SLEEP_TCSM_RESUME_TEXT,(unsigned int *)cpu_resume,256); */

			/* TCSM_PCHAR('n'); */
			/* TCSM_PCHAR('n'); */
			/* TCSM_PCHAR('n'); */
			/* TCSM_PCHAR('n'); */
			/* TCSM_PCHAR('n'); */
			/* TCSM_PCHAR('n'); */
			/* local_irq_disable(); */
	/* reim = get_smp_reim(); */
	/* reim &= ~(0xffff << 16); */
	/* reim |= (unsigned int)SLEEP_TCSM_RESUME_TEXT & (0xffff << 16); */
	/* set_smp_reim(reim); */
	/* printk("reim = 0x%08x\n",get_smp_reim()); */
	/* addr = __read_32bit_c0_register($12, 7); */
	/* printk("addr = 0x%08x\n",addr); */
	/* __write_32bit_c0_register($12, 7, (unsigned int)SLEEP_TCSM_RESUME_TEXT & (0xffff)); */

	/* addr = __read_32bit_c0_register($12, 7); */
	/* printk("addr = 0x%08x\n",addr); */

//			config_powerdown_core((unsigned int *)SLEEP_TCSM_RESUME_TEXT);

	/* blast_dcache32(); */
	/* blast_icache32(); */
	/* blast_scache32(); */

/* 	cache_prefetch(LABLE1,200); */
/* 	/\* blast_dcache32(); *\/ */
/* 	/\* __sync(); *\/ */
/* 	/\* __fast_iob(); *\/ */
/* LABLE1: */
			/* val = ddr_readl(DDRC_CTRL);//DDR keep selrefresh,when it exit the sleep state. */
			/* val |= (1 << 17);//enter to hold ddr state */
			/* ddr_writel(val,DDRC_CTRL); */
			printf("entry\n");
			/* *((volatile unsigned int*)(0xb30100bc)) &= ~(0x1); */
			REG32(0xB000100C) = 0x1 << 16;

			__asm__ volatile(".set mips32\n\t"
					 "wait\n\t"
					 "nop\n\t"
					 "nop\n\t"
					 "nop\n\t"
					 /* "jr %0\n\t" */
					 ".set mips32 \n\t"
					 /* : */
					 /* :"r"(SLEEP_TCSM_RESUME_TEXT) */
				);
			REG32(0xB0001008) = 0x1 << 16;
			printf("opcr_save = %x\n", opcr_save);
			/* opcr = REG32(0xb0000024); */
			/* opcr &= ~((1 << 2) | (1 << 28)); */
			REG32(0xb0000024) = opcr_save;
		}
		/* *(volatile unsigned int *)CPM_DRCG = 0x53; */
		/* mdelay(1); */
		/* val = ddr_readl(DDRC_CTRL); */
		/* val |= 1 << 1; */
		/* val &= ~(1<< 17);// exit to hold ddr state */
		/* ddr_writel(val,DDRC_CTRL); */
		/* val = ddr_readl(DDRC_CTRL); */

		/* *(volatile unsigned int *)CPM_DRCG = 0x51; */
		/* mdelay(1); */


		/* *((volatile unsigned int*)(0xb30100bc)) |= (0x1); */
		/* mdelay(1); */

//-----------------------------------------
		/* *(volatile unsigned int *)(0xb3011000 + 0x80) |= (1 << 22) | 1; */
		/* printf("AAAA DDRC_PHYRST:%x\n",*(volatile unsigned int *)(0xb3011000 + 0x80)); */
		/* mdelay(10); */
		/* *(volatile unsigned int *)(0xb3011000 + 0x80) &= ~((1 << 22)  | 1); */
		/* mdelay(10); */
		/* printf("DDRC_PHYRST:%x\n",*(volatile unsigned int *)(0xb3011000 + 0x80)); */
//-----------------------------------------
		/* *(volatile unsigned int *)CPM_DRCG = 0x53; */
		/* TCSM_DELAY(0x1ff); */

		/* *(volatile unsigned int *)CPM_DRCG = 0x51; */
		/* TCSM_DELAY(0x1ff); */

		/* val =  DDRP_PIR_ITMSRST | 1 << 17 | 1 << 29 | 1 << 28;// | 1 << 7 | 1 << 17 | 1 << 29; */
		/* ddr_writel(val, DDRP_PIR); */

		/* while (!(ddr_readl(DDRP_PGSR) == (DDRP_PGSR_IDONE) && --timeout)) */
		/* 	printf("ddr_readl(DDRP_PGSR) = %x\n", ddr_readl(DDRP_PGSR)); */

		/* if (timeout == 0) { */
		/* 	printf("DDR PHY init timeout: PGSR=%X\n", ddr_readl(DDRP_PGSR)); */
		/* 	hang(); */
		/* } */
		ddr_writel(0, DDRP_DTAR);

		val = DDRP_PIR_INIT | DDRP_PIR_ITMSRST  | 1 << 17 | 1 << 29;// | 1 << 7 | 1 << 17 | 1 << 29;
		ddr_writel(val, DDRP_PIR);
		mdelay(100);
		val = DDRP_PIR_INIT | 1 << 7  | 1 << 17 | 1 << 29;// | 1 << 7 | 1 << 17 | 1 << 29;
		ddr_writel(val, DDRP_PIR);
		while(ddr_readl(DDRP_PGSR) != 0x1f) {
		while ((ddr_readl(DDRP_PGSR) != (DDRP_PGSR_IDONE
						 | DDRP_PGSR_DLDONE
						 | DDRP_PGSR_ZCDONE
						 | DDRP_PGSR_DIDONE
						 | DDRP_PGSR_DTDONE))
		       && !(ddr_readl(DDRP_PGSR)
			    & (DDRP_PGSR_DTDONE | DDRP_PGSR_DTERR | DDRP_PGSR_DTIERR))
		       && --timeout)
			printf("-------ddr_readl(DDRP_PGSR) = %x\n", ddr_readl(DDRP_PGSR));

		if (timeout == 0) {
			printf("DDR training timeout\n");
			result = -1;
		} else if (ddr_readl(DDRP_PGSR)
			   & (DDRP_PGSR_DTERR | DDRP_PGSR_DTIERR)) {
			printf("DDR hardware training error\n");
			result = ddr_readl(DDRP_PGSR);
			printf("ddr_readl(DDRP_PGSR) = %x\n", ddr_readl(DDRP_PGSR));
			val = 1 << 28  | 1 << 17 | 1 << 29;// | 1 << 7 | 1 << 17 | 1 << 29;
			ddr_writel(val, DDRP_PIR);
		}
		printf("ddr_readl(DDRP_PGSR) = %x\n", ddr_readl(DDRP_PGSR));

		}
		printf("3\n");
		for(val = 0; val < TEST_SIZE/ 4;val++) {
			/* printf("d=%x  e=%x\n", ); */
		}

		for(val = 0; val < TEST_SIZE/ 4;val++) {
			while(test_data_uncache[val] != (unsigned int)&test_data_uncache[val])
			{
				printf("%d: d=%x  e=%x\n",val,test_data_uncache[val],(unsigned int)&test_data_uncache[val]);
				run = 1;
			}
		}
		printf("ddddddddddddddddddddddddddd============\n");
		for(val = 0; val < TEST_SIZE/ 4;val++) {
			while(test_data_uncache[val] != (unsigned int)&test_data_uncache[val])
			{
				printf("%d: d=%x  e=%x\n",val,test_data_uncache[val],(unsigned int)&test_data_uncache[val]);
				run = 1;
			}
		}
		/* for(val = 0; val < TEST_SIZE / 4;val++) */
		/* 	test_data_uncache[val] = (unsigned int)&test_data_uncache[val]; */
		printf("ddr test finish!\n");
	}
	while(1);
}
