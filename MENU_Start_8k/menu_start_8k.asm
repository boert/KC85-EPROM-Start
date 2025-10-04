; Hilfsprogramm
; zum Starten eine Programms aus dem ROM
; 8k-Version für z.B. M025 USER PROM 8K
; Start des Programms mittels MENU
;
; (c) 2025, Bert Lange

PV1     EQU  0xF003 	; Sprungverteiler
UP_LOOP EQU  0x12

    org 0c000h

    ; die folgenden Informationen
    ; werden vom PC-Programm ergänzt

    ; Programminfo
prg_dest:
    dw 00200h   ; Zieladresse für Programm
prg_start:
    dw 0e000h   ; Startadresse für Programm
prg_args:
    db 0        ; Anzahl der Argumente

    ; Block 1
bl1_start:
    dw pr_end   ; Quelle, Block 1
bl1_size:
    dw 00000h   ; Länge, Block 1

    dw 07f7fh
    db 'S'
    db 1

    ds 9, 0


    ; Block 1
    ld bc, (bl1_size)
    ld a, b
    or c
    jr z, skip
    ld de, (prg_dest)
    ld hl, (bl1_start)
    ldir
skip:

    ld hl, (prg_start)
    push hl

    ; Startmodus (=Anzahl der Argumente)
    ld  a, (prg_args)
    cp 3
    jr c, NOSTART

    ; Programmadresse liegt auf Stack
    ; eigentliches Programm starten
    ret
    ;= call (HL)

    ; zurück zum CAOS
CAOS:
    CALL PV1
    DB   UP_LOOP

NOSTART:
    pop hl  ; Programmadresse aufräumen
    jr CAOS

pr_end:
