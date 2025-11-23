; Hilfsprogramm
; zum Starten eine Programms aus dem ROM
; Version für 8k + 16k
; Start des Programms mittels
; AUTOSTART (im Schacht 08) oder 
; MENU (Menüeintrag wählbar) oder
; JUMP (in beliebigem Modulschacht)
;
; mit ZX0-Entpacker, zusätzlichen Ausgaben und Prüfsummen
;
; (c) 2025, Bert Lange

PV1     EQU  0F003h     ; Sprungverteiler
UP_CRT  EQU  000h
UP_LOOP EQU  012h
UP_WAIT EQU  014h
UP_HLHX EQU  01Ah
UP_HLDE EQU  01Bh
UP_OSTR EQU  023h
UP_MODU EQU  026h
UP_CRLF EQU  02Ch

; Sonderzeichen
CLL     EQU  002h       ; clear a line
CUD     EQU  00Ah       ; cursor down
CLS     EQU  00Ch       ; clear screen
CR      EQU  00Dh       ; new line
HOME    EQU  010h       ; cursor home

; IRM-Zellen
CASS    EQU  0B700h     ; Kassettenpuffer
ARG1    EQU  0B782h     ; 1. Argument
ARG2    EQU  0B784h     ; 2. Argument

; sonstiges
PIOAD   EQU 088h
SW_LENGTH EQU  SW_END - SW_PROG
;   IX+11   ; Zwischenspeicher für Modulschacht
ZEIT    EQU 200


    ; alles im ROM
    org 0C000h

    ; aber erstmal auf 4000h
    .phase 04000h
    ; Einsprungpunkt für Autostart

    CALL PV1
    DB UP_OSTR
    DB HOME, CLL, "Start auf 4000 (Autostart)", CR, CUD, CLL, 0
    CALL WAIT_4000

    CALL PV1
    DB UP_OSTR
    DB CUD, CLL, "Hilfsprogramm nach B700", CR, 0

    CALL WAIT_4000

    ; Hilfsprogramm zum Modul umschalten (41h -> C1h)
    ; umkopieren und starten
    LD  HL, CCPROG      ; Quelle
    LD  DE, CASS        ; Ziel
    LD  BC, SW_LENGTH   ; Länge
    LDIR
    LD  (IX+11), 08h    ; Modulschacht für später
    JP  CASS

WAIT_4000:
    LD  A, ZEIT
    CALL    PV1
    DB  UP_WAIT
    RET

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

    CALL PV1
    DB UP_OSTR
    DB CUD, CLL, "BASIC-ROM aus", CR, 0
    CALL WAIT_B700

    ; BASIC wegschalten (nur bei 85/3 nötig, bei 85/4 unproblematisch)
    LD  A, 2
    LD  L, 02h      ; Schacht
    LD  D, 0        ; Steuerwort
    CALL    PV1
    DB  UP_MODU

    CALL PV1
    DB UP_OSTR
    DB CUD, CLL, "Modul auf C000", CR, 0
    CALL WAIT_B700

    ; Modul auf Adresse C000 schalten
    LD  A, 2
    LD  L, (IX+11)  ; Modulschacht
    LD  D, 0C1h     ; Steuerwort
    CALL    PV1
    DB  UP_MODU

    CALL PV1
    DB UP_OSTR
    DB CUD, CLL, "RAM4 ein", CR, 0
    CALL WAIT_B700

    ; RAM4 zuschalten (nur bei 85/4 nötig, bei 85/3 unproblematisch)
    LD  A, 2
    LD  L, 004h     ; Schacht
    LD  D, 003h     ; Steuerwort
    CALL    PV1
    DB  UP_MODU

;   CALL PV1
;   DB UP_OSTR
;   DB CUD, CLL, "weiter auf C000", CR, 0
;   CALL WAIT_B700

    JP  CSTART

WAIT_B700:
    LD  A, ZEIT
    CALL    PV1
    DB  UP_WAIT
    RET

