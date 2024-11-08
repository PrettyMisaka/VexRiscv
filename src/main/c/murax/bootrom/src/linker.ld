OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv", "elf32-littleriscv")
OUTPUT_ARCH(riscv)
/* 确保你的 BootROM 入口点正确 */
ENTRY(bootrom_start)

MEMORY {
  BootROM (rx) : ORIGIN = 0x10000000, LENGTH = 2K /* 0xF0080000 Adjust size as needed */
  Stack (rwx) : ORIGIN = 0x10000700, LENGTH = 256 /* 定义栈的大小为 256 字节，根据需要调整 */
}

_stack_size = DEFINED(_stack_size) ? _stack_size : 256;

SECTIONS {

  ._vector ORIGIN(BootROM): {
    *start.o(.start_jump);
    *start.o(.text);
  } > BootROM

  .text : {
    *(.text)
    *(.text.*)
  } > BootROM

  /* 如果你的 BootROM 需要初始化数据，可以在这里添加 */
  .data : {
    *(.data)
    *(.data.*)
  } > BootROM

  /* 如果你的 BootROM 需要包含特定的只读数据，可以在这里添加 */
  .rodata : {
    *(.rodata)
    *(.rodata.*)
  } > BootROM

  /* 如果你的 BootROM 需要包含特定的初始化代码，可以在这里添加 */
  .init_array : {
    KEEP(*(.init_array*))
  } > BootROM

  /* 栈区段现在引用了 MEMORY 定义中的 Stack 区域 */
  ._stack : {
    . = ALIGN(16); /* 确保栈顶地址对齐 */
    PROVIDE (_stack_end = .);
    /*. = . + _stack_size;*/
    . = ALIGN(16);
    PROVIDE (_stack_start = .);
    /* 栈的结束位置 */
  } > Stack
}