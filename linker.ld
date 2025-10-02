ENTRY(Reset_Handler)

SECTIONS {
  . = 0x00000000;
  .text : { *(.isr_vector) *(.text*) }
}