SW_END:
    .dephase

#assert SW_END < 0B780h ; Kasettenpuffer begrenzt

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
bl1_fsum:
    dw 00000h   ; Prüfsumme, Block 1

    ; Block 2
bl2_start:
    dw 0c000h   ; Quelle, Block 2
bl2_size:
    dw 00000h   ; Länge, Block 2
bl2_fsum:
    dw 00000h   ; Prüfsumme, Block 2

    ; und hier das eigentliche Programm
CSTART:

    LD  A, (0E011h) ; CAOS ab 4.1 hat dort 07f7fh stehen
    CP  07Fh        ; KC85/4?
    JP  Z, AEND

    ; auf KC85/3 nach RAM-Modulen suchen

    ; könnte noch verbessert werden:
    ; Suche M022 ab Schacht 08
    ; Suche M011 hinter eigenem Schacht
    ; bei JUMP ist ein M022 im Schacht 8 schon aktiviert

    ; nur hinter eigenem Schacht suchen
    LD  A, (IX+11)
    INC A
    LD  (IX+11), A

    CALL PV1
    DB UP_OSTR
    DB CUD, CLL, "M011 suchen", CR, 0
    CALL WAIT_C000

    LD  A, 0F6h     ; M011
    CALL    MODSUCH
    JR  NC, RAMFOUND

    CALL PV1
    DB UP_OSTR
    DB CUD, CLL, "M022 suchen", CR, 0
    CALL WAIT_C000

    LD  A, 0F4h     ; M022
    CALL    MODSUCH
    JR  NC, RAMFOUND

    CALL PV1
    DB UP_OSTR
    DB CUD, CLL, "kein RAM-Modul gefunden", CR, 0
    CALL WAIT_C000

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

    CALL PV1
    DB UP_OSTR
    DB HOME, CLL, "MENU-Start", CR, CUD, CLL, 0

    CALL WAIT_C000

AEND:
    CALL PV1
    DB UP_OSTR

    DB CUD, CLL, "Block 1 in ROM: ", 0

    ; Checksumme im ROM
    call romoff
    ld bc, (bl1_size)
    ld hl, (bl1_start)
    call fletcher16
    call romon

    ; Ausgabe Hexwert
    call PV1
    db UP_HLHX

    ld de, (bl1_fsum)
    call compare_fsum
    call WAIT_C000
    jp c, JUMPCAOS

    CALL PV1
    DB UP_OSTR
    DB CUD, CLL, "Block 1 umladen", CR, 0

    CALL WAIT_C000

    call romoff
    ; Block 1
    ld bc, (bl1_size)
    ld de, (prg_dest)
    ld hl, (bl1_start)
    ldir
    ; Zwischenzieladresse wegspeichern
    ld (ARG1), de
    call romon

    CALL PV1
    DB UP_OSTR
    DB CUD, CLL, "Block 1 in RAM: ", 0

    ; Checksumme im RAM
    ld bc, (bl1_size)
    ld hl, (prg_dest)
    call fletcher16

    ; Ausgabe Hexwert
    call PV1
    db UP_HLHX

    ld de, (bl1_fsum)
    call compare_fsum
    call WAIT_C000
    jp c, JUMPCAOS

    ; Block 2

    ld bc, (bl2_size)
    ld a, b
    or c
    jp z, skipcopy

    CALL PV1
    DB UP_OSTR

    DB CUD, CLL, "Block 2 in ROM: ", 0

    ; Checksumme im ROM
    call romoff
    ld bc, (bl2_size)
    ld hl, (bl2_start)
    call fletcher16
    call romon

    ; Ausgabe Hexwert
    call PV1
    db UP_HLHX

    ld de, (bl2_fsum)
    call compare_fsum
    call WAIT_C000
    jp c, JUMPCAOS

    ; Ausgabe
    CALL PV1
    DB UP_OSTR
    DB CUD, CLL, "Block 2 umladen", CR, 0

    CALL WAIT_C000

    call romoff
    ld bc, (bl2_size)
    ld de, (ARG1)
    ld hl, (bl2_start)
    ldir
    call romon

    CALL PV1
    DB UP_OSTR
    DB CUD, CLL, "Block 2 in RAM: ", 0

    ; Checksumme im RAM
    ld bc, (bl2_size)
    ld hl, (ARG1)
    call fletcher16

    ; Ausgabe Hexwert
    call PV1
    db UP_HLHX

    ld de, (bl2_fsum)
    call compare_fsum
    call WAIT_C000
    jp c, JUMPCAOS

