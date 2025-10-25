; Hilfsprogramm
; zum Starten eine Programms aus dem ROM
; Version für 8k + 16k
; Start des Programms mittels
; AUTOSTART (im Schacht 08) oder 
; MENU (Menüeintrag wählbar) oder
; JUMP (in beliebigem Modulschacht)
;
; mit ZX0-Entpacker
;
; (c) 2025, Bert Lange

PV1     EQU  0xF003     ; Sprungverteiler
UP_CRT  EQU  0x00
UP_LOOP EQU  0x12
UP_MODU EQU  0x26

; Sonderzeichen
CLS     EQU  00Ch

; IRM-Zellen
CASS    EQU  0B700h     ; Kassettenpuffer
ARG1    EQU  0B782h     ; 1. Argument

; sonstiges
PIOAD   EQU 088h
SW_LENGTH EQU  SW_END - SW_PROG
;   IX+11   ; Zwischenspeicher für Modulschacht


    ; alles im ROM
    org 0C000h

    ; aber erstmal auf 4000h
    .phase 04000h
    ; Einsprungpunkt für Autostart

    ; Hilfsprogramm zum Modul umschalten (41h -> C1h)
    ; umkopieren und starten
    LD  HL, CCPROG      ; Quelle
    LD  DE, CASS        ; Ziel
    LD  BC, SW_LENGTH   ; Länge
    LDIR
    LD  (IX+11), 08h    ; Modulschacht für später
    JP  CASS

CCPROG:
    .dephase

PROG:

    ; weiter mit Hilfsroutine zum Modul umschalten
    .phase CASS
SW_PROG:
    ; ROM E einschalten
    in a, (PIOAD)
    set 0, a
    out (PIOAD), a

    ; BASIC wegschalten (nur bei 85/3 nötig, bei 85/4 unproblematisch)
    LD  A, 2
    LD  L, 02h      ; Schacht
    LD  D, 0        ; Steuerwort
    CALL    PV1
    DB  UP_MODU

    ; Modul auf Adresse C000 schalten
    LD  A, 2
    LD  L, (IX+11)  ; Modulschacht
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
prg_args:
    db 0        ; Anzahl der Argumente

decomp_addr:
    dw 0        ; Adresse der komprimierten Daten

    ; Block 1
bl1_start:
    dw pr_end   ; Quelle, Block 1
bl1_size:
    dw 00000h   ; Länge, Block 1

    ; Block 2
bl2_start:
    dw 0c000h   ; Quelle, Block 2
bl2_size:
    dw 00000h   ; Länge, Block 2

    ; und hier das eigentliche Programm
CSTART:

    LD  A, (0E011h) ; CAOS ab 4.1 hat dort 07f7fh stehen
    CP  07Fh        ; KC85/4?
    JR  Z, AEND

    ; auf KC85/3 nach RAM-Modulen suchen

    ; könnte noch verbessert werden:
    ; Suche M022 ab Schacht 08
    ; Suche M011 hinter eigenem Schacht
    ; bei JUMP ist ein M022 im Schacht 8 schon aktiviert

    ; nur hinter eigenem Schacht suchen
    LD  A, (IX+11)
    INC A
    LD  (IX+11), A

    LD  A, 0F6h     ; M011
    CALL    MODSUCH
    JR  NC, RAMFOUND
    LD  A, 0F4h     ; M022
    CALL    MODSUCH
    JR  NC, RAMFOUND
    JR AEND

    ; Modulsuche ab Schacht (IX+11)
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
    LD  C, 080h
    LD  B, (IX+11)
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

    JR  AEND

    ; hier startet der Menüeintrag
    DW 07F7Fh
mwort:
    DB 'S'
    DB 1

    ; Einsprungpunkt für MENU
    DS 9, 0

AEND:

    ; Interrupts deaktivieren
    di
    ; ROM E ausschalten
    in a, (PIOAD)
    res 0, a
    out (PIOAD), a

    ; Block 1
    ld bc, (bl1_size)
    ld de, (prg_dest)
    ld hl, (bl1_start)
    ldir

    ; Block 2
    ld bc, (bl2_size)
    ld a, b
    or c
    jr z, skipcopy
    ;  DE ist noch richtig
    ld hl, (bl2_start)
    ldir

skipcopy:

    ; dekomprimieren
    ld hl, (prg_dest)
    ld de, (decomp_addr)
    call dzx0_standard

    ; ROM E einschalten
    in a, (PIOAD)
    set 0, a
    out (PIOAD), a
    ; Interrupts einschalten
    ei

    ; Stack vorbereiten
    ld de, JUMPCAOS
    push de

    ; Startmodus (=Anzahl der Argumente)
    ld  a, (prg_args)
    cp 3
    jr c, JUMPCAOS

    ; Programm starten
    ld hl, (prg_start)
    jp (hl)

    ; zurück zum CAOS
JUMPCAOS:
    CALL PV1
    DB   UP_LOOP
    ; -> UP_BYE geht nicht, ergibt ggf. Autostartschleife
    ; -> RET geht auch nicht, da ggf. nix im Stack liegt


    ;dzx0
    ; Parameters:
    ;   HL: source address (compressed data)
    ;   DE: destination address (decompressing)

#include "ZX0/dzx0_standard.asm"

pr_end:

    ; hier kommt Block 1

    ; Einsprungpunkt für JUMP
    .phase 0f012h

jump_prog:

    ; Hilfsprogramm zum Modul umschalten (FFh -> C1h)
    ; umkopieren und starten
    LD  HL, PROG        ; Quelle
    LD  DE, CASS        ; Ziel
    LD  BC, SW_LENGTH   ; Länge
    LDIR
    LD  A, (ARG1)       ; 1. Argument (= Modulschacht)
    LD  (IX+11), A      ; Modulschacht wegspeichern
    JP  CASS

    .dephase
    
    ; hier kommt Block 2
