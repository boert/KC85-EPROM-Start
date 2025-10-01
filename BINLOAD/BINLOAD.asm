; basiert direkt auf der LOAD-Routine
; der M030-EPROM-Software von maleuma

; Ladeadresse: 4000h (oder 1. Parameter)
; Endadresse:  E000h (oder 2. Parameter), oder Dateiende

        ORG 180h
        INCLUDE "CAOS.INC"      ; CAOS-Vereinbarungen


; KCC-Header:
;
        DB      'BINLOAD.KCC'   ; Dateiname
        DS      5,0             ; frei
        DB      2               ; 2 Argumente
        DW      PBEGIN, PENDE   ; Anfangs- und Endadresse
        DS      107,0           ; frei

;
PBEGIN:
VERSION: DB     0               ; ermittelte CAOS-Version (fuer Schnellzugriff)

; Einlesen von CAOS-Dateien in den Programmierpuffer (hinter IRM)
; - ohne Parameter direkt in den Programmierpuffer
; - mit Parameter auf die angegebene Adresse
;
M5:     DW      07F7Fh
        DB      'BINLOAD',1

        CALL    CHKCAOS

; 1. Kontrolle, ob gueltiger Puffer benutzt wird:
        LD      DE,4000H        ; Start des Bereichs
        LD      A,(ARGN)
        CP      A,0             ; Ladeadresse angegeben?
        JR      Z,LD0           ; nein
        LD      DE,(ARG1)       ; Laden ab angegebener Adresse

LD0:    CALL    PROTEC          ; Speicherschutz?
        RET     C               ; Abbruch

; 2. ersten Block einlesen:
        CALL    PV1
        DB      OSTR            ; 3 Zeilen "frei" scrollen
        DB      LF,LF,LF,0BH,0BH,0BH,0
        LD      A,(VERSION)     ; erkannte CAOS-Version
        CP      46h             ; CAOS 4.6 ?
        CCF                     ; CY negieren
        CALL    C,NAME          ; Dateiname abfragen ab CAOS 4.6
        RET     C               ; mit BRK abgebrochen
        LD      (IX+3),1        ; 1. Block lesen
        LD      (IX+5),lo( CASS)
        LD      (IX+6),hi( CASS)    ; Kassettenpuffer setzen
        CALL    PV1             ; HL = Dateiname
        DB      ISRI            ; Vorblock einlesen
LD1:    CALL    BLNR            ; Blocknummer anzeigen
        JP      NZ,LOAD5        ; -> Abbruch mit CSRI
        JR      NC,LD2          ; Vorblock korrekt gelesen
        CALL    PV1
        DB      MBI             ; naechsten Block lesen
        JR      LD1             ; und auswerten

; 3. Laden ab angegebener Adresse
LD2:    LD      DE,04000H       ; Start des Bereichs
        LD      HL,0E000H       ; Ende des Bereichs
        LD      A,(ARGN)
        CP      A,0             ; Ladeadresse angegeben?
        JR      Z,LD3           ; nein

        LD      DE,(ARG1)       ; Laden ab angegebener Adresse
        CP      A,1             ; Endadresse angegeben?
        JR      Z,LD3           ; nein

        LD      HL,(ARG2)       ; Bereichsende holen
LD3:

; 5. nur Startadresse anzeigen
        CALL    PV1
        DB      OSTR
        DB      19h,"Start: ",0 ; 19h = Cursor auf Zeilenanfang
        EX      DE,HL
        CALL    PV1
        DB      HLHX            ; Anfangsadresse
        CALL    PV1
        DB      CRLF            ; Zeilenvorschub nach Adressen
        EX      DE,HL           ; DE = Ladeadresse
        DJNZ    LOAD3           ; ersten Block als Datenblock verwenden

; 6. LOAD-Hauptschleife:
;       HL = Endadresse
;       DE = Ladeadresse
;       B = 0   gueltiger Vorblock (Ende durch Adresskontrolle)
;       B = FFh ungueltiger Vorblock (Ende durch Blocknummernkontrolle)
;
LOAD1:  INC     (IX+3)          ; naechster Block wird erwartet
LOAD2:  PUSH    DE
        PUSH    BC
        CALL    PV1
        DB      MBI             ; 128 Byte einlesen
        POP     BC
        CALL    BLNR            ; Blocknummer anzeigen
        POP     DE
        JR      NZ,LOAD5        ; -> Abbruch mit CSRI
        JR      C,LOAD2         ; Block wiederholen
