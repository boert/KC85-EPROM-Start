# Prüfprogramm für KCC/KCB-Dateien

Dieses Projekt enthält ein Prüfprogramm für KCC- bzw. KCB-Dateien.  
Das Dateiformat wird auf dem Kleincomputer [KC85](https://de.wikipedia.org/wiki/Kleincomputer_KC_85/2-4) von Mühlhausen genutzt.

Folgende Prüfungen werden durchgeführt:

- ob ein KCC-Header vorhanden ist
- ob die Dateilänge ein Vielfaches von 128 ist
- ob Datei evtl. eine BASIC-Datei ist (z.B. Kennung SSS)
- ob Datei evtl. eine TAP-Datei mit BASIC-Inhalt ist
- ob der Header eine gültige Anzahl an Adressargumenten hat (zwischen 2 und 10)
- ob die Dateigröße zur Größe aus dem Header passt

Die Ausgabe enthält:

- alle Informationen aus dem KCC-Header
- wieviel RAM für das Einladen des Programms benötigt wird (zur Laufzeit kann es ggf. mehr sein)
- enthaltene Menüeinträge
- ggf. Speichergröße aus BASIC-Arbeitszellen
- ggf. BASIC-Autostartroutine


## Aufruf
`check_KCC <KCC-Datei(en)>`


## Beispielausgabe für KCC-Datei

```
Datei: TEXPRO.KCC
Größe: 7808 Bytes

Header (Auszug):
01 37 CB 10 13 3E 06 BB   20 F3 78 17 17 CD 30 BB   .7...>..  .x...0. 
02 00 02 00 20 01 0A 27   00 00 00 C9 00 00 00 00   .... ..' ........ 
 
Header-Informationen
Name:           .7 ..>.   x
Anzahl Adr.:    2
Anfangsadr.:    0200h
Endeadr(+1):    2000h
(Startadr.)     (0A01h)
Programmgröße:  7680 Bytes
ungenutzt:      0 Byte(s) (am Ende)
min. 16 kByte RAM benötigt (RAM0).

Menüeinträge:
 1: B          -> 0DAAh
 2: NEW        -> 0DFCh
 3: TD         -> 0E3Eh
 4: TLOAD      -> 1931h
```

## Beispielausgabe für KCB-Datei

```
Datei: 90JAHRE.KCB
Größe: 640 Bytes

Header (Auszug):
39 30 4A 41 48 52 45 00   00 00 00 00 00 00 00 00   90JAHRE. ........ 
03 00 03 DA 04 70 03 00   00 00 00 00 00 00 00 00   .....p.. ........ 

Header-Informationen
Name:           90JAHRE    
Anzahl Adr.:    3
Anfangsadr.:    0300h
Endeadr(+1):    04DAh
Startadr.:      0370h
Programmgröße:    474 Bytes
ungenutzt:      38 Byte(s) (am Ende)
min. 16 kByte RAM benötigt (RAM0).

INFO: BASIC-Arbeitszellen vorhanden.
Startcode für BASIC-Start:
DB 88 F6 80 E6 FB D3 88   DD 7E 04 F6 60 D3 86 DD   ........ .~..`... 
77 04 3E 89 32 62 03 C3   1F C4 00 00 00 00 00 00   w.>.2b.. ........ 
00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00   ........ ........ 
00 00 00 00 00 00 00 00   00 00 00                  ........ ...

BASIC-Programminfo:
Adresse 03C4h: 3FFFh (Zeiger für Strings)
Adresse 03B0h: 3FFFh (Speichergröße)
Adresse 0356h: 3EFFh (RAM-Minimum)
Entwickelt mit 16 kByte BASIC-RAM.

```
