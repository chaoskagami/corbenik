.align 4
.global _start
.section .text.start, "x"
_start:
    ldr r2, =argc
    str r0, [r2]

    ldr r2, =argv
    str r1, [r2]

    b init

argc: .int 0x00000000
argv: .int 0x00000000

init:
    // Disable IRQ
    mrs r0, cpsr
    orr r0, r0, #0x80
    msr cpsr_c, r0

    // Flush caches, make sure to sync memory with what's on the cache before
    // turning off the MPU
    adr r0, flush_all_caches_offset
    ldr r1, [r0]
    add r0, r1, r0
    blx r0

    // Disable caches and MPU
    adr r0, disable_mpu_and_caching_offset
    ldr r1, [r0]
    add r0, r1, r0
    blx r0

    // Flush caches, for good measure. Makes sure there is nothing in the caches
    // when the MPU is brought back online.
    adr r0, flush_all_caches_offset
    ldr r1, [r0]
    add r0, r1, r0
    blx r0

    // FIXME should we attempt to save the last stack?
    // Setup stacks
    adr r0, setup_stacks_offset
    ldr r1, [r0]
    add r0, r1, r0
    blx r0

    // Switch to system mode
    mrs r0, cpsr
    orr r1, r0, #0x1F
    msr cpsr_c, r1

    // Change the stack pointer
    ldr sp, =0x27F00000

    // make sure ITCM is accessible
    mrc p15, 0, r0, c1, c0, 0
    orr r0, r0, #(1<<18)
    mcr p15, 0, r0, c1, c0, 0

    // clear bss
    adr r0, clear_bss_offset
    ldr r1, [r0]
    add r0, r1, r0
    blx r0

    // Give read/write access to all the memory regions
    ldr r0, =0x33333333
    mcr p15, 0, r0, c5, c0, 2 // write data access
    mcr p15, 0, r0, c5, c0, 3 // write instruction access

    // Set MPU permissions and cache settings
    ldr r0, =0xFFFF001D // ffff0000 32k   | bootrom unprotected
    ldr r1, =0x3000801B // 30000000 16k   | dtcm
    ldr r2, =0x00000035 // 00000000 128MB | itcm
    ldr r3, =0x08000029 // 08000000 2M    | arm9 mem
    ldr r4, =0x10000029 // 10000000 2M    | io mem
    ldr r5, =0x20000037 // 20000000 256M  | fcram
    ldr r6, =0x1FF00027 // 1FF00000 1M
    ldr r7, =0x1800002D // 18000000 8M
    mcr p15, 0, r0, c6, c0, 0
    mcr p15, 0, r1, c6, c1, 0
    mcr p15, 0, r2, c6, c2, 0
    mcr p15, 0, r3, c6, c3, 0
    mcr p15, 0, r4, c6, c4, 0
    mcr p15, 0, r5, c6, c5, 0
    mcr p15, 0, r6, c6, c6, 0
    mcr p15, 0, r7, c6, c7, 0
    mov r0, #0b10101001 // FIXME which sections does this do... stuff to?
    mcr p15, 0, r0, c2, c0, 0  // data cacheable
    mcr p15, 0, r0, c2, c0, 1  // instruction cacheable
    mcr p15, 0, r0, c3, c0, 0  // data bufferable

    // Enable caches and MPU
    adr r0, enable_mpu_and_caching_offset
    ldr r1, [r0]
    add r0, r1, r0
    blx r0

    // Fix mounting of SDMC
    ldr r0, =0x10000020
    mov r1, #0x340
    str r1, [r0]

    // Reload argc and argv.
    ldr r0, argc
    ldr r1, argv

    // Launch main(argc, argv)
    adr r2, main_offset
    ldr r3, [r2]
    add r2, r3, r2
    blx r2
    bx lr

    die:
    b die //if we return, just forcibly hang (should we attempt to call the rest vector???)

disable_mpu_and_caching_offset:
.word disable_mpu_and_caching-.

enable_mpu_and_caching_offset:
.word enable_mpu_and_caching-.

relocate_section_offset:
.word relocate_section-.

flush_all_caches_offset:
.word flush_all_caches-.

setup_stacks_offset:
.word setup_stacks-.

__bss_start_offset:
.word __bss_start__-.

__bss_end_offset:
.word __bss_end__-.

clear_bss_offset:
.word clear_bss-.

main_offset:
.word main-.

clear_bss:
    //clear bss
    adr r0, __bss_start_offset
    ldr r1, [r0]
    add r0, r1, r0

    adr r1, __bss_end_offset
    ldr r2, [r1]
    add r1, r2, r1
    mov r2, #0
    .Lclear_bss_loop:
        cmp r0, r1
        beq .Lclear_bss_loop_done
        str r2, [r0], #4
        b .Lclear_bss_loop
    .Lclear_bss_loop_done:
    blx lr

setup_stacks:
    // Set up the stacks for all CPU modes
    // start by clearing mode bits
    mrs r0, cpsr
    mov r2, r0 // preserve current mode
    bic r0, r0, #0x1F

    // System mode
    orr r1, r0, #0x1F
    msr cpsr_c, r1
    ldr sp, =0x8000

    // Abort mode
    orr r1, r0, #0x17
    msr cpsr_c, r1
    ldr sp, =0x8000

    // IRQ mode
    orr r1, r0, #0x12
    msr cpsr_c, r1
    ldr sp, =0x8000

    // FIQ mode
    orr r1, r0, #0x11
    msr cpsr_c, r1
    ldr sp, =0x8000

    // Supervisor mode
    orr r1, r0, #0x13
    msr cpsr_c, r1
    ldr sp, =0x8000

    // Restore prvious mode
    msr cpsr_c, r2

    blx lr

disable_mpu_and_caching:
    // Disable caches and MPU
    mrc p15, 0, r0, c1, c0, 0  // read control register
    bic r0, r0, #(1<<12)       // - instruction cache enable
    bic r0, r0, #(1<<2)        // - data cache enable
    bic r0, r0, #(1<<0)        // - mpu enable
    mcr p15, 0, r0, c1, c0, 0  // write control register

    bx lr

enable_mpu_and_caching:
    // Enable caches, MPU, and itcm
    mrc p15, 0, r0, c1, c0, 0  // read control register
    orr r0, r0, #(1<<18)       // - itcm enable
    orr r0, r0, #(1<<12)       // - instruction cache enable
    orr r0, r0, #(1<<2)        // - data cache enable
    orr r0, r0, #(1<<0)        // - mpu enable
    mcr p15, 0, r0, c1, c0, 0  // write control register

    bx lr

// r0 - region start
// r1 - region end
// r2 - relocation base (usually starting PC address)
relocate_section:

    adr r2, relocation_base_offset
    ldr r3, [r2]
    add r2, r3, r2

    .Lreloc_init:
    cmp r0, r1
    beq .Lrelocinit_done
    ldr r3, [r0]
    add r3, r2, r3
    str r3, [r0], #4
    b .Lreloc_init
    .Lrelocinit_done:

    bx lr

relocation_base_offset:
.word _start-.

flush_all_caches:

    // flush instruction cache, it's not flushed by Nintendo's function
    mov r0, #0
    mcr p15, 0, r0, c7, c5, 0

    // Nintendo's function uses r0-r2, r12, all registers that don't need
    // to be saved, just be aware that they are changed
    // use Nintendo's bx lr to return
    ldr r0, =0xFFFF0830 // Nintendo's flush function in unprot. bootrom
    bx r0

