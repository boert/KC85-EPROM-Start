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
    ; müssen vom PC-Programm ergänzt werden

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

    ; Menüpunkte restaurieren
    ; Anzahl
menu_cnt:
    db 0
    ; bis zu drei Adressen
menu_addr:
    dw 0
    dw 0
    dw 0

    ; hier startet der Menüeintrag
    dw 07F7Fh
mwort:
    db 'S'
    db 1

    ds 9, 0

    ; und hier das eigentliche Programm

    ; Block 1
    ld bc, (bl1_size)
    ld a, b
    or c
    jr z, skipcopy
    ld de, (prg_dest)
    ld hl, (bl1_start)
    ldir
skipcopy:

    ld a, (menu_cnt)
    or a
    jr z, skipmenu

    ld b, a
    ld a, 7Fh   ; Prolog
    ld hl, menu_addr

nextmenu:
    ld e, (hl)
    inc hl
    ld d, (hl)
    inc hl
    ld (de), a
    djnz nextmenu

skipmenu:
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
