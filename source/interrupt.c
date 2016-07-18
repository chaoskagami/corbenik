#include <common.h>
#include <ctr9/ctr_interrupt.h>
#include <ctr9/ctr_irq.h>

void dump_state_printf(uint32_t* regs) {
	fprintf(stderr, "  cpsr:%x sp:%x lr:%x\n"
                    "  r0:%x r1:%x r2:%x r3:%x\n"
                    "  r4:%x r5:%x r6:%x r7:%x\n"
                    "  r8:%x r9:%x r10:%x r11:%x\n"
                    "  r12:%x\n",
                    (unsigned int)regs[0], (unsigned int)regs[1], (unsigned int)regs[2],
                    (unsigned int)regs[3], (unsigned int)regs[4], (unsigned int)regs[5], (unsigned int)regs[6],
                    (unsigned int)regs[7], (unsigned int)regs[8], (unsigned int)regs[9], (unsigned int)regs[10],
                    (unsigned int)regs[11], (unsigned int)regs[12], (unsigned int)regs[13], (unsigned int)regs[14],
                    (unsigned int)regs[15]);
}

void reset_INT(_UNUSED uint32_t* regs) {
	fprintf(stderr, "Reset called.\n");
}

void undef_INT(uint32_t* regs) {
	fprintf(stderr, "Undefined instruction.\n");
	dump_state_printf(regs);
	abort("Cannot continue. Halting.\n");
}

void swi_INT(_UNUSED uint32_t* regs) {
	fprintf(stderr, "SWI called. Returning.\n");
}

void preabrt_INT(uint32_t* regs) {
	fprintf(stderr, "Prefetch Abort.\n");
	dump_state_printf(regs);
	abort("Cannot continue. Halting.\n");
}

void databrt_INT(uint32_t* regs) {
	fprintf(stderr, "Data abort.\n");
	dump_state_printf(regs);
	abort("Cannot continue. Halting.\n");
}

void fiq_INT(_UNUSED uint32_t* regs) {
	fprintf(stderr, "FIQ called. Returning.\n");
}


void install_interrupts() {
	ctr_interrupt_prepare();
    ctr_irq_initialize();

    ctr_interrupt_set(CTR_INTERRUPT_RESET,   reset_INT);
    ctr_interrupt_set(CTR_INTERRUPT_UNDEF,   undef_INT);
    ctr_interrupt_set(CTR_INTERRUPT_SWI,     swi_INT);
    ctr_interrupt_set(CTR_INTERRUPT_PREABRT, preabrt_INT);
    ctr_interrupt_set(CTR_INTERRUPT_DATABRT, databrt_INT);
    ctr_interrupt_set(CTR_INTERRUPT_FIQ,     fiq_INT);
}
