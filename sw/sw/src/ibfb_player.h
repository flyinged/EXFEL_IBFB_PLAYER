/*
 * ibfb_player.h
 *
 *  Created on: 14.03.2016
 *      Author: malatesta_a
 */

#ifndef IBFB_PLAYER_H_
#define IBFB_PLAYER_H_

#include <stdio.h>
#include "func_ctrl_v1_01_a.h"


#define IBFB_CTRL_XFEL_BUCKET_SPACE			48				// 48 clock cycles of 216.(6) MHz
#define IBFB_CTRL_XFEL_BUCKET_NUMBER		2700			// E-XFEL


/* Register map */ //TODO ML84 should be removed
#define NREGS 128
#define REG_RST   0
#define		REG_RST_QSFP0    0x00000001 /* MGT FIFO RESET */
#define		REG_RST_QSFP1    0x00000002
#define		REG_RST_QSFP2    0x00000004
#define		REG_RST_QSFP3    0x00000008
#define		REG_RST_BPM0     0x00000010
#define		REG_RST_BPM1     0x00000020
#define		REG_RST_BPM2     0x00000040
#define		REG_RST_BPM3     0x00000080
#define		REG_RST_P0_0     0x00000100
#define		REG_RST_P0_1     0x00000200
#define		REG_RST_LCNT_Q0  0x00001000 /* loss-of-sync counter reset */
#define		REG_RST_LCNT_Q1  0x00002000
#define		REG_RST_LCNT_Q2  0x00004000
#define		REG_RST_LCNT_Q3  0x00008000
#define		REG_RST_LCNT_B0  0x00010000
#define		REG_RST_LCNT_B1  0x00020000
#define		REG_RST_LCNT_B2  0x00040000
#define		REG_RST_LCNT_B3  0x00080000
#define		REG_RST_LCNT_P00 0x00100000
#define		REG_RST_LCNT_P01 0x00200000
#define		REG_INT_TRIG_EN  0x01000000 /* enable internal trigger */
#define		REG_RST_ALL      0x003FF3FF
#define		REG_RST_LCNT_ALL 0x003FF000
#define REG_LB_QB   1
#define		REG_LB_Q0    0x00000007 /* loopback */
#define		REG_LB_Q1    0x00000070
#define		REG_LB_Q2    0x00000700
#define		REG_LB_Q3    0x00007000
#define		REG_LB_B0    0x00070000
#define		REG_LB_B1    0x00700000
#define		REG_LB_B2    0x07000000
#define		REG_LB_B3    0x70000000
#define REG_LB_P0   2
#define		REG_LB_P0_0  0x00000007 /* loopback */
#define		REG_LB_P0_1  0x00000070
#define REG_MGT_QB 3
#define		REG_MGT_Q13_LOCK 0x00000001 /* mgt pll lock */
#define		REG_MGT_Q02_LOCK 0x00000100
#define		REG_MGT_B01_LOCK 0x00010000
#define		REG_MGT_B23_LOCK 0x01000000
#define		REG_Q0_RSTD      0x00000400 /* mgt reset done */
#define		REG_Q1_RSTD      0x00000004
#define		REG_Q2_RSTD      0x00000200
#define		REG_Q3_RSTD      0x00000002
#define		REG_B0_RSTD      0x00002000
#define		REG_B1_RSTD      0x00004000
#define		REG_B2_RSTD      0x02000000
#define		REG_B3_RSTD      0x04000000
#define		REG_Q0_LOS       0x0000C000 /* mgt loss-of-sync FSM */
#define		REG_Q1_LOS       0x000000C0
#define		REG_Q2_LOS       0x00003000
#define		REG_Q3_LOS       0x00000030
#define		REG_B0_LOS       0x00300000
#define		REG_B1_LOS       0x00C00000
#define		REG_B2_LOS       0x30000000
#define		REG_B3_LOS       0xC0000000
#define REG_MGT_P0 4
#define		REG_MGT_P0_LOCK  0x00000001
#define		REG_P00_RSTD     0x00000002
#define		REG_P01_RSTD     0x00000004
#define		REG_P00_LOS      0x00000030
#define		REG_P01_LOS      0x000000C0
#define     REG_PL_EN_Q0     0x00000100 /* enabled players */
#define     REG_PL_EN_Q1     0x00000200
#define     REG_PL_EN_Q2     0x00000400
#define     REG_PL_EN_Q3     0x00000800
#define     REG_PL_EN_P00    0x00001000
#define     REG_PL_EN_P01    0x00002000
#define     REG_PL_EN_B0     0x00004000
#define     REG_PL_EN_B1     0x00008000
#define     REG_PL_EN_B2     0x00010000
#define     REG_PL_EN_B3     0x00020000
#define     REG_PL_EN_ALL    0x0003FF00
#define     REG_USE_EXT_CLK  0x00100000 /* using external clock */
#define     REG_PL_RAM_AW    0xFF000000
#define REG_PL_START    5
#define     REG_PL_START_Q0  0x00000001 /* player start */
#define     REG_PL_START_Q1  0x00000002
#define     REG_PL_START_Q2  0x00000004
#define     REG_PL_START_Q3  0x00000008
#define     REG_PL_START_P00 0x00000010
#define     REG_PL_START_P01 0x00000020
#define     REG_PL_START_B0  0x00000040
#define     REG_PL_START_B1  0x00000080
#define     REG_PL_START_B2  0x00000100
#define     REG_PL_START_B3  0x00000200
#define     REG_PL_RSYNC_Q0  0x00000400 /* mgt rx sync */
#define     REG_PL_RSYNC_Q1  0x00000800
#define     REG_PL_RSYNC_Q2  0x00001000
#define     REG_PL_RSYNC_Q3  0x00002000
#define     REG_PL_RSYNC_P00 0x00004000
#define     REG_PL_RSYNC_P01 0x00008000
#define     REG_PL_RSYNC_B0  0x00010000
#define     REG_PL_RSYNC_B1  0x00020000
#define     REG_PL_RSYNC_B2  0x00040000
#define     REG_PL_RSYNC_B3  0x00080000
#define     REG_PL_CTRL_EOS  0xFF000000 /* end of stream CTRL value */
#define REG_PL_SENT_Q0  6  /* player sent-packets counter */
#define REG_PL_SENT_Q1  7
#define REG_PL_SENT_Q2  8
#define REG_PL_SENT_Q3  9
#define REG_PL_SENT_P00 10
#define REG_PL_SENT_P01 11
#define REG_PL_SENT_B0  12
#define REG_PL_SENT_B1  13
#define REG_PL_SENT_B2  14
#define REG_PL_SENT_B3  15
#define REG_LCNT_Q10    16 /* loss of sync counters */
#define REG_LCNT_Q32    17
#define REG_LCNT_P10    18
#define REG_LCNT_B10    19
#define REG_LCNT_B32    20
#define 	REG_LCNT_0       0x0000FFFF
#define 	REG_LCNT_1       0xFFFF0000
#define REG_XTRIG_DELAY 21

#define IBFB_K_SOP 0xFB
#define IBFB_K_EOP 0xFD
#define IBFB_K_BAD 0x5C

#define IBFB_CTRL_FUNC_NUM_OF_INT_REGS		27				// number of first registers which have enabled interrupts
#define IBFB_PLAYER_MEM_ELEMS				4096

struct structPlayerFunctions{						//		addr		size =
	volatile unsigned int player_cmd;				//	  0x0000		   4
	volatile unsigned char trg_source;			    //	  0x0004		   1
	volatile unsigned char trg_mode;			    //	  0x0005		   1
	volatile unsigned char pad1;				    //    0x0006           1
	volatile unsigned char pad2;				    //	  0x0007		   1
	volatile unsigned int bunch_delay;			    //	  0x0008		   4
	volatile unsigned short bunch_number;		    //	  0x000C		   2
	volatile unsigned short bunch_space;		    //	  0x000E		   2
};

struct structPlayerCtrlMem {											//		addr		size =
	volatile unsigned int timestamp[IBFB_PLAYER_MEM_ELEMS];				//	  0x0000	   16384
	volatile unsigned int control[IBFB_PLAYER_MEM_ELEMS];				//	  0x4000	   16384
	volatile float x[IBFB_PLAYER_MEM_ELEMS];							//	  0x8000	   16384
	volatile float y[IBFB_PLAYER_MEM_ELEMS];							//	  0xC000	   16384
};

