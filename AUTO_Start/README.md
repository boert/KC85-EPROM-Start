# `AUTO_Start`

Mit diesem Programm läßt sich eine KCC (bzw. KCB)-Datei in ein startfähiges ROM umwandeln.
Geeignet für die Kleincomputer KC85/3, KC85/4 und KC85/5.

Der Start im KC85 erfolgt automatisch, wenn das Modul im Schacht 8 geteckt wird und das Strukturbyte 01h besizt.

Falls ein anderer Schacht genutzt wird oder das Modul nicht das Strukturbyte 01h hat, kann das Programm auch wie folgt gestartet werden:

    `%SWITCH <Modulschacht> C1`

    und

    `%START`

Es wird ein ROM- bzw. EPROM-Modul mit 8 oder 16 kByte Segmenten benötigt.  
Folgende 8 kByte-Module sind geeignet, wenn das Strukturbyte auf 01h gejumpert wird:

- M045  32k segmented ROM
- M046  64k segmented ROM
- M047  128k segmented ROM
- M062  32k/64k seg. RAM/ROM

Die folgenden 8 kByte-Module erfordern eine Modifikation der Hardware auf das Strukturbyte 01h:
- M025  USER PROM 8K
- M125  USER PROM 8K/16K/64K

Die folgenden 16k-Byte Module erfordern eine Modifikation der Hardware auf das Strukturbyte 01h:
- M028  16k EPROM
- M040  USER PROM 16K
- M048  256k segmented ROM

> [!WARNING]
> Limitierung der Programmgröße auf 8 bzw. 16 kByte,
abzüglich des Hilfsprogrammes (ca. 200 Bytes).

Die erzeugte ROM-Datei muß im ersten Segement gespeichert werden, damit Autostart funktioniert.

## Programmaufruf
```
./AUTO_Start [-o|-v] [-m MENUWORT] -s <Segmentgröße> <KCC-Datei> <ROM-Datei>
```
Programmptionen:  
-s n Segmentgröße n kByte, (n = 8 oder 16)  
-o   evtl. vorhandene ROM-Datei überschreiben  
-m   neues Menüwort angeben (Standard: START)  
-v   Programmversion ausgeben  

## Beispielaufruf

```
./AUTO_Start  -s 16  DELIRO.KCC  DELIRO.ROM

Lese KCC-Datei: DELIRO.KCC
Größe: 16000 Bytes

Header-Informationen
Name:           DELIRO     
# Adressen:     3
Anfangsadr.:    0200h
Endeadr(+1):    4000h
Startadr.:      2D00h
Programmgröße:  15872 Bytes
Menüeinträge:
 1: DELIRO     -> 31D3h

ROM-Informationen
ROM-Größe: 16384 Bytes
Hilfsprog: 188 Bytes
verfügbar: 16196 Bytes

Erzeuge ROM-Datei: DELIRO.ROM
Block 1: C0BCh...FEBBh
Menüwort: START
frei: 324 Bytes
```
