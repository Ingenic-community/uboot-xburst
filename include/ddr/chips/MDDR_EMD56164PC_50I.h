#ifndef __MDDR_CONFIG_H
#define __MDDR_CONFIG_H

/*
 * This file contains the memory configuration parameters for the cygnus board.
 */
/*--------------------------------------------------------------------------------
 * MDDR info
 */
/* Chip Select */
//#define DDR_CS1EN 1 // CSEN : whether a ddr chip exists 0 - un-used, 1 - used
//#define DDR_CS0EN 1
//#define DDR_DW32 1/* 0 - 16-bit data width, 1 - 32-bit data width */

/* MDDR paramters */
#define DDR_ROW 13 /* ROW : 12 to 14 row address */
#define DDR_ROW1 13 /* ROW : 12 to 14 row address */
#define DDR_COL 9/* COL :  8 to 10 column address */
#define DDR_COL1 9/* COL :  8 to 10 column address */
#define DDR_BANK8 0 /* Banks each chip: 0-4bank, 1-8bank 0 for falcon fpga, 1 for develop board */
#define DDR_CL 3 /* CAS latency: 1 to 7 */

/*
 * MDDR controller timing1 register
 */
#define DDR_tRAS DDR__ns(40) /*tRAS: ACTIVE to PRECHARGE command period to the same bank. */
#define DDR_tRP  DDR__tck(3) /* tRP: PRECHARGE command period to the same bank */
#define DDR_tRCD DDR__ns(15) /* ACTIVE to READ or WRITE command period to the same bank. */
#define DDR_tRC  (DDR_tRAS + DDR_tRP) /* ACTIVE to ACTIVE command period to the same bank.*/
#define DDR_tRRD DDR__ns(10)   /* ACTIVE bank A to ACTIVE bank B command period. */
#define DDR_tWR  DDR__ns(15) /* WRITE Recovery Time defined by register MR of DDR2 memory */
#define DDR_tWTR DDR__tck(2) /* WRITE to READ command delay. */

/*
 * MDDR controller timing2 register
 */
#define DDR_tRFC DDR__ns(72) /* ns,  AUTO-REFRESH command period. */

#define DDR_tXP  DDR__tck(3) /* EXIT-POWER-DOWN to next valid command period: 1 to 8 tCK. */
#define DDR_tMRD DDR__tck(2) /* unit: tCK Load-Mode-Register to next valid command period: 1 to 4 tCK */

/* new add */
#define DDR_BL	4   /* MDDR Burst length: 3 - 8 burst, 2 - 4 burst , 1 - 2 burst*/
#define DDR_RL  DDR__tck(DDR_CL)	/* MDDR: Read Latency = tAL + tCL */
#define DDR_WL  DDR__tck(1)		/* MDDR: must 1 */

#define DDR_tCKE DDR__tck(2)		/* CKE minimum pulse width, tCK */
#define DDR_tXSR DDR__ns(120)
#define DDR_tDQSSMAX DDR__tck(1)

/*
 * MDDR controller refcnt register
 */
#define DDR_tREFI	        DDR__ns(3900)	/* Refresh period: 4096 refresh cycles/64ms */
#endif /* __MDDR_CONFIG_H */
