/*
 * system.c
 *
 *  Created on: 15.08.2014
 *      Author: koprek_w
 */

#include "system.h"

void system_clrscr() {
	int i;

	for (i=1;i<80;i++)
		print("\n\r");

	return;
}

void system_hello(struct structSystem *this) {

	system_clrscr();

	xil_printf("GPAC 2.1, BPM%d FPGA\n\r", this->BPMService.serviceRegs->cfg_bpm_sel+1);
	print("European XFEL IBFB Player\n\n\r");

	return;
}

void system_set_bp_addresses(struct structSystem *this) {

	//unsigned int bpm = this->BPMService->serviceRegs->cfg_bpm_sel;
	//unsigned int reg_addr = SYSTEM_DAC16HL_1_ADDR + SYSTEM_DAC16HL_ADDR_SPACE * bpm + SYSTEM_DAC16HL_REG_OFFSET;
	//unsigned int spi_addr = SYSTEM_DAC16HL_1_ADDR + SYSTEM_DAC16HL_ADDR_SPACE * bpm + SYSTEM_DAC16HL_SPI_OFFSET;
	//unsigned int i2c_addr = SYSTEM_DAC16HL_1_ADDR + SYSTEM_DAC16HL_ADDR_SPACE * bpm + SYSTEM_DAC16HL_I2C_OFFSET;

	//this->DAC16HL->init_addresses(this->DAC16HL, reg_addr, i2c_addr, spi_addr);
	//this->IBFBCtrl->DAC16HL->DAC16HL->init_addresses(this->IBFBCtrl->DAC16HL->DAC16HL, reg_addr, i2c_addr, spi_addr);

	return;
}


void system_init_hardware(struct structSystem *this) {

    //Enable interrupts in RS232
    XUartLite_SetControlReg(SYSTEM_UART_BASEADDR, XUL_CR_ENABLE_INTR | XUL_CR_FIFO_TX_RESET);

    system_hello(this);

    print("Hardware initialization...");

    // init addresses for BP FPGA
    system_set_bp_addresses(this);
    // init interrupts
    this->Interrupt.init(&this->Interrupt);
    this->Interrupt.assign_handler(&this->Interrupt, XPAR_INTC_INST_UART_INST_INTERRUPT_INTR, &this->Shell, this->Shell.shell);
    this->Interrupt.assign_handler(&this->Interrupt, XPAR_INTC_INST_IBFB_FUNC_CTRL_CPU_INT_INTR, &this->IBFBPlayer.FunctionController, this->IBFBPlayer.FunctionController.interruptHandler);

	// register shell commands
    this->Shell.init(&this->Shell); //launch shell init function
	this->Shell.attach_cmd("help", &this->Shell, this->Shell.help);
	this->Shell.attach_cmd("version", &this->BPMService, this->BPMService.print_fw_version);
	this->Shell.attach_cmd("player", &this->IBFBPlayer, this->IBFBPlayer.ibfb_cmd);

    this->BPMService.init_version_registers(&this->BPMService);
    // GTX init
    this->IBFBPlayer.init(&this->IBFBPlayer);

    print("done\n\n\r");
    this->Interrupt.enableInterrupts();
	this->Shell.prompt(&this->Shell);

	return;
}

int structSystem_new(struct structSystem *this) {


	/***********************************************************************************
	 * Build up the system of software component and make callbacks between them
	 ***********************************************************************************/
	//create BPM service object
	structBPMService_new(&this->BPMService, SYSTEM_SHARED_DDR2_SERVICE, XPAR_BPM_SERVICE_BASEADDR);

	// interrupt handler
	structInterrupt_new(&this->Interrupt, XPAR_INTC_INST_BASEADDR, this);

	// Create QDR2 object
	//structQDR2_new(SYSTEM_QDR2_BASE_ADDR);

	// create shell
	structShell_new(&this->Shell, STDIN_BASEADDRESS);

	// create IBFB switch
	structIBFBPlayer_new(&this->IBFBPlayer,
			             XPAR_IBFB_PLAYER_0_BASEADDR,
			             XPAR_IBFB_PLAYER_0_MEM0_BASEADDR,
			             XPAR_IBFB_FUNC_CTRL_BASEADDR,
			             PLAYER_FUNC_CTRL_MEM0_BASEADDR,
			             PLAYER_FUNC_CTRL_MEM_SIZE,
			             PLAYER_CTRL_MEM_BASEADDR);

	// system methods
	this->init_hardware = system_init_hardware;

	return 0;
}