struct structIBFBPlayerRegs {					//		addr		size =
	volatile unsigned char  int_trig_en;	    //		0x00		   1
	volatile unsigned char  resets[3];		    //		0x01		   3
	volatile unsigned short loopback_bpm;	    //		0x04		   2
	volatile unsigned short loopback_sfp;	    //		0x06		   2
	volatile unsigned char  k_sop;	            //		0x08		   1
	volatile unsigned char  k_eop;	            //		0x09		   1
	volatile unsigned short loopback_p0;	    //		0x0A		   2
	volatile unsigned char  bpm23_status;       //		0x0C		   1
	volatile unsigned char  bpm01_status;       //		0x0D		   1
	volatile unsigned char  qsfp02_status;      //		0x0E		   1
	volatile unsigned char  qsfp13_status;      //		0x0F		   1
	volatile unsigned char  player_info[3];	    //		0x10		   3
	volatile unsigned char  p0_status;          //		0x13		   1
	volatile unsigned int   player_start;		//		0x14		   4
	volatile unsigned int   sfp0_npackets_sent;	//		0x18		   4
	volatile unsigned int   sfp1_npackets_sent;	//		0x1C		   4
	volatile unsigned int   sfp2_npackets_sent;	//		0x20		   4
	volatile unsigned int   sfp3_npackets_sent;	//		0x24		   4
	volatile unsigned int   p00_npackets_sent;	//		0x28		   4
	volatile unsigned int   p01_npackets_sent;	//		0x2C		   4
	volatile unsigned int   bpm0_npackets_sent;	//		0x30		   4
	volatile unsigned int   bpm1_npackets_sent;	//		0x34		   4
	volatile unsigned int   bpm2_npackets_sent;	//		0x38		   4
	volatile unsigned int   bpm3_npackets_sent;	//		0x3C		   4
	volatile unsigned short sfp1_los_counter;	//		0x40		   2
	volatile unsigned short sfp0_los_counter;	//		0x42		   2
	volatile unsigned short sfp3_los_counter;	//		0x44		   2
	volatile unsigned short sfp2_los_counter;	//		0x46		   2
	volatile unsigned short p01_los_counter;	//		0x48		   2
	volatile unsigned short p00_los_counter;	//		0x4A		   2
	volatile unsigned short bpm1_los_counter;	//		0x4C		   2
	volatile unsigned short bpm0_los_counter;	//		0x4E		   2
	volatile unsigned short bpm3_los_counter;	//		0x50		   2
	volatile unsigned short bpm2_los_counter;	//		0x52		   2

	volatile unsigned char pad54;				// 		0x54		   1
	volatile unsigned char trg_source;			// 		0x55		   1
	volatile unsigned char trg_mode;			// 		0x56		   1
	volatile unsigned char trg_enabled;			// 		0x57		   1
	volatile unsigned int bunch_delay;			// 		0x58		   4
	volatile unsigned short bucket_space;		// 		0x5C		   2
	volatile unsigned short bucket_number;		// 		0x5E		   2
	volatile unsigned char pad60[3];			// 		0x60		   3
	volatile unsigned char trg_rate;			// 		0x63		   1
	volatile unsigned int trg_once;			    // 		0x64		   4
	volatile unsigned char pad68[2];			// 		0x68		   2
	volatile unsigned char read_ready;			// 		0x6A		   1
	volatile unsigned char trg_ext_missing;		// 		0x6B		   1

