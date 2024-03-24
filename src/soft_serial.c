#include "soft_serial.h"
#include "ch55x.h"

#define SSP SPLIT_SOFT_SERIAL_PIN

void soft_serial_init() {
    __asm
        setb SPLIT_SOFT_SERIAL_PIN
    __endasm;
}

void soft_serial_recv() {
    soft_serial_did_not_respond = 1;
    __asm
        mov r0, #8
        mov r2, #0
    00000$:
        jnb SSP, 00001$
        djnz r2, 00000$     ; poll start bit 256 times
        sjmp 00099$
    00001$:
        mov r1, #0          ; 2 cyc nop
        // cpl 0xB2         ; for testing only
        mov r1, #0
        mov r1, #0
        mov r1, #0
    00002$:
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov c, SSP          ; 2 cyc
        // cpl 0xB2         ; for testing only
        rrc a               ; 1 cyc
        djnz r0, 00002$     ; if jump 4 else 2 cyc
    
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0

        nop                 ; add some delay before confirming stop bit
        jnb SSP, 00099$     ; fail if stop bit not detected
        // cpl 0xB2         ; for testing only

        mov dptr, #_soft_serial_sbuf
        movx @dptr, a

        mov dptr, #_soft_serial_did_not_respond
        mov a, #0
        movx @dptr, a
    00099$:
    __endasm;
}

void soft_serial_send() {
    __asm
        mov dptr, #_soft_serial_sbuf
        movx a, @dptr
        mov r0, #8
        clr SSP        ; start bit
        mov r1, #0     ; 2 cyc nop
        mov r1, #0
        nop
    00001$:
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        rrc a           ; 1 cyc
        mov SSP, c      ; 2 cyc
        djnz r0, 00001$ ; if jump 4 else 2 cyc
    
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        mov r1, #0
        setb SSP        ; 2 cyc
    __endasm;
}
