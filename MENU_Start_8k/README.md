# `MENU_Start_8k`

Mit diesem Programm läßt sich eine KCC (bzw. KCB)-Datei in ein startfähiges ROM umwandeln.
Geeignet für die Kleincomputer KC85/3, KC85/4 und KC85/5.

Der Start im KC85 erfolgt mit

    `%SWITCH <Modulschacht> C1`

    und

    `%START`

> [!WARNING]
> Beim KC85/3 muß erst der BASIC-ROM deaktiviert werden:
> `%SWITCH 2 0`

Es wird ein ROM- bzw. EPROM-Modul mit 8 kByte Segmenten benötigt.  
Folgende Module sind geeignet (Auswahl):

- M025  USER PROM 8K
- M045  32k segmented ROM
- M046  64k segmented ROM
- M047  128k segmented ROM
- M062  32k/64k seg. RAM/ROM
- M125  USER PROM 8K/16K/64K

> [!WARNING]
> Limitierung der Programmgröße auf 8 kByte,
abzüglich des Hilfsprogrammes (<80 Bytes).

Die erzeugte ROM-Datei belegt ein Segement.
Falls das Modul mehrere Speichersegmente unterstützt, können unterschiedliche Programme in unterschiedlichen Segementen gespeichert werden.
Die Auswahl des jeweiligen Segments erfolgt beim Kommando `SWITCH`.

## Programmaufruf
```
./MENU_Start_8k [-o|-v] [-m MENUWORT] <KCC-Datei> <ROM-Datei>
```
Programmptionen:  
-o   evtl. vorhandene ROM-Datei überschreiben  
-m   neues Menüwort angeben (Standard: START)  
-v   Programmversion ausgeben  

## Beispielaufruf

```
MENU_Start_8k -m EARTH earth.kcc EARTH.ROM

Lese KCC-Datei: earth.kcc
Größe: 7680 Bytes

Header-Informationen
Name:           EARTH      
# Adressen:     3
Anfangsadr.:    2000h
Endeadr(+1):    3D50h
Startadr.:      200Eh
Programmgröße:  7504 Bytes

ROM-Informationen
ROM-Größe: 8192 Bytes
verfügbar: 8134 Bytes

Erzeuge ROM-Datei: EARTH.ROM
Block 1: C03Ah...DD89h
Menüwort: EARTH
frei: 630 Bytes
```

## Test

Vor dem Brennen der EPROMs kann ein Test mit einem RAM-Modul erfolgen.  
Folgende Module mit 8k-Segmenten sind dafür geeignet:

- M062  32k/64k seg. RAM/ROM (mit RAM bestückt)
- M120  8 kB CMOS RAM

Der erzeugte EPROM-Inhalt wird dazu in den Modulspeicher geladen.
Das kann z.B. mit [BINLOAD](../BINLOAD) erfolgen.

Der Start erfolgt auch hier mit

    `%SWITCH <Modulschacht> C1`
    `%START`

