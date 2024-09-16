OUTPUT_FORMAT("elf32-littleriscv", "elf32-littleriscv", "elf32-littleriscv")
OUTPUT_ARCH(riscv)
/* 确保你的 BootROM 入口点正确 */
ENTRY(main)

MEMORY {
  BootROM (rx) : ORIGIN = 0x10000000, LENGTH = 1K /* 0xF0080000 Adjust size as needed */
}

SECTIONS {
  . = ORIGIN(BootROM);
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

}