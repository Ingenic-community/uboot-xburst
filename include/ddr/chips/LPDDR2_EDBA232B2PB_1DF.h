#ifndef __LPDDR2_CONFIG_H
#define __LPDDR2_CONFIG_H

/*
 * This file contains the MICRON memory configuration parameters.
 */
/*--------------------------------------------------------------------------------
 * LPDDR2 info
 */
/* LPDDR2 paramters */
#define DDR_ROW 	14 /* ROW : 12 to 14 row address */
#define DDR_ROW1 	14 /* ROW : 12 to 14 row address */
#define DDR_COL 	11  /* COL :  8 to 10 column address */
#define DDR_COL1 	11  /* COL :  8 to 10 column address */
#define DDR_BANK8 	1  /* Banks each chip: 0-4bank, 1-8bank 0 for falcon fpga, 1 for develop board */

/*
 * LPDDR2 controller timing1 register
 */
#define DDR_tRAS 	DDR_SELECT_MAX__tCK_ps(3, 42 * 1000) /* tRAS: ACTIVE to PRECHARGE command period to the same bank. ns*/
#define DDR_tRTP 	DDR_SELECT_MAX__tCK_ps(2, 7500)  /* 7.5ns READ to PRECHARGE command period. ???*/
#define DDR_tRP 	DDR_SELECT_MAX__tCK_ps(3, 21 * 1000) /* tRP: PRECHARGE command period to the same bank, wgao DDR_tRPab 8bank?? */
#define DDR_tRCD 	DDR_SELECT_MAX__tCK_ps(3, 18 * 1000) /* ACTIVE to READ or WRITE command period to the same bank. */
#define DDR_tRC 	(DDR_tRAS + DDR_tRP) /* ACTIVE to ACTIVE command period to the same bank. wgao pb or ab?? */
#define DDR_tRRD 	DDR_SELECT_MAX__tCK_ps(2, 10 * 1000) /* ACTIVE bank A to ACTIVE bank B command period. */
#define DDR_tWR 	DDR_SELECT_MAX__tCK_ps(3, 15 * 1000) /* WRITE Recovery Time defined by register MR of DDR2 memory , ns*/
#define DDR_tWTR 	DDR_SELECT_MAX__tCK_ps(2, 7500)  /* WRITE to READ command delay. */

/*
 * LPDDR2 controller timing2 register
*/
#define DDR_tRFCab	DDR__ns(90) /* Refresh cycle time 64-512MB, ns */
#define DDR_tRFC 	DDR_tRFCab /* AUTO-REFRESH command period. */
#define DDR_tXSR 	DDR_SELECT_MAX__tCK_ps(2, DDR_tRFCab + DDR__ns(10)) 	/* 	    SELF REFRESH exit to next valid command delay. */

#define DDR_tXP 	DDR_SELECT_MAX__tCK_ps(2, 7500)   /* EXIT-POWER-DOWN to next valid command period. ns */
#define DDR_tMRW	DDR__tck(5)	/* Mode Register Write command period */
#define DDR_tMRR	DDR__tck(2)	/* Mode Register Read command period */

/* new add */
#define DDR_tDQSCK    	DDR__ps(2500)	/* LPDDR2 only: DQS output access from ck_t/ck_c, 2.5ns */
#define DDR_tDQSCKMAX 	DDR__ps(5500)	/* LPDDR2 only: MAX DQS output access from ck_t/ck_c, 5.5ns */

#define DDR_BL	 	8	/* wgao??? LPDDR2 Burst length: 3 - 8 burst, 2 - 4 burst , 4 - 16 burst */
#define DDR_RL  	-1 	/* LPDDR2: Read Latency  - 3 4 5 6 7 8 , tck */
#define DDR_WL  	-1 	/* LPDDR2: Write Latency - 1 2 2 3 4 4 , tck */
#define DDR_tCCD 	DDR__tck(DDR_BL/2)	/* CAS# to CAS# command delay , tCK */
#define DDR_tFAW 	DDR_SELECT_MAX__tCK_ps(8, 50 * 1000)	/* Four bank activate period, ns */
#define DDR_tCKESR	DDR_SELECT_MAX__tCK_ps(3, DDR__ns(15))	/* CKE minimum pulse width, tCK */
#define DDR_tCKE	DDR__tck(3)		/* CKE minimum pulse width, tCK */

/*
 * LPDDR2 controller refcnt register
 */
#define DDR_tREFI	DDR__ns(3900)	/* Refresh period */


#endif /* __LPDDR2_CONFIG_H */
