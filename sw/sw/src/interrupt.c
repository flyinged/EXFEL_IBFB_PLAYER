/*
 * interrupt.c
 *
 *  Created on: 16.07.2012
 *      Author: koprek_w
 */

#include "interrupt.h"

void DeviceDriverHandler(void *CallbackRef)
{
	unsigned int i;
	struct structInterrupt *this = (struct structInterrupt *)CallbackRef;

	/* Enable FPU present in MBR
	 * The system interrupt handler disables FPU during interupt handling
	 * to avoid infinite loop of handling FPU exceptions
	 * In this case the FPU is needed in the user interrupt handlers
	 * therefore the FPU is enabled but the exceptions generated by FPU
	 * in the user interrupts routing are ignored
	 *
	 */
	mtmsr(mfmsr() | XREG_MSR_FLOATING_POINT_UNIT_ENABLE);

	/*
	 * Enable CPU busy counter
	 */
//	this->sys->bpmService->serviceRegs->cpu_busy_ena = 1;

	/*
	 * Call corresponding interrupt functions
	 */
	for (i=0;i<XPAR_INTC_INST_NUM_INTR_INPUTS;i++) {						// for each interrupt
		if ((this->deviceRegs->ipr & (1<<i)) > 0) {							// if pending interrupt
//			this->sys->bpmService->serviceRegs->served_int = i;				// for debugging, to check in which intterupt the PPC hangs
			if (this->interruptHandlers[i] != NULL)							// does interupt handler exist?
				this->interruptHandlers[i](this->interruptObjects[i]);		// call the user interrupt handler
			this->deviceRegs->iar = 1<<i;									// always acknnowledge even if the user handler does not exist
//			this->sys->bpmService->serviceRegs->served_int = 0;				// for debugging, to check in which intterupt the PPC hangs
		}
	}

	/*
	 * Disable CPU busy counter
	 */
//	this->sys->bpmService->serviceRegs->cpu_busy_ena = 0;

	return;
}

void interrupt_dummy_handler(struct structInterrupt *this) {


	return;
}

void interrupt_assign_handler(struct structInterrupt *this, unsigned int interrupt, void *object, void *function) {

	this->interruptHandlers[interrupt] = function;
	this->interruptObjects[interrupt] = object;

	return;
}

void interrupt_init(struct structInterrupt *this) {

	int i;

	/*
	 * Assign dummy handler to all interrupts
	 */
	for (i=0;i<XPAR_INTC_MAX_NUM_INTR_INPUTS;i++) {
		this->interruptHandlers[i] = interrupt_dummy_handler;
		this->interruptObjects[i] = this;
	}

	/*
	 * Enable interrupts for all devices that cause interrupts, and enable
	 * the INTC master enable bit.
	 */
	//XIntc_Out32(base_addr+XIN_IER_OFFSET, (1<<XPAR_INTC_MAX_NUM_INTR_INPUTS)-1);
	this->deviceRegs->ier = (1<<XPAR_INTC_MAX_NUM_INTR_INPUTS)-1;

	/*
	 * Set the master enable bit.
	 */
	//XIntc_Out32(base_addr + XIN_MER_OFFSET, XIN_INT_MASTER_ENABLE_MASK | XIN_INT_HARDWARE_ENABLE_MASK);
	this->deviceRegs->mer = XIN_INT_MASTER_ENABLE_MASK | XIN_INT_HARDWARE_ENABLE_MASK;


	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler)DeviceDriverHandler,
			this);

	return;
}

void enableInterrupts(){
	Xil_ExceptionEnable();
}

void disableInterrupts(){
	Xil_ExceptionDisable();
}

int structInterrupt_new(struct structInterrupt *this, unsigned int base_addr, struct structSystem *sys) {

	this->sys = sys;
	this->deviceRegs = (struct structInterruptRegs *)base_addr;
	this->enableInterrupts = enableInterrupts;
	this->disableInterrupts = disableInterrupts;
	this->init = interrupt_init;
	this->assign_handler = interrupt_assign_handler;

	return 0;
}
