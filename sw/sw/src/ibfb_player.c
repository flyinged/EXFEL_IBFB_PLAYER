/*
 * ibfb_switch.c
 *
 *  Created on: 08.01.2016
 *      Author: koprek_w
 */

#include "ibfb_player.h"
#include "shell_utility.h"

//#define RAM_SIZE 20
//#define NUM_RAMS 2
//const unsigned int ram[NUM_RAMS][RAM_SIZE] = {
//	{                                                   //time | CTRL | BPM |  BKT |     XPOS |     YPOS
//		0x00000010, 0x1A010010, 0x11110001, 0x22220001, //  10 |   1A |  01 | 0010 | 11100001 | 22220001
//		0x00000020, 0x1B010011, 0x11110002, 0x22220002, //  20 |   1B |  01 | 0011 | 11110002 | 22220002
//		0x00000030, 0x1C010012, 0x11110003, 0x22220003, //  30 |   1C |  01 | 0012 | 11110003 | 22220003
//		0x00000040, 0x1D010013, 0x11110004, 0x22220004, //  40 |   1D |  01 | 0013 | 11110004 | 22220004
//		0x00000000, 0xFF000000, 0x00000000, 0x00000000  //  00 |   end of stream
//	},{
//		0x00000010, 0x2A030010, 0x33330001, 0x44440001, //  10 |   2A |  03 | 0010 | 33330001 | 44440001
//		0x00000020, 0x2B030011, 0x33330002, 0x44440002, //  20 |   2B |  03 | 0011 | 33330002 | 44440002
//		0x00000030, 0x2C030012, 0x33330003, 0x44440003, //  30 |   2C |  03 | 0012 | 33330003 | 44440003
//		0x00000040, 0x2D030013, 0x33330004, 0x44440004, //  40 |   2D |  03 | 0013 | 33330004 | 44440004
//		0x00000000, 0xFF000000, 0x00000000, 0x00000000  //  00 |   end of stream
//	}
//};

/*
 * 0.5: added init command
 * 0.6: added ram generation command, added los counters support
 * 0.7: corrected display bug (P0 loss-of-sync)
 * 0.8: updated initialization procedure
 * 0.9: updated status display
 * 1.0: First release
 * 1.1: Support for new player RAM addressing
 * 1.2: Added information on Feedback link
 * 1.3: Changed RAM content initialization, added command to write transform parameters
 * 1.4: New RAM content: 4 different BPMs (1,2,3,4,1,..), changed initial delay and packet spacing
 * 1.5: Modified RAM init command to set first BPM_ID to use
 */
#define VMAJ 1
#define VMIN 5

