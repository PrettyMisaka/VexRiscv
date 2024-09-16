.global crtStart
.global main

call main
crtStart:
  //long jump to allow crtInit to be anywhere
  //do it always in 12 bytes
  lui x10,       %hi(0x80000000)
  addi x10, x10,  %lo(0x80000000)
  jalr x1,x10
  nop