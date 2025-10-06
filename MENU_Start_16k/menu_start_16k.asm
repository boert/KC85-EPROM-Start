; Hilfsprogramm
; zum Starten eine Programms aus dem ROM
; 16k-Version für z.B. M028 16 K PROM
; Start des Programms mittels MENU
;
; (c) 2025, Bert Lange

PV1     EQU  0xF003 	; Sprungverteiler
UP_LOOP EQU  0x12
PIOAD	EQU 88h

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

    dw 07F7Fh
    db 'S'
    db 1

    ds 9, 0


    ; Block 1
    ld bc, (bl1_size)
    ld a, b
    or c
    jr z, skipcopy

    ; ROM E off
    di
    in a, (PIOAD)
    res 0, a
    out (PIOAD), a

    ld de, (prg_dest)
    ld hl, (bl1_start)
    ldir

    ; ROM E on
    in a, (PIOAD)
    set 0, a
    out (PIOAD), a

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
    ei
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
