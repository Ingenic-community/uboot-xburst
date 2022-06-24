#ifndef __MDDR_CONFIG_H
#define __MDDR_CONFIG_H

/*
 * This file contains the memory configuration parameters for the cygnus board.
 */

/* 
 * MDDR paramters
 */
#define DDR_ROW    13	/* ROW : 12 to 14 row address */
#define DDR_ROW1   13	/* ROW : 12 to 14 row address */
#define DDR_COL    9	/* COL :  8 to 10 column address */
#define DDR_COL1   9	/* COL :  8 to 10 column address */
#define DDR_BANK8  0	/* Banks each chip: 0 - 4 bank for falcon fpga, 1 - 8 bank for develop board */
#define DDR_BL	   8    /* MDDR Burst length: 2 or 4 or 8 or 16 burst */
#define DDR_CL	   3	/* CAS latency: 2 or 3 */
#define DDR_WL     1
#define DDR_RL     DDR__tck(3)
#define DDR_tCKE   DDR__tck(2)	/* CKE minimum pulse width : 1 - x tCK */
#define DDR_tXSRD  DDR__ns(120) /* Exit self-refresh to next valid command delay, 120 - x ns */
#define DDR_tXSR   DDR__ns(200)
#define DDR_tDQSSMAX  DDR__tck(1)

/*
 * MDDR controller timing1 register
 */
#define DDR_tRAS   DDR__ns(40) /* ACTIVE to PRECHARGE command period to the same bank : 40 - 70000 ns */
#define DDR_tRP    DDR__tck(3) /* PRECHARGE command period to the same bank : 3 - x */
#define DDR_tRC    (DDR_tRAS + DDR_tRP) /* ACTIVE to ACTIVE command period to the same bank : 45 - x */
#define DDR_tRCD   DDR__ns(15) /* ACTIVE to READ or WRITE command period to the same bank : 15 - x */
#define DDR_tRRD   DDR__ns(10) /* ACTIVE bank A to ACTIVE bank B command period : 12 - x  */
#define DDR_tWR    DDR__ns(15) /* WRITE Recovery Time defined by register MR of DDR2 memory : 15 - x */
#define DDR_tWTR   DDR__tck(1) /* WRITE to READ command delay : 1 - x */

/*
 * MDDR controller timing2 register
 */
#define DDR_tRFC   DDR__ns(72)	/* AUTO-REFRESH command period. max to 72 */
#define DDR_tXP	   DDR__tck(2)	/* EXIT-POWER-DOWN to next valid command period : 1 to 8 tCK */
#define DDR_tMRD   DDR__tck(2)  /* Load-Mode-Register to next valid command period : 1 to 4 tCK */

/*
 * MDDR controller refcnt register
 */
#define DDR_tREFI  DDR__ns(7800) /* Refresh period: 4096 refresh cycles/64ms , line / 64ms , 8192 lines*/

#define DDR_CLK_DIV 1    /* Clock Divider. auto refresh
			  * cnt_clk = memclk / (16 * (2 ^ DDR_CLK_DIV))
			  */

#endif /* __MDDR_CONFIG_H */