LOAD3:  PUSH    HL              ; Endadresse
        PUSH    BC
        LD      HL,CASS         ; Kassettenpuffer
        LD      C,88H
        IN      B,(C)
LOAD4:  LD      A,(HL)          ; Datenbyte aus Kassettenpuffer
        RES     2,B             ; IRM off
        OUT     (C),B
        LD      (DE),A          ; Datenbyte in Speicher
        SET     2,B             ; IRM on
        OUT     (C),B
        INC     DE
        INC     L
        JP      P,LOAD4         ; wiederholen solange Bit 7=0 ist
        POP     BC
        POP     HL
        LD      A,(IX+2)        ; Blocknummer
        AND     B               ; Blocknummernkontrolle
        INC     A               ; und Block FF erkannt?
     ;  JR      Z,LOAD5         ; -> dann fertig eingelesen
     ; auch mehr als 32k (Blocknummer > FFh) einlesen
        PUSH    HL
        SBC     HL,DE           ; Endadresse erreicht?
        POP     HL
        JR      Z,LOAD5         ; fertig
        JR      NC,LOAD1        ; es sind weitere Daten einzulesen
LOAD5:  CALL    PV1
        DB      CSRI            ; Abschluss MB-Eingabe
        CALL    PV1
        DB      OSTR
        DB      19h,"End: ",0   ; 19h = Cursor auf Zeilenanfang
        EX      DE,HL
        CALL    PV1
        DB      HLHX            ; Endadresse
        CALL    PV1
        DB      CRLF
        RET
;
; Dateiname eingeben fuer LOAD und SAVE
; PA:   HL      Zeiger auf eingegebenen Namen im IRM
;       CY=1    Eingabe mit BRK abgebrochen bei CAOS 3.4 und ab CAOS 4.3
; VR:   AF, HL, DE
;
NAME:   CALL    PV1
        DB      OSTR
        DB      'Name :',0
        CALL    PV1
        DB      INLIN           ; Dateiname abfragen
        JR      NC,NAM1         ; Eingabe nicht mit BRK beendet
        LD      A,(VERSION)     ; BRK wird aber erst ab CAOS 4.3 ausgewertet
        CP      34h
        JR      Z,NAM2
        CP      43h
NAM2:   CCF
        RET     C               ; BRK bei CAOS 3.4 und 4.3+
NAM1:   LD      HL,6
        ADD     HL,DE           ; Beginn Name
        RET
;
; Blocknummer anzeigen bei LOAD
; PE:   CY=1    Block fehlerhaft gelesen
;       (IX+3)  erwartete Blocknummer
; PA:   Z=0     fehlerhaft - Abbruch (CAOS 4.6 und Device > 0 oder BRK)
;       CY=1    fehlerhaft - Wiederholung noetig (Kassette)
; VR:   AF
;
BLNR:   PUSH    AF              ; CY merken
        LD      A,19H
        CALL    PV1             ; Cursor auf Zeilenanfang
        DB      CRT
        CALL    PV1
        DB      BRKT            ; BRK gedrueckt?
        JR      C,BLN0          ; ja, Abbruch
        POP     AF
        JR      NC,BLN2         ; kein Fehler
        LD      A,(VERSION)
        CP      46h             ; CAOS 4.6?
        JR      C,BLN1          ; nein, also Lesefehler von Kassette
        LD      A,(IX+8)
        AND     00011100b       ; Device?
        JR      Z,BLN1          ; Lesefehler bei Device=0 (Kassette)
        DB      3Eh             ; LD A,n
BLN0:   POP     AF
        OR      0FFh            ; Z=0 - Fehler erfordert Abbruch
        RET

; Lesefehler von Kassette
BLN1:   LD      A,'?'           ; fehlerhafter Block
        JR      BLN3