	volatile unsigned short feedback_data0;		// 		0x6C		   2
	volatile unsigned short feedback_data1;		// 		0x6E		   2
	volatile unsigned short feedback_data2;		// 		0x70		   2
	volatile unsigned short feedback_data3;		// 		0x72		   2
	volatile unsigned short feedback_pkt_cnt;	// 		0x74		   2
	volatile unsigned short feedback_bad_cnt;	// 		0x76		   2
	volatile unsigned char bpm_id0;				// 		0x78		   1
	volatile unsigned char bpm_id1;				// 		0x79		   1
	volatile unsigned char bpm_id2;				// 		0x7A		   1
	volatile unsigned char bpm_id3;				// 		0x7B		   1
	volatile unsigned int FW_VERSION;			// 		0x7C		   4
	volatile float tr_sfp0_kx0[4];				// 		0x80		   4	//one parameter per BPM_ID
	volatile float tr_sfp0_kx1[4];				// 		0x90		   4	//one parameter per BPM_ID
	volatile float tr_sfp0_ky0[4];				// 		0xA0		   4	//one parameter per BPM_ID
	volatile float tr_sfp0_ky1[4];				// 		0xB0		   4	//one parameter per BPM_ID
	volatile float tr_sfp1_kx0[4];				// 		0xC0		   4	//one parameter per BPM_ID
	volatile float tr_sfp1_kx1[4];				// 		0xD0		   4	//one parameter per BPM_ID
	volatile float tr_sfp1_ky0[4];				// 		0xE0		   4	//one parameter per BPM_ID
	volatile float tr_sfp1_ky1[4];				// 		0xF0		   4	//one parameter per BPM_ID
	volatile float tr_sfp2_kx0[4];				// 		0x100		   4	//one parameter per BPM_ID
	volatile float tr_sfp2_kx1[4];				// 		0x110		   4	//one parameter per BPM_ID
	volatile float tr_sfp2_ky0[4];				// 		0x120		   4	//one parameter per BPM_ID
	volatile float tr_sfp2_ky1[4];				// 		0x130		   4	//one parameter per BPM_ID
	volatile float tr_sfp3_kx0[4];				// 		0x140		   4	//one parameter per BPM_ID
	volatile float tr_sfp3_kx1[4];				// 		0x150		   4	//one parameter per BPM_ID
	volatile float tr_sfp3_ky0[4];				// 		0x160		   4	//one parameter per BPM_ID
	volatile float tr_sfp3_ky1[4];				// 		0x170		   4	//one parameter per BPM_ID
	volatile float tr_p00_kx0[4];				// 		0x180		   4	//one parameter per BPM_ID
	volatile float tr_p00_kx1[4];				// 		0x190		   4	//one parameter per BPM_ID
	volatile float tr_p00_ky0[4];				// 		0x1A0		   4	//one parameter per BPM_ID
	volatile float tr_p00_ky1[4];				// 		0x1B0		   4	//one parameter per BPM_ID
	volatile float tr_p01_kx0[4];				// 		0x1C0		   4	//one parameter per BPM_ID
	volatile float tr_p01_kx1[4];				// 		0x1D0		   4	//one parameter per BPM_ID
	volatile float tr_p01_ky0[4];				// 		0x1E0		   4	//one parameter per BPM_ID
	volatile float tr_p01_ky1[4];				// 		0x1F0		   4	//one parameter per BPM_ID
};

struct structIBFBPlayer{
	volatile unsigned int *reg; //TODO ML84 should be removed
	struct structIBFBPlayerRegs *regs;
	volatile unsigned int *ram;
	struct structFunctionController FunctionController;
	struct structPlayerFunctions *functions;
	struct structPlayerCtrlMem *ctrl_mem;
	int (*ibfb_cmd)(void *object, int argc, char *argv[]);
	int (*init)(struct structIBFBPlayer *this);
};



int structIBFBPlayer_new(struct structIBFBPlayer *this,
		                 unsigned int reg_addr,
		                 unsigned int ram_addr,
		                 unsigned int func_reg_addr,
		                 unsigned int func_mem_addr,
		                 unsigned int func_mem_size,
		                 unsigned int ctrl_mem_addr);



#endif /* IBFB_PLAYER_H_ */
