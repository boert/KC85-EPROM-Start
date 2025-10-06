# `AUTO_Start_8k`

Mit diesem Programm läßt sich eine KCC (bzw. KCB)-Datei in ein startfähiges ROM umwandeln.
Geeignet für die Kleincomputer KC85/3, KC85/4 und KC85/5.

Der Start im KC85 erfolgt automatisch, wenn das Modul im Schacht 8 geteckt wird.

Falls ein anderer Schacht genutzt wird oder das Modul nicht das Strukturbyte 01h hat, kann das Programm auch wie folgt gestartet werden:

    `%SWITCH <Modulschacht> C1`

    und

    `%START`

Es wird ein ROM- bzw. EPROM-Modul mit 8 kByte Segmenten benötigt.  
Folgende Module sind geeignet, wenn das Strukturbyte auf 01h gejumpert wird:

- M045  32k segmented ROM
- M046  64k segmented ROM
- M047  128k segmented ROM
- M062  32k/64k seg. RAM/ROM

Die folgenden Module erfordern eine Modifikation der Hardware auf das Strukturbyte 01h:
- M025  USER PROM 8K
- M125  USER PROM 8K/16K/64K

> [!WARNING]
> Limitierung der Programmgröße auf 8 kByte,
abzüglich des Hilfsprogrammes (ca. 180 Bytes).

Die erzeugte ROM-Datei muß im ersten Segement gespeichert werden, damit Autostart funktioniert.

## Programmaufruf
```
./AUTO_Start_8k [-o|-v] [-m MENUWORT] <KCC-Datei> <ROM-Datei>
```
Programmptionen:  
-o   evtl. vorhandene ROM-Datei überschreiben  
-m   neues Menüwort angeben (Standard: START)  
-v   Programmversion ausgeben  

## Beispielaufruf

```
./AUTO_Start_8k HELI-4.KCC HELI4.ROM

Lese KCC-Datei: HELI-4.KCC
Größe: 6400 Bytes

Header-Informationen
Name:           HELI    KCC
# Adressen:     3
Anfangsadr.:    07A0h
Endeadr(+1):    2000h
Startadr.:      0D21h
Programmgröße:  6240 Bytes
Menüeinträge:
 1: HELI       -> 0D07h

ROM-Informationen
ROM-Größe: 8192 Bytes
verfügbar: 8018 Bytes

Erzeuge ROM-Datei: HELI4.ROM
Block 1: C0AEh...D90Dh
Menüwort: START
frei: 1778 Bytes
```
