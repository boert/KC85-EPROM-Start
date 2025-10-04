# Startfähige ROM/EPROM-Dateien für KC85 erstellen
Dieses Projekt enthält verschiedene Hilfsprogramme (für Linux und Windows), um startfähige ROM/EPROM-Dateien zu erstellen.
Als Basis dient jeweils eine KCC-Datei[^1] oder eine KCB-Datei[^2].

Die jeweiligen Programm werden grundsätzlich in der RAM umkopiert und ggf. von dort gestartet.
ROM-fähige Programme (wie z.B. 

Es werden unterschiedliche Modultypen und verschiedene Startmethoden unterstützt:

| Modul                         | Struktur-<br>byte  | Segment-<br>größe | Segmente | JUMP 8k | JUMP 16k | MENU 8k | MENU 16k
| :---                          | :---:              | :---:             | :---:    | :---:   | :---:    | :---:   | :---:
| M025  USER PROM 8K            | F7                 |  8 kByte          | 1        | ja      | nein     | ja      | nein
| M028  16 K EPROM              | F8                 | 16 kByte          | 1        | nein    | ja       | nein    | ja
| M040  USER PROM 16k           | F8                 | 16 kByte          | 1        | nein    | ja       | nein    | ja
| M045  32k segmented ROM       | 70                 |  8 kByte          | 4        | ja      | nein     | ja      | nein
| M046  64k segmented ROM       | 71                 |  8 kByte          | 8        | ja      | nein     | ja      | nein
| M047  128k segmented ROM      | 72                 |  8 kByte          | 16       | ja      | nein     | ja      | nein
| M048  256k segmented ROM      | 73                 | 16 kByte          | 16       | nein    | ja       | nein    | ja
| M062  32k/64k seg. RAM/ROM    | F3                 |  8 kByte          | 4/8      | ja      | nein     | ja      | nein
| M125  USER PROM 8K/16K/64K    |                    |  8 kByte          | 1-4      | ja      | nein     | ja      | nein

## Für Module mit 8k-Segmenten

### `JUMP_Start_8k`
für ROM/EPROM-Module mit 8 kByte-Segmenten, Start mittels JUMP

### `MENU_Start_8k`
für ROM/EPROM-Module mit 8 kByte-Segmenten, Start mittels MENU


## Für Module mit 16k-Segmenten

## `JUMP_Start_16k`
für ROM/EPROM-Module mit 16 kByte-Segmenten, Start mittels JUMP

## `MENU_Start_16k`
für ROM/EPROM-Module mit 16 kByte-Segmenten, Start mittels MENU

# Hilfsprogramme

## Binärdateien laden
siehe [BINLOAD](BINLOAD).

## KCC-Dateien überprüfen
siehe [check_KCC](check_KCC).

[^1]: CAOS Maschinenprogramm  
[^2]: BASIC-Programm, abgespeichert als Maschinenprogramm
