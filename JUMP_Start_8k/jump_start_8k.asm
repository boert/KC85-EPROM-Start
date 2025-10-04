; Hilfsprogramm
; zum Starten eine Programms aus dem ROM
; 8k-Version für z.B. M025 USER PROM 8K
;
; (c) 2025, Bert Lange

pio1_A: equ 88h
NCAOS:  equ 0b7b4h

    org 0f000h

    ; die folgenden Informationen
    ; werden von JUMP_KCC ergänzt

    ; Programminfo
prg_dest:
    dw 00200h   ; Zieladresse für Programm
prg_start:
    dw 0e000h   ; Startadresse für Programm

    ; Block 1
bl1_start:
    dw 0ffffh   ; Quelle, Block 1
bl1_size:
    dw 00000h   ; Länge, Block 1

    ; Block 2
bl2_start:
    dw 0e000h   ; Quelle, Block 2
bl2_size:
    dw 00000h   ; Länge, Block 2


    ; Einsprungpunkt für JUMP
    org 0f012h

    ; Block 1
    ld bc, (bl1_size)
    ld a, b
    or c
    jr z, skip
    ld de, (prg_dest)
    ld hl, (bl1_start)
    ldir
    ; Block 2
    ld bc, (bl2_size)
    ld a, b
    or c
    jr z, skip
    ld hl, (bl2_start)
    ldir
skip:

    ld hl, (prg_start)

    ld (NCAOS+3), hl    ; weil hier die Zieladresse umgestellt wird, funktioniert JUMP nur einmal
    in a, (pio1_A)
    or 01h
    jp NCAOS
    ; = out (pio1_A),A
    ; = JP hl

