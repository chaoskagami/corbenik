// This is svcBackdoor's code from earlier FIRMs
.arm.little
.create "backdoor.bin", 0
	bic r1, sp, #0xff
	orr r1, r1, #0xf00
	add r1, r1, #0x28
	ldr r2, [r1]
	stmdb r2!, {sp, lr}
	mov sp, r2
	blx r0
	pop {r0, r1}
	mov sp, r0
	bx r1
.close