; kein Fehler
BLN2:   LD      A,(IX+2)        ; gelesener Block
        CP      (IX+3)          ; gleich erwarteter Block?
        JR      Z,BLN4          ; OK, richtiger Block
        INC     A               ; oder Endeblock FF?
        JR      Z,BLN4          ; auch als OK durchgehen lassen
        OR      (IX+3)
        DEC     C               ; Blocknummer 0 ebenfalls als Vorblock OK
        JR      Z,BLN4
        LD      A,'*'           ; nicht erwarteter Block
BLN3:    EX     AF,AF'
         LD     A,(IX+3)        ; erwarteter Block
         CALL   PV1
         DB     AHEX            ; Anzeige erwarteter Block
         LD     A,':'
         CALL   PV1             ; Doppelpunkt
         DB     CRT
         LD     A,(IX+2)        ; gelesene Blocknummer
         CALL   PV1
         DB     AHEX            ; Blocknummer anzeigen
        EX      AF,AF'
        CALL    PV1
        DB      CRT             ; Kennzeichnung * oder ?
        XOR     A
        SCF                     ; Z=1 und CY=1 - fehlerhaft, Wiederholung noetig
        RET

; richtiger Block
BLN4:   LD      A,(IX+2)        ; gelesener Block
        CALL    PV1
        DB      AHEX            ; Anzeige Blocknummer
        CALL    PV1
        DB      OSTR
        DB      '>   ',0        ; richtiger Block
        XOR     A               ; Z=1 und CY=0 - korrekt
        RET


; Kontrolle, dass RAM0 (unterhalb 0400h) nicht ueberschrieben wird
; PE:   DE      Anfangsadresse Speicherbereich
; PA:   CY=1    Speicherschutz -> Abbruch erforderlich
; VR:   AF
;
PROTEC: ld      a,d
        cp      04h
        jr      c,PROT          ; Beginn kleiner als 0400h
        ret             ; mit CY = 0
        ;
PROT:   CALL    PV1
        DB      OSTR
        DB      'Speicherschutz!',CR,LF,0
        SCF             ; CY = 1
        RET

; Test, welche CAOS-Version vorliegt und abspeichern in Arbeitszelle "VERSION"
; (zur Unterscheidung CAOS 3.1 und CAOS 3.4 wird das Menuewort BASIC im ROM-E
; gesucht, welches unter 4.3 wie beim KC85/4 am Anfang des ROM-E steht, aber
; auf einer anderen Adresse...)
; PA:   CY=1    CAOS 3.4 oder hoeher
;       A       CAOS-Versionsnummer BCD
; VR:   AF

CHKCAOS:LD      A,(0E011H)      ; Beim ist KC85/4 hier immer BASIC-Menuewort
        CP      7Fh             ; KC 85/4 ?
        JR      NZ,KCT1
        SCF                     ; wir haben CAOS 4.x
        LD      A,(CAOSNR)      ; Versionsnummer steht seit CAOS 4.1 immer hier
        JR      KCT4
        ;
KCT1:   PUSH    HL              ; Beim KC85/3 Suche nach Menuewort "BASIC"
        PUSH    DE              ; im ROM E -> dann CAOS 3.4
        PUSH    BC
        LD      A,7FH           ; A = CAOS-Prologbyte
        LD      DE,BASIC        ; DE = Vergleichszeichenkette
        LD      HL,0E000h       ; HL = Beginn Suchbereich
        LD      BC,100h         ; BC = Laenge des Suchbereichs
        CALL    PV1             ; BASIC-Menuewort suchen ab E000h
        DB      ZSUCH           ; wenn vorhanden, dann CAOS 3.4 oder hoeher
        LD      A,34h           ; CAOS 3.4 melden
        JR      C,KCT3          ; Menuewort gefunden
        LD      HL,0E800H
        LD      (HL),0FFH       ; Beim KC85/2 ist hier kein ROM, evtl. aber RAM?
        LD      A,(HL)          ; Testen was drin steht in der Speicherzelle
        INC     A
        LD      A,31h           ; CAOS 3.1 melden
        JR      NZ,KCT3
        LD      A,22h           ; CAOS 2.2 melden
KCT3:   POP     BC
        POP     DE
        POP     HL
KCT4:   LD      (VERSION),A     ; fuer weitere Abfragen hier ablegen
        RET
        ;
BASIC:  DB      'BASIC',0       ; Vergleichskette fuer Suche CAOS 3.4


PENDE:
        align 128