skipcopy:

    CALL PV1
    DB UP_OSTR
    DB CUD, CLL, "Entpacken", CR, 0
    CALL WAIT_C000

    ; dekomprimieren
    ld hl, (prg_dest)
    ld de, (decomp_addr)
    call dzx0_standard

    ; Stack vorbereiten
    ld de, JUMPCAOS
    push de

    ; Startmodus (=Anzahl der Argumente)
    ld  a, (prg_args)
    cp 3
    jr JUMPCAOS     ; kein Start
    jr c, JUMPCAOS

    CALL PV1
    DB UP_OSTR
    DB CUD, CLL, "Starten", CR, 0
    CALL WAIT_C000

    ; Programm starten
    ld hl, (prg_start)
    jp (hl)

    ; zurück zum CAOS
JUMPCAOS:

    CALL PV1
    DB UP_OSTR
    DB CUD, CLL, "zum CAOS", CR, CUD, CLL, CUD, CLL, 0
    CALL WAIT_C000

    CALL PV1
    DB   UP_LOOP
    ; -> UP_BYE geht nicht, ergibt ggf. Autostartschleife
    ; -> RET geht auch nicht, da ggf. nix im Stack liegt

WAIT_C000:
    PUSH    AF
    LD  A, ZEIT
    CALL    PV1
    DB  UP_WAIT
    POP     AF
    RET

romoff:
    ; Interrupts deaktivieren
    di

    ; ROM E ausschalten
    in a, (PIOAD)
    res 0, a
    out (PIOAD), a

    ret

romon:
    ; ROM E einschalten
    in a, (PIOAD)
    set 0, a
    out (PIOAD), a

    ; Interrupts einschalten
    ei

    ret

    ; Prüfsumme in HL
compare_fsum:
    ; vergleichen
    or a    ; clear carry
    sbc hl, de

    jr z, csumgood

    call PV1
    db UP_OSTR
    db "FALSCH!", 0
    scf     ; set carry
    ret

csumgood:
    call PV1
    db UP_OSTR
    db "ok.", 0
    or a    ; clear carry
    ret


    ;dzx0
    ; Parameters:
    ;   HL: source address (compressed data)
    ;   DE: destination address (decompressing)

#include "ZX0/dzx0_standard.asm"

;; Fletcher-16 checksum
;; Quelle:
;; https://64nops.wordpress.com/2021/06/19/checksum-algorithms-2/
;;
;; Input:
;;  HL = Data address
;;  BC = Data length
;;
;; Output:
;;  HL (und DE) = Fletcher-16
;;  BC,AF are modified

fletcher16:
     ; Initialize both sums to zero
     ld de,0
     ; Adjust 16-bit length for 2x8-bit loops
     inc b
     dec bc
     inc c
_fletcher16_loop:
       ld a,(hl)
       inc hl
       ; sum1 += data
       add a,e
       ld e,a
       ; sum2 += sum1
       add a,d
       ld d,a
       dec c
       jr nz,_fletcher16_loop
       djnz  _fletcher16_loop
       ld h, d
       ld l, e
     ret

    ; Ende der Hilfsprogramme
pr_end:

    ; hier kommt Block 1 (Daten)
    ; ...
    ; Ende Block 1 (Daten)

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

    ; hier kommt Block 2 (Daten)
