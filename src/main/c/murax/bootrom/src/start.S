.global bootrom_start
.global main
.global trap_handler

    .section	.start_jump,"ax",@progbits
bootrom_start:
crtInit:
  .option push
  .option norelax
  .option pop
  la sp, _stack_start

  li a0, 0x880     //880 enable timer + external interrupts
  csrw mie,a0
  li a0, 0x1808     //1808 enable interrupts
  csrw mstatus,a0
  j main

.global  trap_entry
.align 5
trap_entry:
  sw x1,  - 1*4(sp)
  sw x5,  - 2*4(sp)
  sw x6,  - 3*4(sp)
  sw x7,  - 4*4(sp)
  sw x10, - 5*4(sp)
  sw x11, - 6*4(sp)
  sw x12, - 7*4(sp)
  sw x13, - 8*4(sp)
  sw x14, - 9*4(sp)
  sw x15, -10*4(sp)
  sw x16, -11*4(sp)
  sw x17, -12*4(sp)
  sw x28, -13*4(sp)
  sw x29, -14*4(sp)
  sw x30, -15*4(sp)
  sw x31, -16*4(sp)
  addi sp,sp,-16*4
  call trap_handler
  lw x1 , 15*4(sp)
  lw x5,  14*4(sp)
  lw x6,  13*4(sp)
  lw x7,  12*4(sp)
  lw x10, 11*4(sp)
  lw x11, 10*4(sp)
  lw x12,  9*4(sp)
  lw x13,  8*4(sp)
  lw x14,  7*4(sp)
  lw x15,  6*4(sp)
  lw x16,  5*4(sp)
  lw x17,  4*4(sp)
  lw x28,  3*4(sp)
  lw x29,  2*4(sp)
  lw x30,  1*4(sp)
  lw x31,  0*4(sp)
  addi sp,sp,16*4
  mret
  .text

.global crtStartRam
crtStartRam:
  //long jump to allow crtInit to be anywhere
  //do it always in 12 bytes
  lui x10,       %hi(0x80000000)
  addi x10, x10,  %lo(0x80000000)
  jalr x1,x10

.global crtStartDDR3
crtStartDDR3:
  //long jump to allow crtInit to be anywhere
  //do it always in 12 bytes
  lui x10,       %hi(0x40030000)
  addi x10, x10,  %lo(0x40030000)
  jalr x1,x10