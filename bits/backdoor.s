// This is a "backdoor" supervisor call we install.
// Roughly, this is based on nintendo's, but there are a few defecits in their code.

// Namely, Nintendo's will crash ARM11 kernel if an IRQ/FIQ is hit while in the call,
// so we play it safe and disable them in cpsr while in backdoor.

.section .text
.global _start
_start:
	// Disable FIQ and IRQ.
	mrs r1, cpsr
	orr r1, #0xc0
	msr cpsr_c, r1

	// Save
	bic r1, sp, #0xff
	orr r1, r1, #0xf00
	add r1, r1, #0x28
	ldr r2, [r1]
	stmdb r2!, {sp, lr}
	mov sp, r2

	// Call user function.
	blx r0

	// Renable FIQ and IRQ.
	mrs r1, cpsr
	bic r1, #0xc0
	msr cpsr_c, r1

	// Restore registers.
	pop {r0, r1}
	mov sp, r0

	// Return.
	bx r1
