    .syntax unified
    .cpu cortex-m3
    .thumb

    .section .isr_vector, "a", %progbits
    .word 0x20001000
    .word Reset_Handler

    .text
    .thumb_func
    .global Reset_Handler
Reset_Handler:
    MOV r0, #0
    MOV r1, #2
    MOV r2, #3
    MOV r3, r1
    MOV r4, r2
    MUL r6, r4, r5
    ADD r7, r3, r6
    MOV r5, r7
    MOV r8, r5
    MOV r2, r8
1:
    B 1b
