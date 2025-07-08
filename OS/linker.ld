ENTRY(_start)

SECTIONS {
  . = 1M;

  .text : {
    *(.multiboot)       /* Multiboot header MUST come first */
    _text_start = .;
    *(.text*)
    _text_end = .;
  }

  .rodata : {
    *(.rodata*)
  }

  .data : {
    *(.data*)
  }

  .bss : {
    _bss_start = .;
    *(.bss*)
    *(COMMON)
    _bss_end = .;
  }

  /* Optional: mark kernel end for heap placement */
  kernel_end = .;
}