const char * const cname[] = { "SFP0", "SFP1", "SFP2", "SFP3", "BPM0", "BPM1", "BPM2", "BPM3", "P0.0", "P0.1"};
const char * const losname[] = { "        SYNC", "      RESYNC", "   SYNC_LOST", "    BAD_CODE"};
const unsigned char CH[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

//int ibfb_switch_rxfifo_isempty(struct structIBFBSwitch *this) {
//
//	return ((this->regs->gtx[0].rx_status & IBFB_R_DF_EMPTY)>>17);
//}

/*
 * Display status of a selected GTX channel
 */
int ibfb_player_print_status(struct structIBFBPlayer *this) {

	unsigned int status_qb, lb_qb, status_p0, lb_p0, status_pl, addr_w, loscnt0, loscnt1, rst_vec;

	rst_vec   = this->reg[REG_RST];
	status_qb = this->reg[REG_MGT_QB];
	status_p0 = this->reg[REG_MGT_P0];
	lb_qb     = this->reg[REG_LB_QB];
	lb_p0     = this->reg[REG_LB_P0];
	status_pl = this->reg[REG_PL_START];

	//tile lock
	print("\n\r");
	xil_printf("Using %s clock\n\r", (status_p0 & REG_USE_EXT_CLK)?"external":"internal");
	xil_printf("Internal trigger %s\n\r", (rst_vec & REG_INT_TRIG_EN)?"enabled":"disabled");
	xil_printf("External trigger delay = 0x%08X\n\r", this->reg[REG_XTRIG_DELAY]);
	xil_printf("MGT reset     = 0x%03X\n\r", (rst_vec & 0x3FF));
	xil_printf("LOS CNT reset = 0x%03X\n\r\n", ((rst_vec>>12) & 0x3FF));
	print("LOCK : QSFP13 | QSFP02 | BPM01 | BPM23 | P0\n\r");
	xil_printf("            %d |      %d |     %d |     %d |  %d\n\r", \
			(status_qb&REG_MGT_Q13_LOCK)?1:0, \
			(status_qb&REG_MGT_Q02_LOCK)?1:0, \
			(status_qb&REG_MGT_B01_LOCK)?1:0, \
			(status_qb&REG_MGT_B23_LOCK)?1:0,  \
			(status_p0&REG_MGT_P0_LOCK)?1:0  \
			);
	xil_printf("RX SYNC (4BPM,2P0,4QSFP) = 0x%03X\n\r", ((status_pl >> 10) & 0x3FF) );
	//Per channel
	print("\n\r");
	print("CHAN  : RST_DONE | LOSS OF SYNC | LOOPBACK\n\r");
	xil_printf("QSFP0 :        %d | %s |        %d%s\n\r", \
			(status_qb&REG_Q0_RSTD)?1:0,
			losname[(status_qb&REG_Q0_LOS)>>14],
			(lb_qb&REG_LB_Q0),
			(lb_qb&REG_LB_Q0)?"*":""
	);
	xil_printf("QSFP1 :        %d | %s |        %d%s\n\r", \
			(status_qb&REG_Q1_RSTD)?1:0,
			losname[(status_qb&REG_Q1_LOS)>>6],
			(lb_qb&REG_LB_Q1)>>4,
			(lb_qb&REG_LB_Q1)?"*":""
	);
	xil_printf("QSFP2 :        %d | %s |        %d%s\n\r", \
			(status_qb&REG_Q2_RSTD)?1:0,
			losname[(status_qb&REG_Q2_LOS)>>12],
			(lb_qb&REG_LB_Q2)>>8,
			(lb_qb&REG_LB_Q2)?"*":""
	);
	xil_printf("QSFP3 :        %d | %s |        %d%s\n\r", \
			(status_qb&REG_Q3_RSTD)?1:0,
			losname[(status_qb&REG_Q3_LOS)>>4],
			(lb_qb&REG_LB_Q3)>>12,
			(lb_qb&REG_LB_Q3)?"*":""
	);
	xil_printf("P0.0  :        %d | %s |        %d%s\n\r", \
			(status_p0&REG_P00_RSTD)?1:0,
			losname[(status_p0&REG_P00_LOS)>>4],
			(lb_p0&REG_LB_P0_0),
			(lb_p0&REG_LB_P0_0)?"*":""
	);
	xil_printf("P0.1  :        %d | %s |        %d%s\n\r", \
			(status_p0&REG_P01_RSTD)?1:0,
			losname[(status_p0&REG_P01_LOS)>>6],
			(lb_p0&REG_LB_P0_1)>>4,
			(lb_p0&REG_LB_P0_1)?"*":""
	);
	xil_printf("BPM0  :        %d | %s |        %d%s\n\r", \
			(status_qb&REG_B0_RSTD)?1:0,
			losname[(status_qb&REG_B0_LOS)>>20],
			(lb_qb&REG_LB_B0)>>16,
			(lb_qb&REG_LB_B0)?"*":""
	);
	xil_printf("BPM1  :        %d | %s |        %d%s\n\r", \
			(status_qb&REG_B1_RSTD)?1:0,
			losname[(status_qb&REG_B1_LOS)>>22],
			(lb_qb&REG_LB_B1)>>20,
			(lb_qb&REG_LB_B1)?"*":""
	);
	xil_printf("BPM2  :        %d | %s |        %d%s\n\r", \
			(status_qb&REG_B2_RSTD)?1:0,
			losname[(status_qb&REG_B2_LOS)>>28],
			(lb_qb&REG_LB_B2)>>24,
			(lb_qb&REG_LB_B2)?"*":""
	);
	xil_printf("BPM3  :        %d | %s |        %d%s\n\r", \
			(status_qb&REG_B3_RSTD)?1:0,
			losname[(status_qb&REG_B3_LOS)>>30],
			(lb_qb&REG_LB_B3)>>28,
			(lb_qb&REG_LB_B3)?"*":""
	);
	/* Player info */
	print("\n\r");
	xil_printf("Enabled players: 0x%03X\n\r", (status_p0 & REG_PL_EN_ALL)>>8);
	xil_printf("End of sequence: 0x%02X\n\r", (status_pl & REG_PL_CTRL_EOS)>>24);
	addr_w = ((status_p0 & REG_PL_RAM_AW) >> 24);
	xil_printf("Ram depth      : %d dwords (%d packets)\n\r", (1<<addr_w), (1<<addr_w)/4);

	print("\n\rLOSS OS SYNC COUNTERS:\n\r");

	loscnt0  = this->reg[REG_LCNT_Q10];
	loscnt1  = (loscnt0>>16) & 0xFFFF;
	loscnt0 &= 0xFFFF;
	xil_printf("    SFP0: 0x%04X %s\n\r", loscnt0, loscnt0?"*":"");
	xil_printf("    SFP1: 0x%04X %s\n\r", loscnt1, loscnt1?"*":"");
	loscnt0  = this->reg[REG_LCNT_Q32];
	loscnt1  = (loscnt0>>16) & 0xFFFF;
	loscnt0 &= 0xFFFF;
	xil_printf("    SFP2: 0x%04X %s\n\r", loscnt0, loscnt0?"*":"");
	xil_printf("    SFP3: 0x%04X %s\n\r", loscnt1, loscnt1?"*":"");
	loscnt0  = this->reg[REG_LCNT_P10];
	loscnt1  = (loscnt0>>16) & 0xFFFF;
	loscnt0 &= 0xFFFF;
	xil_printf("    P0_0: 0x%04X %s\n\r", loscnt0, loscnt0?"*":"");
	xil_printf("    P0_1: 0x%04X %s\n\r", loscnt1, loscnt1?"*":"");
	loscnt0  = this->reg[REG_LCNT_B10];
	loscnt1  = (loscnt0>>16) & 0xFFFF;
	loscnt0 &= 0xFFFF;
	xil_printf("    BPM0: 0x%04X %s\n\r", loscnt0, loscnt0?"*":"");
	xil_printf("    BPM1: 0x%04X %s\n\r", loscnt1, loscnt1?"*":"");
	loscnt0  = this->reg[REG_LCNT_B32];
	loscnt1  = (loscnt0>>16) & 0xFFFF;
	loscnt0 &= 0xFFFF;
	xil_printf("    BPM2: 0x%04X %s\n\r", loscnt0, loscnt0?"*":"");
	xil_printf("    BPM3: 0x%04X %s\n\r", loscnt1, loscnt1?"*":"");

	print("\n\r");


	return 0;
}

void ibfb_player_init_memory(struct structIBFBPlayer *this) {

	int i;

	for (i=0; i<IBFB_PLAYER_MEM_ELEMS; i++) {
		this->ctrl_mem->timestamp[i] = 0;
		this->ctrl_mem->control[i] = REG_PL_CTRL_EOS;
		this->ctrl_mem->x[i] = 0.0;
		this->ctrl_mem->y[i] = 0.0;
	}

	return;
}

void ibfb_player_update_player_memory(struct structIBFBPlayer *this, const unsigned int cmd) {

	unsigned char mem = cmd & 0xFF;
	int i, eop=0;
	struct structPlayerCtrlMem *player_mem;

	// check if valid memory was selected
	if (mem>5)
		return;

	//xil_printf("\n\rSelected memory: %d\n\r", mem);

	player_mem = (struct structPlayerCtrlMem *)(this->ram + mem*IBFB_PLAYER_MEM_ELEMS*4); // IBFB_PLAYER_MEM_ELEMS * four_memories
	//xil_printf("mem %d: 0x%08X\n\r", mem, player_mem);

	//ibfb_player_init_memory(this);

	for (i=0; i<IBFB_PLAYER_MEM_ELEMS-1; i++) {
		if ((this->ctrl_mem->control[i] == REG_PL_CTRL_EOS) || (eop)) {
			eop=1;
			player_mem->timestamp[i] = 0;
			player_mem->control[i] = REG_PL_CTRL_EOS;
			player_mem->x[i] = 0.0;
			player_mem->y[i] = 0.0;
		}
		else {
			player_mem->timestamp[i] = this->ctrl_mem->timestamp[i];
			player_mem->control[i] = this->ctrl_mem->control[i];
			player_mem->x[i] = this->ctrl_mem->x[i];
			player_mem->y[i] = this->ctrl_mem->y[i];
		}
	}

	// the last element must be always terminating frame
	player_mem->timestamp[IBFB_PLAYER_MEM_ELEMS-1] = 0;
	player_mem->control[IBFB_PLAYER_MEM_ELEMS-1] = REG_PL_CTRL_EOS;
	player_mem->x[IBFB_PLAYER_MEM_ELEMS-1] = 0.0;
	player_mem->y[IBFB_PLAYER_MEM_ELEMS-1] = 0.0;

	return;
}



void ibfb_player_init_player_memory(struct structIBFBPlayer *this) {

	int i;

	ibfb_player_init_memory(this);

	for (i=0;i<6;i++)
		ibfb_player_update_player_memory(this, i);

	return;
}

/*
 * Resets all GTX tiles
 */
int ibfb_player_mgt_init(struct structIBFBPlayer *this) {

	unsigned int reg;

	// reset all MGTs
	reg = this->reg[REG_RST]; //store previous value

	this->reg[REG_RST] = 0;
	this->reg[REG_RST] = REG_RST_ALL;

	this->reg[REG_RST] = reg; //restore previous value
	print("\n\rReset all MGTs and counters\n\r");

	// KW84 - init memories
	ibfb_player_init_player_memory(this);

	return 0;
}


/*
 * Set loopback value for selected GTX channel
 */
int ibfb_player_set_loopback(struct structIBFBPlayer *this, char chan, unsigned long data) {
	unsigned long reg;

	if (chan > 9) {
		return 1;
	}

	if (data == 3 || data == 5 || data > 6) {
		return 2;
	}

	//get current loopback
	if (chan == 4 || chan == 5) {
		reg = this->reg[REG_LB_P0];
		//xil_printf("Current loopback register (P0) = 0x%08X\n\r", reg);
	} else {
		reg = this->reg[REG_LB_QB];
		//xil_printf("Current loopback register (QSFP/BPM) = 0x%08X\n\r", reg);
	}

	switch (chan) {
	case 0:
		//reset current value
		reg &= ~REG_LB_Q0;
		//set new value
		reg |= data;
		break;
	case 1:
		//reset current value
		reg &= ~REG_LB_Q1;
		//set new value
		reg |= data<<4;
		break;
	case 2:
		//reset current value
		reg &= ~REG_LB_Q2;
		//set new value
		reg |= data<<8;
		break;
	case 3:
		//reset current value
		reg &= ~REG_LB_Q3;
		//set new value
		reg |= data<<12;
		break;
	case 6:
		//reset current value
		reg &= ~REG_LB_B0;
		//set new value
		reg |= data<<16;
		break;
	case 7:
		//reset current value
		reg &= ~REG_LB_B1;
		//set new value
		reg |= data<<20;
		break;
	case 8:
		//reset current value
		reg &= ~REG_LB_B2;
		//set new value
		reg |= data<<24;
		break;
	case 9:
		//reset current value
		reg &= ~REG_LB_B3;
		//set new value
		reg |= data<<28;
		break;
	case 4:
		//reset current value
		reg &= ~REG_LB_P0_0;
		//set new value
		reg |= data;
		break;
	case 5:
		//reset current value
		reg &= ~REG_LB_P0_1;
		//set new value
		reg |= data<<4;
		break;
	default:
		return 1;
	}

	//set new loopback
	if (chan == 4 || chan == 5) {
		this->reg[REG_LB_P0] = reg;
		xil_printf("%d: New loopback register (P0) = 0x%08X\n\r", chan, this->reg[REG_LB_P0]);
	} else {
		this->reg[REG_LB_QB] = reg;
		xil_printf("%d: New loopback register (QSFP/BPM) = 0x%08X\n\r", chan, this->reg[REG_LB_QB]);
	}

	return 0;
}

#define PRAM_NEW_ADDR
#define TEST_XPOS  1.0f
#define TEST_YPOS  1.0f
#define FIRST_PKT_DELAY 65556
#define PKT_SPACING 48 //FIXME TBD

int ibfb_player_ramgen(struct structIBFBPlayer *this, char chan, unsigned long npackets, unsigned long first_bpm) {

	unsigned char addr_w;
	unsigned long dword, ctrl, time, p, data, *x, *y, bpm;
	float xf, yf;

	if (chan > 9) {
		print("Bad parameters: channel shall be in range [0:9]\n\r");
		return 1;
	}

	if (first_bpm > 255) {
		print("Bad parameters: first_bpm shall be in range [0:255]\n\r");
		return 1;
	}

	xf = TEST_XPOS;
	yf = TEST_YPOS;

	x = (unsigned long *)&xf;
	y = (unsigned long *)&yf;
	xil_printf("Xpos = 0x%08X, Ypos = 0x%08X\n\r", *x, *y);

	addr_w = ((this->reg[REG_MGT_P0] & REG_PL_RAM_AW) >> 24); //get RAM address width (BYTE address)
	dword  = chan << (addr_w-2); //compute DWORD offset (byte offset / 4)

	//max number of packets = 2^(ADDR_W-4)-1
	if ((npackets+1) > (1<<(addr_w-4))) {
		xil_printf("ERROR: maximum allowed number of packets is %d\n\r", (1<<(addr_w-4))-1);
		return 1;
	}


	time   = FIRST_PKT_DELAY; //timestamp
	bpm    = first_bpm;
	ctrl   = chan; //CTRL = channel number

#ifdef PRAM_NEW_ADDR
	xil_printf("    Writing timestamps to byte offset 0x%08X\n\r", dword<<2);
	for (p=0; p<npackets; p++) {
		//timestamp
		this->ram[dword++] = time;
		time += PKT_SPACING;
	}
	this->ram[dword++] = 0x00000000; //dummy timestamp

	dword  = chan << (addr_w-2); //compute DWORD offset (byte offset / 4)
	dword |= (1 << (addr_w-4)); //Player RAM's section 1 (contains CTRL/BPM/BKT fields)
	xil_printf("    Writing CTRL,BPM,BKT to byte offset 0x%08X\n\r", dword<<2);
	dword  = ( (chan << (addr_w-2)) | (1 << (addr_w-4)) ); //compute DWORD offset for CTRL,BPM,PKT dword
	for (p=0; p<npackets; p++) {
		//ctrl, bpm, bkt
		this->ram[dword++] = (ctrl<<24) | (bpm<<16) | (p&0xFFFF);
		if (bpm == (first_bpm+3)) {
			bpm = first_bpm;
		} else {
			bpm++;
		}
	}
	data = (this->reg[REG_PL_START] & REG_PL_CTRL_EOS); //end of stream code
	this->ram[dword++] = data;

	dword  = chan << (addr_w-2); //compute DWORD offset (byte offset / 4)
	dword |= (2 << (addr_w-4)); //Player RAM's section 2 (contains XPOS field)
	xil_printf("    Writing XPOS to byte offset 0x%08X\n\r", dword<<2);
	dword  = ( (chan << (addr_w-2)) | (2 << (addr_w-4)) ); //compute DWORD offset for XPOS dword
	for (p=0; p<npackets; p++) {
		//xpos
		this->ram[dword++] = *x; //p+0x11110000;
	}

	dword  = chan << (addr_w-2); //compute DWORD offset (byte offset / 4)
	dword |= (3 << (addr_w-4)); //Player RAM's section 2 (contains XPOS field)
	xil_printf("    Writing YPOS to byte offset 0x%08X\n\r", dword<<2);
	dword  = ( (chan << (addr_w-2)) | (3 << (addr_w-4)) ); //compute DWORD offset for YPOS dword
	for (p=0; p<npackets; p++) {
		//ypos
		this->ram[dword++] = *y; //p+0x22220000;
	}

#else
	for (p=0; p<npackets; p++) {
		//timestamp
		this->ram[dword++] = time;
		time += 0x10;
		//ctrl, bpm, bkt
		this->ram[dword++] = (ctrl<<24) | (bpm<<16) | (p&0xFFFF);
		//xpos, ypos
		this->ram[dword++] = p+0x11110000;
		this->ram[dword++] = p+0x22220000;
	}
	//end of sequence
	this->ram[dword++] = 0x00000000; //dummy timestamp
	data = (this->reg[REG_PL_START] & REG_PL_CTRL_EOS); //end of stream code
	this->ram[dword++] = data;
#endif

	dword  = chan << addr_w; //compute DWORD offset (byte offset / 4)
	xil_printf("Wrote %d packets to RAM (byte offset 0x%08X)\n\r", npackets, dword);

	return 0;
}



/*
 * Execute shell commands
 */

int ibfb_player_ibfb_cmd(void *object, int argc, char *argv[]){

	char cmd;
	unsigned char chan, addr_w, *cptr;
	unsigned long data, offset, n, dword;
	//int i;
	struct structIBFBPlayer *this = (struct structIBFBPlayer*)(object);

	if (argc>1)
		cmd = argv[1][0];
	else //ibfb == 1
		cmd = 'h';
		//return -1;

	switch (cmd) {
	//case 'h': //print help
		//default
	case '0': /* RESET */
		ibfb_player_mgt_init(this);
		break;
	case '1': //print all status data
		print("RAW REGISTER MAP\n\r");
		//     N   : 0x 00.00.00.00 (0x00000000 0x00000000)
		for (dword = 0; dword < NREGS; dword++) {
			xil_printf("  0x%02X (%02d): 0x%08X\n\r", 4*dword, dword, this->reg[dword]);
		}
		break;
	case '2':
		print("\n\rRAM write\n\r");
		//arbitrary write ram
		if (argc < 4) {
			print("player 2 RAM_BYTE_OFFSET DATA");
		} else {
			offset = strtol(argv[2], NULL, 0);
			data   = strtol(argv[3], NULL, 0);
			xil_printf("A:0x%08X <= D:0x%08X\n\r", offset, data);
			this->ram[(offset/4)] = data;
		}
		break;
	case '3':
		print("\n\rRAM read\n\r");
		//arbitrary read ram
		if (argc < 3) {
			print("player 3 RAM_BYTE_OFFSET");
		} else {
			offset = strtol(argv[2], NULL, 0);
			xil_printf("A:0x%08X => D:0x%08X (dword %d)\n\r", offset, this->ram[offset/4], offset/4);
		}
		break;
	case 'a': /* DISPLAY STATUS */
		ibfb_player_print_status(this);
		break;
	case 'b': /* set bpm IDs for feeback */
		if (argc < 3) {
			print("player b 0xB0B1B2B3\n\r");
		} else {
			data   = strtol(argv[2], NULL, 0);
			cptr = (unsigned char*) &data;
			this->regs->bpm_id0 = cptr[0];
			this->regs->bpm_id1 = cptr[1];
			this->regs->bpm_id2 = cptr[2];
			this->regs->bpm_id3 = cptr[3];
			xil_printf("\n\rSet BPM_IDs as following: 0x %01X %02X %02X %02X\n\r", \
					this->regs->bpm_id0, \
					this->regs->bpm_id1, \
					this->regs->bpm_id2, \
					this->regs->bpm_id3
			);
		}
	    break;
	case 'c': /* reset LOS counters */
		//read-modify-write
		data = this->reg[REG_RST];
		this->reg[REG_RST] = data | REG_RST_LCNT_ALL; //reset
		this->reg[REG_RST] = data & (~REG_RST_LCNT_ALL); //release reset
		print("Loss-of-Sync counters reset\n\n\r");
		break;
	case 'd': //set external trigger delay
		if (argc<3) {
			print("\n\rUSAGE:\n\r");
			print("    player d DELAY\n\r");
		} else {
			data   = strtol(argv[2], NULL, 0);
			this->reg[REG_XTRIG_DELAY] = data;
			xil_printf("Set external trigger delay to 0x%08X clock cycles\n\r", data);
		}
		break;
	case 'f': //feedback info
		print("\n\r*** FEEDBACK INFORMATION ***\n\r");
		xil_printf("    Feedback packets received: %d\n\r", this->regs->feedback_pkt_cnt);
		xil_printf("    Bad packets received     : %d\n\r", this->regs->feedback_bad_cnt);
		xil_printf("    Current BPM IDs          : 0x %02X %02X %02X %02X",
				this->regs->bpm_id0, \
				this->regs->bpm_id1, \
				this->regs->bpm_id2, \
				this->regs->bpm_id3
				);
		break;
	case 'g': //generate RAM content
		if (argc<5) {
			print("\n\rUSAGE:\n\r");
			print("    player g CHAN NPACKETS FIRST_BPM_ID\n\r");
		} else {
			chan   = strtol(argv[2], NULL, 0);
			data   = strtol(argv[3], NULL, 0);
			n      = strtol(argv[4], NULL, 0);
			ibfb_player_ramgen(this, chan, data, n);
		}
		break;
	case 'i': /* init */
		if (argc<3) {
			print("\n\rERROR: Number of packets not specified.\n\r");
			break;
			n = 256;
		} else {
			n = strtol(argv[2], NULL, 0);
		}

		data = this->reg[REG_MGT_P0];
		data = ((data & REG_PL_EN_ALL)>>8); //enabled players

		//generate RAM data for all enabled players, loopback disabled channels
		print(">>> Setting BPM ID = CHANNEL ID <<<\n\r");
		for (chan = 0; chan<10; chan++) {
			if ((data>>chan)&0x1) {
				xil_printf("CH %d ON : ", chan);
				ibfb_player_ramgen(this, chan, n, chan);
			} else {
				xil_printf("CH %d off: ", chan);
				//ibfb_player_set_loopback(this, chan, 1); //near-end PCS
			}
		}

		//reset counters
		data = this->reg[REG_RST];
		this->reg[REG_RST] = data | REG_RST_LCNT_ALL; //reset
		this->reg[REG_RST] = data & (~REG_RST_LCNT_ALL); //release reset
		print("Loss-of-Sync counters reset\n\n\r");

		break;
	case 'k': /* SET LOOPBACK */
		if (argc < 4) {
			print("\n\rUSAGE:\n\r");
			print("    player b CHANNEL MODE\n\r");
			print("    MODE: loopback mode\n\r");
			print("        0: no loopback\n\r");
			print("        1: near-end PCS\n\r");
			print("        2: near-end PMA\n\r");
			print("        4: far-end PMA\n\r");
			print("        6: far-end PCS\n\r");
			print("\n\r");
		} else {
			chan = strtol(argv[2], NULL, 0);
			data = strtol(argv[3], NULL, 0);
			if (ibfb_player_set_loopback(this, chan, data))
				print("Bad paramaters\n\r");
		}
		break;

	case 'r': /* DEBUG: read from RAM */
		if (argc<4) {
			print("\n\rUSAGE:\n\r");
			print("    player r CHAN NUMBER_OF_DWORDS\n\r");
		} else {
			chan = strtol(argv[2], NULL, 0);
			if (chan > 9) {
				print("Bad parameters: allowed range [0:9]\n\r");
				return 1;
			}

			n    = strtol(argv[3], NULL, 0);

			addr_w = ((this->reg[REG_MGT_P0] & REG_PL_RAM_AW) >> 24); //address width
			offset = chan << addr_w; //byte offset
			xil_printf("Reading %d data from Player-RAM %d (offset 0x%08X)\n\r", n, chan, offset);
			for (dword = 0; dword < n; dword++) {
				xil_printf("    A:0x%08X => D:0x%08X\n\r", ((4*dword)+offset), this->ram[dword+(offset/4)]);
			}
		}
		break;
	case 's': /* start player */
		if (argc<3) {
			print("\n\rUSAGE:\n\r");
			print("    player s CHANNEL_MASK\n\r");
		} else {
			xil_printf("Starting players. Mask = 0x%03X\n\r", strtol(argv[2], NULL, 0));
			data = this->reg[REG_RST]; //store current status of reset vector
			this->reg[REG_RST] = data | REG_INT_TRIG_EN; //enable internal trigger if not
			this->reg[REG_PL_START] = 0;
			this->reg[REG_PL_START] = strtol(argv[2], NULL, 0); //send start
			this->reg[REG_PL_START] = data; //restore previous state
			//show sent packets counter
			print("Sent packets:\n\r");
			for (n=0; n<9; n++) {
				if ( (1<<n) & strtol(argv[2], NULL, 0)) {
					xil_printf("    CH%d: %08X\n\r", n, this->reg[REG_PL_SENT_Q0+n]);
				}
			}
		}
		break;
	case 't': /* toggle external trigger enable */
		if (argc<3) {
			print("\n\rUSAGE:\n\r");
			print("    player t INT_TRIG_EN\n\r");
		} else {
			n = strtol(argv[2], NULL, 0); //get selection
			data = this->reg[REG_RST]; //get previous value
			if (n) { //enable
				this->reg[REG_RST] = (data | REG_INT_TRIG_EN);
				print("Internal trigger is now ENABLED\n\r");
			} else {
				this->reg[REG_RST] = (data & ~REG_INT_TRIG_EN);
				print("Internal trigger is now DISABLED\n\r");
			}
		}
		break;
	case 'x': /* write transform values for feedback correction */
		if (argc<3) {
			print("\n\rUSAGE:\n\r");
			print("    player x CHAN\n\r");
		} else {
			chan = strtol(argv[2], NULL, 0); //get channel id
			xil_printf("Setting kx0 = kx1 = ky0 = ky1 = 1.0 for channel %d and bpm(0)\n\r", chan);
			switch(chan){
			case 0:
				this->regs->tr_sfp0_kx0[0] = 1.0;
				this->regs->tr_sfp0_kx1[0] = 1.0;
				this->regs->tr_sfp0_ky0[0] = 1.0;
				this->regs->tr_sfp0_ky1[0] = 1.0;


				this->regs->tr_sfp0_kx0[1] = 0.0;
				this->regs->tr_sfp0_kx1[1] = 0.0;
				this->regs->tr_sfp0_ky0[1] = 0.0;
				this->regs->tr_sfp0_ky1[1] = 0.0;
				this->regs->tr_sfp0_kx0[2] = 0.0;
				this->regs->tr_sfp0_kx1[2] = 0.0;
				this->regs->tr_sfp0_ky0[2] = 0.0;
				this->regs->tr_sfp0_ky1[2] = 0.0;
				this->regs->tr_sfp0_kx0[3] = 0.0;
				this->regs->tr_sfp0_kx1[3] = 0.0;
				this->regs->tr_sfp0_ky0[3] = 0.0;
				this->regs->tr_sfp0_ky1[3] = 0.0;
				break;

			default:
				print("ERROR: Only channel 0 is currently supported\n\r");
			} //end case
		}
		break;
	case 'n':
		print("\n\rSent packets:\n\r");
		xil_printf("    SFP0: %08X\n\r", this->reg[REG_PL_SENT_Q0]);
		xil_printf("    SFP1: %08X\n\r", this->reg[REG_PL_SENT_Q1]);
		xil_printf("    SFP2: %08X\n\r", this->reg[REG_PL_SENT_Q2]);
		xil_printf("    SFP3: %08X\n\r", this->reg[REG_PL_SENT_Q3]);
		xil_printf("    P0.0: %08X\n\r", this->reg[REG_PL_SENT_P00]);
		xil_printf("    P0.1: %08X\n\r", this->reg[REG_PL_SENT_P01]);
		xil_printf("    BPM0: %08X\n\r", this->reg[REG_PL_SENT_B0]);
		xil_printf("    BPM0: %08X\n\r", this->reg[REG_PL_SENT_B1]);
		xil_printf("    BPM0: %08X\n\r", this->reg[REG_PL_SENT_B2]);
		xil_printf("    BPM0: %08X\n\r", this->reg[REG_PL_SENT_B3]);
		break;
	default: /* HELP */
		xil_printf("\n\rIBFB Player Commands (Version %d.%d, FW 0x%X)\n\r", VMAJ, VMIN, this->regs->FW_VERSION);
		print("    h - print this help\n\r");
		print("    0 - reset all GTX tiles\n\r");
		print("    1 - print all registers\n\r");
		print("    2 - arbitrary ram write\n\r");
		print("    3 - arbitrary ram read\n\r");
		print("    a - Print player's status\n\r");
		print("    b - Set BPM_ID for feedback correction\n\r");
		print("    c - Reset LOS counters\n\r");
		print("    d - Set external trigger delay\n\r");
		print("    f - Display feedback infos\n\r");
		print("    g - generate RAM content\n\r");
		print("    i - self initialize (write RAMs, set loopbacks)\n\r");
		print("    k - set loopback mode\n\r");
		print("    n - show number of sent packets\n\r");
		print("    r - read data from RAM\n\r");
		print("    s - start players\n\r");
		print("    t - set external trigger enable\n\r");
		print("    x - set correction transform values\n\r");
		print("\n\rChannel Numbering (9:0): BPM(3,2,1,0); P0(1,0); QSFP(3,2,1,0)\n\r");
		print("\n\r");

	}

	return 0;
}

int ibfb_player_init_functions(struct structIBFBPlayer *this) {

	// init functions
	this->functions->trg_mode = 0;
	this->functions->trg_source = 0;
	this->functions->bunch_delay = 65535;
	this->functions->bunch_number = 5;
	this->functions->bunch_space = 1;

	return 0;
}

int ibfb_player_update_timing(struct structIBFBPlayer *this) {

	this->regs->bunch_delay = this->functions->bunch_delay;
	this->regs->bucket_number = IBFB_CTRL_XFEL_BUCKET_NUMBER;
	this->regs->bucket_space = IBFB_CTRL_XFEL_BUCKET_SPACE;
	this->regs->trg_enabled = 1;
	this->regs->trg_mode = this->functions->trg_mode;
	if (this->functions->trg_source)
		this->regs->trg_source = 1;		// 1 - auto
	else
		this->regs->trg_source = 0;		// 0 - external
	this->regs->trg_rate = this->functions->trg_source;

	return 0;
}

void ibfb_player_function_process(void *funcUserObject, unsigned int function) {

	struct structIBFBPlayer *this = (struct structIBFBPlayer *)funcUserObject;
	//unsigned char idx;

	switch (function*4) {
	case 0x000 :
		ibfb_player_update_player_memory(this, this->functions->player_cmd);
		break;
	case 0x004 :
	case 0x008 :
	case 0x00C :
	case 0x048:
		ibfb_player_update_timing(this);
		break;
	}

	return;
}

int ibfb_player_init(struct structIBFBPlayer *this) {

	ibfb_player_mgt_init(this);
	ibfb_player_init_functions(this);
	ibfb_player_update_timing(this);

	return 0;
}


/*
 * Initialize IBFB data structure
 */
int structIBFBPlayer_new(struct structIBFBPlayer *this,
		                 unsigned int reg_addr,
		                 unsigned int ram_addr,
		                 unsigned int func_reg_addr,
		                 unsigned int func_mem_addr,
		                 unsigned int func_mem_size,
		                 unsigned int ctrl_mem_addr) {

	//objects
	structFunctionController_new(&this->FunctionController, func_reg_addr, func_mem_addr, func_mem_size);

	this->FunctionController.userCallback = ibfb_player_function_process;
	this->FunctionController.userObject = this;
	this->FunctionController.interruptInit(&this->FunctionController, IBFB_CTRL_FUNC_NUM_OF_INT_REGS);

	this->functions = (struct structPlayerFunctions *)func_mem_addr;

	// properties
	this->reg = (volatile unsigned int *)reg_addr; //TODO ML84 should be removed
	this->regs = (struct structIBFBPlayerRegs *)reg_addr;
	this->ram = (volatile unsigned int *)ram_addr;
	this->ctrl_mem = (struct structPlayerCtrlMem *)ctrl_mem_addr;

	// methods
	this->ibfb_cmd = ibfb_player_ibfb_cmd;
	this->init = ibfb_player_init;

	//
	ibfb_player_init_memory(this);

	return 0;
}



