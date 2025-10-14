; Hilfsprogramm
; zum Starten eine Programms aus dem ROM
; 8k-Version für z.B. M025 USER PROM 8K (modifiziertes Strukturbyte)
; Start des Programms mittels 
; AUTOSTART (im Schacht 08) oder per MENU
;
; (c) 2025, Bert Lange

PV1     EQU  0xF003     ; Sprungverteiler
UP_CRT  EQU  0x00
UP_LOOP EQU  0x12
UP_MODU EQU  0x26

; Sonderzeichen
CLS     EQU     00Ch

; IRM-Zellen
CASS    EQU  0B700h     ; Kassettenpuffer

; sonstiges
SW_LENGTH EQU  SW_END - SW_PROG


    ; alles im ROM
    org 0C000h

    ; aber erstmal auf 4000h
    .phase 04000h

    ; Hilfsprogramm zum Modul umschalten (41h -> C1h)
    ; umkopieren und starten
    LD  HL, CCPROG      ; Quelle
    LD  DE, CASS        ; Ziel
    LD  BC, SW_LENGTH   ; Länge
    LDIR
    JP  CASS

CCPROG:
    .dephase

    ; weiter mit Hilfsroutine zum Modul umschalten
    .phase CASS
SW_PROG:

    ; BASIC wegschalten (nur bei 85/3 nötig, bei 85/4 unproblematisch)
    LD  A, 2
    LD  L, 02h      ; Schacht
    LD  D, 0        ; Steuerwort
    CALL    PV1
    DB  UP_MODU

    ; Modul auf Adresse C000 schalten
    LD  A, 2
    LD  L, 008h     ; Schacht
    LD  D, 0C1h     ; Steuerwort
    CALL    PV1
    DB  UP_MODU

    ; RAM4 zuschalten (nur bei 85/4 nötig, bei 85/3 unproblematisch)
    LD  A, 2
    LD  L, 004h     ; Schacht
    LD  D, 003h     ; Steuerwort
    CALL    PV1
    DB  UP_MODU

    JP  CSTART
SW_END:
    .dephase

    ; die folgenden Informationen
    ; müssen vom PC-Programm ergänzt werden

    ; Programminfo
prg_dest:
    dw 00200h   ; Zieladresse für Programm
prg_start:
    dw JUMPCAOS ; Startadresse für Programm

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
CSTART:

    LD  A, (0E011h) ; CAOS ab 4.1 hat dort 07f7fh stehen
    CP  07Fh        ; KC85/4?
    JR  Z, AEND

    ; auf KC85/3 nach RAM-Modulen (M011/M022) suchen
    LD  A, 0F6h     ; M011
    CALL    MODSUCH
    JR  NC, RAMFOUND
    LD  A, 0F4h     ; M022
    CALL    MODSUCH
    JR  NC, RAMFOUND
    JR AEND

    ; Modulsuche ab Schacht 8
    ; in:
    ;   A   Modulkennung
    ; out:
    ;   CY = 0  gefunden und
    ;   A   Modulschacht oder
    ;   CY = 1  nicht gefunden
    ; ändert:
    ;   BC, DE
MODSUCH:
    LD  D, A        ; D = gesuchte Kennung
    LD  BC, 00880h
MODNEXT:
    IN  A, (C)      ; Kennung einlesen
    CP  D
    JR  Z, MODFOUND ; gefunden
    INC B
    JR  NZ, MODNEXT
                ; nicht gefunden
    SCF         ; Carry auf 1
    RET

MODFOUND:
    LD  A, B    ; Modulschacht 
    SCF         ; Carry auf 1
    CCF         ; Carry invert
    RET

RAMFOUND:
    ; M022/M011 gefunden
    LD  B, A        ; Schacht wegspeichern

    ; M022 oder M011 einschalten
MODON:  LD  A, 2
    LD  L, B        ; Schacht
    LD  D, 043h     ; Steuerwort
    CALL    PV1
    DB  UP_MODU

AEND:
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

    ; Stack vorbereiten
    ld de, JUMPCAOS
    push de

    ; Programm starten
    ld hl, (prg_start)
    jp (hl)

    ; zurück zum CAOS
JUMPCAOS:
    CALL PV1
    DB   UP_LOOP
    ; -> UP_BYE geht nicht, ergibt Autostartschleife
    ; -> ret geht auch nicht, da ggf. nix im Stack liegt

pr_end:
