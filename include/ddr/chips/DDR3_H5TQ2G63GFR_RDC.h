#ifndef __DDR_CONFIG_H__
#define __DDR_CONFIG_H__

/*
 * This file contains the memory configuration parameters for the cygnus board.
 */
/*--------------------------------------------------------------------------------
 * DDR3-1333 info
 */
/* DDR3 paramters */
#define DDR_ROW     14  /* ROW : 12 to 18 row address ,1G only 512MB*/
#define DDR_ROW1     14  /* ROW : 12 to 18 row address ,1G only 512MB*/
#define DDR_COL     10  /* COL :  8 to 14 column address */
#define DDR_COL1     10  /* COL :  8 to 14 column address */
#define DDR_BANK8   1 	/* Banks each chip: 0-4bank, 1-8bank */
#define DDR_CL      7   /* CAS latency: 7 is much more stable than 6 on mensa board */
#define DDR_tCWL   (DDR_CL - 1)	/* DDR3 only: CAS Write Latency, 5 to 8 */

/*
 * DDR3 controller timing1 register
 */
#define DDR_tRAS DDR__ps(37500)  /* tRAS: ACTIVE to PRECHARGE command period to the same bank. ns*/
#define DDR_tRP  DDR__ps(13125)  /* tRP: PRECHARGE command period to the same bank. ns*/
#define DDR_tRCD DDR__ps(13125)  /* ACTIVE to READ or WRITE command period to the same bank. ns*/
#define DDR_tRC  DDR__ps(50625)  /* ACTIVE to ACTIVE command period to the same bank. ns*/
#define DDR_tWR  DDR__ns(15)  /*FIXME WRITE Recovery Time defined by register MR of DDR2 memory, ns*/
#define DDR_tRRD DDR__tck(6) /* FIXME ACTIVE bank A to ACTIVE bank B command period. DDR3 - tCK*/
#define DDR_tRTP DDR_SELECT_MAX__tCK_ps(4, 7500) /* FIXME READ to PRECHARGE command period. DDR3 spec no. 7.5ns*/
#define DDR_tWTR DDR_SELECT_MAX__tCK_ps(4, 7500) /* FIXME WRITE to READ command delay. DDR3 spec no. 7.5 ns*/

/*
 * DDR3 controller timing2 register
 */
#define DDR_tRFC   DDR__tck(86) 	/* AUTO-REFRESH command period. DDR3 - ns*/
#define DDR_tXP    DDR_SELECT_MAX__tCK_ps(3, 6000)	/*FIXME DDR3 only: Exit active power down to any valid command, ns*/
#define DDR_tMRD   DDR__tck(4)    /*FIXME unit: tCK. Load-Mode-Register to next valid command period: DDR3 rang 4 to 7 tCK. DDR3 spec no */

/* new add */
#define DDR_BL	   8   /* DDR3 Burst length: 0 - 8 burst, 2 - 4 burst , 1 - 4 or 8(on the fly)*/
#define DDR_tCCD   DDR__tck(4)	/* CAS# to CAS# command delay , tCK. 4 or 5 */
#define DDR_tFAW   DDR__tck(27)	/* Four bank activate period, DDR3 - tCK */
#define DDR_tCKE   	DDR_SELECT_MAX__tCK_ps(3, 7500)	/* CKE minimum pulse width, DDR3 spec no, tCK */
#define DDR_RL 	DDR__tck(DDR_CL)	/* DDR3: Read Latency = tAL + tCL */
#define DDR_WL 	DDR__tck(DDR_tCWL)	/* DDR3: Write Latency = tAL + tCWL */
#define DDR_tCKSRE 	DDR_SELECT_MAX__tCK_ps(5, 10000) /* Valid Clock Requirement after Self Refresh Entry or Power-Down Entry */
#define DDR_tCKESR 	DDR__tck(9)     /* Minimum Self-Refresh / Deep-Power-Down , tCK, no */
//#define DDR_tCKESR 	(DDR_tCKE + DDR__tck(1))     /* Minimum Self-Refresh / Deep-Power-Down , tCK, no */

#define DDR_tXSDLL 	DDR__tck(512)		/* DDR3 only: EXit self-refresh to command requiring a locked DLL, tck*/
#define DDR_tMOD   	DDR_SELECT_MAX__tCK_ps(12, 15 * 1000)	/* DDR3 only: Mode Register Set Command update delay*/
#define DDR_tXPDLL 	DDR_SELECT_MAX__tCK_ps(10, 24 * 1000)	 /* DDR3 only: Exit active power down to command requirint a locked DLL, ns*/
#define DDR_tXS    	DDR_SELECT_MAX__tCK_ps(5, (DDR_tRFC + 10000)) /* DDR3 only: EXit self-refresh to command not requiring a locked DLL, ns*/
#define DDR_tXSRD  	DDR__tck(10)		/* DDR2 only: Exit self refresh to a read command, tck */

/*
 * DDR3 controller refcnt register
 */
#define DDR_tREFI   DDR__ns(7800)	/* Refresh period: 64ms / 32768 = 1.95 us , 2 ^ 15 = 32768 */

#endif /* __DDR_CONFIG_H__ */
