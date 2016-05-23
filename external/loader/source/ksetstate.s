.section .text
.global KernelSetState
KernelSetState:
	svc 0x7C // KernelSetState
	bx lr // return;
