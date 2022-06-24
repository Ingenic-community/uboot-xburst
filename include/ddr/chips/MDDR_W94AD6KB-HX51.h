#ifndef __MDDR_CONFIG_H
#define __MDDR_CONFIG_H

/*
 * This file contains the memory configuration parameters for the cygnus board.
 */
/*--------------------------------------------------------------------------------
 * MDDR info
 */
/* MDDR paramters */
#define DDR_ROW		13 /* ROW : 12 to 14 row address */
#define DDR_COL		10 /* COL :  8 to 10 column address */
#define DDR_BANK8	0 /* Banks each chip: 0-4bank, 1-8bank 0 for falcon fpga, 1 for develop board */
#define DDR_CL		3 /* CAS latency: 2 or 3 */

/*
 * MDDR controller timing1 register
 */
#define DDR_tRAS	DDR__ns(42) /* tRAS:42 to 70000 ACTIVE to PRECHARGE command period to the same bank. */
#define DDR_tRP		DDR__ns(18) //3 /* tRP: 3 to x PRECHARGE command period to the same bank */
#define DDR_tRCD	DDR__ns(18) /* 18 to x ACTIVE to READ or WRITE command period to the same bank. */
#define DDR_tRC		(DDR_tRAS + DDR_tRP) /*45 to x ACTIVE to ACTIVE command period to the same bank.*/
#define DDR_tRRD	DDR__ns(12) /*12 to x ACTIVE bank A to ACTIVE bank B command period. */
#define DDR_tWR		DDR__ns(15) /* 15 to x WRITE Recovery Time defined by register MR of DDR2 memory */
#define DDR_tWTR	DDR__tck(2) /* 1 to x WRITE to READ command delay. */

/*
 * MDDR controller timing2 register
 */
#define DDR_tRFC	DDR__ns(140) //72 to max/* ns,  AUTO-REFRESH command period. */
#define DDR_tXP		DDR__ns(25) /* 2 to max EXIT-POWER-DOWN to next valid command period: 1 to 8 tCK. */

#define DDR_tMRD	DDR__tck(2) /* 2 to max  unit: tCK Load-Mode-Register to next valid command period: 1 to 4 tCK */

/* new add */
#define DDR_BL		4 /* MDDR Burst length: 3 - 8 burst, 2 - 4 burst , 1 - 2 burst*/
#define DDR_RL		DDR__tck(DDR_CL) /* MDDR: Read Latency = tAL + tCL */
#define DDR_WL		DDR__tck(1) /* MDDR: must 1 */

#define DDR_tCKE	DDR__tck(2) /* 1 to max * CKE minimum pulse width, tCK */
#define DDR_tXSR	DDR__ns(200) /* 120 to max Exit self-refresh to next valid command delay, ns */
#define DDR_tDQSSMAX	DDR__tck(1)

/*
 * MDDR controller refcnt register
 */
#define DDR_tREFI	DDR__ns(7800) /* Refresh period: 4096 refresh cycles/64ms , line / 64ms , 8192 lines*/

#endif /* __MDDR_CONFIG_H */


