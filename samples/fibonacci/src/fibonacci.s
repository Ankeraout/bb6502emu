.segment "VECTORS"

vector_nmi:
    .word $0000

vector_reset:
    .word _start

vector_irq:
    .word $0000

.segment "INIT"
_start:
    jmp main

.segment "BSS"
    fib_a:
        .res 1
    fib_b:
        .res 1

.segment "CODE"
main:
    lda #$00
    sta fib_a
    lda #$01
    sta fib_b
    ldx #$00
    clc

loop:
    lda fib_a
    adc fib_b
    bcs endloop
    sta $00, x
    inx

    ldy fib_b
    sty fib_a
    sta fib_b

    jmp loop

endloop:
    jmp endloop
