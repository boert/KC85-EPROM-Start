//////////////////////////////////////////////////
//
// Dieses Programm generiert EPROM-Inhalte, die
// mit JUMP,
// per MENÜ oder
// per Autostart gestartet werden können
//
// Version für 8k und 16k-ROMs
//
// (c) 2025, Bert Lange
//////////////////////////////////////////////////
#include <cstdint>      // uint8_t, uint16_t
#include <print>
#include <fstream>      // ifstream, ofstream
#include <filesystem>   // short filename
#include <iterator>     // ostream_iterator
#include <vector>
#include <cctype>       // isprint
#include <string>
#include <unistd.h>     // getopt
#include <string.h>     // strcmp

extern "C" {
#include "ZX0/zx0.h"
}

#include "rom.h"        // Symbole aus ROM-Datei

//////////////////////////////////////////////////
// ROM-Datei einbinden
const std::vector<uint8_t> rom_prog
{
#embed "multi_start_zx0.rom"
};


//////////////////////////////////////////////////
typedef struct
{
    std::string entry;          // Zeichenkette
    uint16_t    address;        // Startadresse
    uint16_t    prolog;         // Beginn vom Prolog
} menu_entry_s;

typedef struct
{
    std::string                 name;
    uint8_t                     addrargs;
    uint16_t                    loadaddr;
    uint16_t                    endaddr;
    uint16_t                    startaddr;
    uint16_t                    prog_size;
    std::vector<menu_entry_s>   menu_entry;
} kcc_header_s;



//////////////////////////////////////////////////
// Hilfetext ausgeben
void help(  const std::string& self_name)
{
    std::println();
    std::println( "Konverter, um eine KCC/KCB-Datei in ein startfähiges ROM umzuwandeln.");
    std::println( "Der Start erfolgt automatisch beim Einschalten,");
    std::println( "wenn beide Bedingungen erfüllt sind:");
    std::println( "1. das Modul hat das Strukturbyte 01h");
    std::println( "2. das Modul steckt im Steckplatz 08 des Grundgerätes");
    std::println();
    std::println( "Der Start kann auch durch JUMP initiiert werden:");
    std::println( "    %JUMP <Modulschacht>");
    std::println();
    std::println( "Alternativ kann der Start nach dem aktivieren des Moduls");
    std::println( "    %SWITCH <Modulschacht> C1");
    std::println( "    per Menüeintrag gestartet werden:");
    std::println( "    %START");
    std::println();
    std::println();
    std::println( "Geeignet für die Kleincomputer KC85/3, KC85/4 und KC85/5.");
    std::println( "Benötigt wird ein ROM- bzw. EPROM-Modul mit 8 oder 16 kByte Segmenten.");
    std::println();
    std::println( "Folgende Module sind geeignet:");
    std::println( "    M028  16 K EPROM");
    std::println( "    M040  USER PROM 16k");
    std::println( "    M048  256k segmented ROM");
    std::println( "Um das Strukturbyte auf 01h einzustellen ist ggf. eine Hardwaremodifikation nötig.");
    std::println();
    std::println( "Achtung! Limitierung der Programmgröße auf 10 bis 16 kByte bzw. 20 bis 32 kByte,");
    std::println( "abhängig von der Kompressionsrate.");
    std::println( "Für JUMP muß die erzeugte ROM-Datei im letzten Segement des Moduls gespeichert werden.");
    std::println( "Für Autostart muß die erzeugte ROM-Datei im ersten Segement des Moduls gespeichert werden.");
    std::println( "Für den Start per Menü kann ein beliebiges Segment gewählt werden.");
    std::println();
    std::println( "Programmaufruf:");
    std::println( "{} [-o|-v] [-m START] -s <Segmentgröße> <KCC-Datei> <ROM-Datei>", self_name);
    std::println();
    std::println( "Programmptionen:");
    std::println( "-s n Segmentgröße n kByte, (n = 8 oder 16)");
    std::println( "-o   evtl. vorhandene ROM-Datei überschreiben");
    std::println( "-m   Menüwort (Standard: START)");
    std::println( "-v   Programmversion ausgeben");
    std::println();
}

// Version ausgeben
void version( void)
{
    std::println( "Version für 8k und 16k ROMs");
    std::println( "Autostart, per Menüwort oder mit JUMP");
    std::println( "komprimiert mit ZX0 von Einar Saukas.");
    std::println( "compiled: " __DATE__ " " __TIME__);
}


// Prüft ob Datei existiert (und sich öffnen läßt)
bool exists( const std::string& name)
{
    if( FILE *file = fopen( name.c_str(), "r"))
    {
        fclose( file);
        return true;
    }
    return false;
}


// extrahiert Daten aus KCC-Header
kcc_header_s read_header( const std::vector<uint8_t>& data)
{
    kcc_header_s result;

    // Name + Erweiterung
    for( int index = 0; index < 11; index++)
    {
        char c = data[ index];
        if( std::isprint( c))
        {
            result.name += c;
        }
        else
        {
            result.name += ( c > 0) ? '.' : ' ';
        }
    }

    // Adressargumente
    result.addrargs  = data[ 16];
    result.loadaddr  = data[ 17] + ( data[ 18] << 8);
    result.endaddr   = data[ 19] + ( data[ 20] << 8);
    result.startaddr = data[ 21] + ( data[ 22] << 8);

    result.prog_size = result.endaddr - result.loadaddr;

    return result;
}


uint8_t lo( uint16_t data)
{
    return data & 0xff;
}


uint8_t hi( uint16_t data)
{
    return (data >> 8);
}


//////////////////////////////////////////////////
void write_mem( std::vector<uint8_t>& mem, const uint16_t address, const uint8_t data)
{
    mem[ address+0] = data;
}

void write_mem( std::vector<uint8_t>& mem, const uint16_t address, const uint16_t data)
{
    mem[ address+0] = lo( data);
    mem[ address+1] = hi( data);
}

void write_mem( std::vector<uint8_t>& mem, uint16_t address, const std::vector<uint8_t>& data)
{
    for( auto by : data)
    {
        mem.at( address) = by;
        address++;
    }
}
//////////////////////////////////////////////////

bool isvalid( char value)
{
    if( std::isalnum( value))
    {
        return true;
    }
    if( value == ':')
    {
        return true;
    }

    return false;
}


// sucht im Programmcode nach Menüeinträgen (+Startadresse)
void find_start_addresses( kcc_header_s& header, const std::vector<uint8_t>& data)
{
    int index = 0;
    uint16_t prolog_address = 0;

    header.menu_entry.clear();
    while( index + 4 < data.size()) // 4 = minimaler Prolog
    {
search:
        // Prolog suchen
        if(( data[ index] == 0x7f) && ( data[ index + 1] == 0x7f))
        {
            std::string name;
            prolog_address = index;
            index += 2;
            while(( index + 1 < data.size()) && isvalid( data[ index]))
            {
                name += (char) data[ index];
                index++;
            }

            // valides Epilogbyte?
            if(( data[ index] <= 0x1f) &&( name.size() > 0))
            {
                uint16_t address = index + 1;
                address += header.loadaddr;
                //std::print( "{}: ", address);
                //std::println( "{}", name);
                header.menu_entry.push_back({ name, address, prolog_address});
            }
        }
        index++;
    }
}


// liest von 'richtiger' Speicheradresse
uint8_t get_basic_byte( const std::vector<uint8_t>& kcc, const int real_addr)
{
    int loadaddr = kcc[ 17] + ( kcc[ 18] << 8);
    if( loadaddr > real_addr) return 0xff;

    int addr = real_addr - loadaddr + 0x80;
    if( addr < 0x80)       return 0xff;
    if( addr > kcc.size()) return 0xff;

    uint8_t result = kcc[ addr];
    return result;
}

// liest von 'richtiger' Speicheradresse
uint16_t get_basic_word( const std::vector<uint8_t>& kcc, const int real_addr)
{
    int loadaddr = kcc[ 17] + ( kcc[ 18] << 8);
    if( loadaddr > real_addr) return 0xffff;

    int addr = real_addr - loadaddr + 0x80;
    if( addr < 0x80)        return 0xffff;
    if( addr >= kcc.size()) return 0xffff;

    uint16_t result = kcc[ addr] + ( kcc[ addr+1] << 8);
    return result;
}


// erzeugt formatierte HEX-Ausgabe (Speicerbereich)
void hex_dump( const std::vector< uint8_t>& data, const int start, const int length)
{
    std::string chars;
    int index;
    for( index = 0; index < length; index++)
    {
        std::print( "{:02X} ", data[ start + index]);
        chars += std::isprint( data[ start + index])  ?  (char) data[ start + index] : '.';

        if( index % 8 == 7)
        {
            std::print( "  ");
            chars += ' ';
        }
        if( index % 16 == 15)
        {
            std::println( "{}", chars);
            chars.clear();
        }
    }
    if( index % 16 < 15)
    {
        while( index % 16 > 0)
        {
            if( index % 8 == 7)
            {
                std::print( "  ");
            }
            std::print( "   ");
            index++;
        }
        std::println( "{}", chars);
    }
}


// Datei einlesen und auswerten
void convert_KCC_file( std::string kcc_filename, std::string rom_filename, std::string menuwort, int size)
{
    const std::string filename_short = std::filesystem::path( kcc_filename).filename().string();

    std::println();
    std::println( "Lese KCC-Datei: {}", kcc_filename);

    // Datei einlesen
    std::ifstream input(  kcc_filename, std::ios::binary | std::ios::in);
    std::vector< uint8_t> kcc_data( std::istreambuf_iterator<char>( input), {});

    if( kcc_data.size() < 128)
    {
        std::println( "FEHLER: KCC-Datei ({}) enthält keinen Header.", filename_short);
        exit( EXIT_FAILURE);
    }

    // Statusinformationen
    std::println( "Größe: {} Bytes", kcc_data.size());


    std::println();
    //std::println( "Header (Auszug):");
    //hex_dump( kcc_data, 0, 0x20);

    // Header decodieren
    auto header = read_header( kcc_data);
    // Programmdaten abspalten
    std::vector<uint8_t> mem_data( kcc_data.begin() + 128, kcc_data.begin() + 128 + header.prog_size);
    if( mem_data.size() == 0)
    {
        std::println( "FEHLER: Datei ({}) enthält keine Programmdaten!", filename_short);
        exit( EXIT_FAILURE);
    }
    // Startadressen suchen
    find_start_addresses( header, mem_data);

    std::println( "Header-Informationen");
    std::println( "Name:           {}", header.name);
    std::println( "# Adressen:     {}", header.addrargs);
    if(( header.addrargs < 2) || ( header.addrargs > 0x0A))
    {
        std::println( "FEHLER: Anzahl der Adressargumente ({}) ungültig.", header.addrargs);
        std::println( "Keine gültige KCC-Datei!");
        exit( EXIT_FAILURE);
    }
    std::println( "Anfangsadr.:    {:04X}h", header.loadaddr);
    std::println( "Endeadr(+1):    {:04X}h", header.endaddr);
    if( header.addrargs > 2)
    {
        std::println( "Startadr.:      {:04X}h", header.startaddr);
    }
    else
    {
        std::println( "FEHLER: Keine Startadresse hinterlegt, bitte KCC-Datei korrigieren.");
        exit( EXIT_FAILURE);
    }
    std::println( "Programmgröße:  {} Bytes", header.prog_size);
    if(( header.prog_size + 128) > kcc_data.size())
    {
        std::println( "FEHLER: KCC-Datei ({}) ist kleiner als im Header beschrieben.", filename_short);
        std::print(   "erwarte: {} Bytes", header.prog_size + 0x80);
        std::println( ", aber die Datei hat nur {} Bytes", kcc_data.size());
        exit( EXIT_FAILURE);
    }

    // Zielspeicher prüfen
    if(( header.loadaddr >= 0xE000) && ( header.loadaddr <= 0xFFFF))
    {
        std::println( "FEHLER: KCC-Datei ({}) direkt für ROM E!", filename_short);
        exit( EXIT_FAILURE);
    }

    if(( header.loadaddr == 0xC000) && ( header.loadaddr <= 0xDFFF))
    {
        std::println( "FEHLER: KCC-Datei ({}) direkt für ROM C!", filename_short);
        exit( EXIT_FAILURE);
    }

    // Menüeinträge ausgeben
    if( header.menu_entry.size() > 0)
    {
        std::println( "Menüeinträge:");
        int index = 1;
        for( auto e: header.menu_entry)
        {
            std::println( "{:2}: {:10} -> {:04X}h", index, e.entry, e.address);
            index++;
        }
    }

    // Programmaten komprimieren
    std::println();
    std::println( "Komprimiere (ZX0)");
    uint8_t *compress_ptr;
    int compressed_size;
    int delta;
    const int skip = 0;
    const int backwards_mode = 0;
    compress_ptr = compress( optimize( mem_data.data(), mem_data.size(), 0, 32640),  mem_data.data(), mem_data.size(), skip, backwards_mode, 1, &compressed_size, &delta);

    //hex_dump( mem_data, 0, 32);
    std::vector<uint8_t> mem_data_compressed( compress_ptr, compress_ptr + compressed_size);
    //hex_dump( mem_data_compressed, 0, 32);


    // wird haben Platz:
    // 16k
    // Block 1: F0xx-FFFF   ~3968 Bytes
    // Block 2: C000-F011   12228 Bytes
    //
    // 8k
    // Block 1: D0xx-DFFF   ~3968 Bytes
    // Block 2: C000-D011    4036 Bytes

    int max_prog_size = size * 1024 - rom_prog.size();

    std::println( "Progamm (komprimiert): {}", mem_data_compressed.size());
    std::println( "Kompressionsfaktor:    {:.1f}%", 100.0 * mem_data_compressed.size() / mem_data.size());

    // Größe prüfen (komprimiert)
    if( mem_data_compressed.size() > max_prog_size)
    {
        std::println( "FEHLER: KCC-Datei ({}) leider zu groß!", filename_short);
        std::println( "verfügbarer Speicher: {}  Bytes", max_prog_size);
        std::println( "benötigter Speicher (komprimiert): {}  Bytes", mem_data_compressed.size());
        exit( EXIT_FAILURE);
    }

    // ROM anlegen
    std::vector<uint8_t> rom( size * 1024, 0xff);
    const uint16_t rom_offset = 0xc000;

    std::println();
    std::println( "ROM-Informationen");
    std::println( "ROM-Größe:  {} Bytes", rom.size());
    std::println( "Hilfsprog.: {} Bytes", rom_prog.size());
    std::println( "verfügbar:  {} Bytes", max_prog_size);

    std::println();
    std::println( "Erzeuge ROM-Datei  {}", rom_filename);

    uint16_t block_1_length;
    uint16_t block_2_length;

    // ASM-Programm in ROM kopieren
    // Teil 1 ab 0C000h
    int part_1_size = pr_end - rom_offset;
    //std::println( "part_1_size: {}", part_1_size);
    std::copy_n( rom_prog.begin(), part_1_size, rom.begin());

    // Teil 2 ab 0F012h (bzw. 0D012h)
    uint16_t part_2_src = part_1_size;
    uint16_t part_2_dest = jump_prog - rom_offset;
    part_2_dest -= ( size == 16) ? 0: 0x2000;
    uint16_t part_2_size = rom_prog.size() - part_1_size;
    std::copy_n( rom_prog.begin() + part_2_src, part_2_size, rom.begin() + part_2_dest);
    if( size == 8)
    {
        if( rom[ 0x1012] != 0x21)
        {
            std::println( "FEHLER: Hilfsprogramm (Teil 2) modifiziert! Bitte Programmautor kontaktieren.");
            exit( EXIT_FAILURE);
        }
        // patch source address for helper program
        // on 8k rom modules
        // necessary for JUMP target
        rom[ 0x1014] += 0x20;
    }

    uint16_t block_1_start = part_1_size;
    uint16_t block_1_max_size = part_2_dest - part_1_size;

    uint16_t block_2_start = part_2_dest + part_2_size;
    uint16_t block_2_max_size = rom.size() - block_2_start;

    if( mem_data_compressed.size() > block_1_max_size)
    {
        // beide Blöcke
        block_1_length = block_1_max_size;
        std::copy_n( mem_data_compressed.begin(), block_1_length, rom.begin() + block_1_start);

        block_2_length = mem_data_compressed.size() - block_1_length;
        std::copy_n( mem_data_compressed.begin() + block_1_length, block_2_length, rom.begin() + block_2_start);

    }
    else
    {
        // nur ein Block
        block_1_length = mem_data_compressed.size();
        std::copy_n( mem_data_compressed.begin(), block_1_length, rom.begin() + block_1_start);

        block_2_length = 0;
    }

    block_1_start += rom_offset;
    block_2_start += rom_offset;

    // Update Kopierinformationen
    // ab 0xC0xxh
    // Adresse müssen zum asm-Programm passen
    uint16_t comp_addr = delta + header.endaddr - mem_data_compressed.size();
    write_mem( rom, prg_dest    - rom_offset, comp_addr);         // prg_dest
    write_mem( rom, prg_start   - rom_offset, header.startaddr);
    write_mem( rom, decomp_addr - rom_offset, header.loadaddr);   // decomp_addr
    write_mem( rom, prg_args    - rom_offset, header.addrargs);
    write_mem( rom, bl1_start   - rom_offset, block_1_start);
    write_mem( rom, bl1_size    - rom_offset, block_1_length);
    write_mem( rom, bl2_start   - rom_offset, block_2_start);
    write_mem( rom, bl2_size    - rom_offset, block_2_length);

    // Menüwort behandeln
    // zu lang
    const int menuwort_max_size = 10;
    if( menuwort.size() > menuwort_max_size)
    {
        std::println( "HINWEIS: Menüwort wird auf {} Zeichen begrenzt.", menuwort_max_size);
        menuwort.resize( menuwort_max_size);
    }
    // zu kurz
    if( menuwort.size() < 1)
    {
        std::println( "FEHLER: Menüwort zu kurz.");
        exit( EXIT_FAILURE);
    }
    // enthält Kleinbuchstaben
    if( std::any_of( menuwort.begin(), menuwort.end(), ::islower))
    {
        std::println( "HINWEIS: Menüwort wird versteckt (enthält Kleinbuchstaben).");
    }

    // Epilogbyte anhängen
    menuwort.push_back( (char)0x01);
    // in ROM einfügen
    std::copy_n( menuwort.begin(), menuwort.size(), rom.begin() + mwort - rom_offset);
    // Epilogbyte entfernen
    menuwort.pop_back();

    // Statusinformationen
    std::println( "Hilfsprog. 1:  {:04X}h...{:04X}h", rom_offset, rom_offset + part_1_size - 1);
    std::println( "Block 1:       {:04X}h...{:04X}h", block_1_start, block_1_start + block_1_length - 1);
    std::println( "Hilfsprog. 2:  {:04X}h...{:04X}h", rom_offset + part_2_dest, rom_offset + part_2_dest + part_2_size - 1);
    if( block_2_length > 0)
    {
        std::println( "Block 2:       {:04X}h...{:04X}h", block_2_start, block_2_start + block_2_length - 1);
    }
    else
    {
        std::println( "Block 2:       ungenutzt");
    }
    std::println( "Menüwort:      {}", menuwort);
    std::println( "frei:          {} Bytes", max_prog_size - mem_data_compressed.size());

    // Datei abspeichern
    std::ofstream output( rom_filename, std::ios::binary | std::ios::out);
    std::copy( rom.begin(), rom.end(), std::ostream_iterator<uint8_t>( output));
}


//////////////////////////////////////////////////
int main( int argc, char** argv)
{

    const std::string self_name = argv[ 0];
    bool overwrite = false;
    int romsize = 0;
    int c;
    std::string kcc_filename;
    std::string rom_filename;
    std::string menuwort = "START";

#if 0
    std::print( "Argumente ({}): ", argc);
    for( int index = 0; index < argc; index++)
    {
        std::print( "{} ", argv[ index]);
    }
    std::println();
#endif

    // Argumente auswerten
    while(( c = getopt( argc, argv, "hvom:s:")) != -1)
    {
        switch( c)
        {
            case 'h':
            case '?':
                help( self_name);
                exit( EXIT_SUCCESS);
                break;

            case 'v':
                version();
                exit( EXIT_SUCCESS);
                break;

            case 'o':
                overwrite = true;
                break;

            case 'm':
                //std::println( "optarg: {}", optarg);
                menuwort = optarg;
                break;

            case 's':
                //std::println( "optarg: {}", optarg);
                if( strcmp( "8", optarg) == 0)
                {
                    romsize = 8;
                }
                if( strcmp( "16", optarg) == 0)
                {
                    romsize = 16;
                }
                break;
        }
    }

    // freie Argumente
    // erster Dateiname
    if( optind < argc)
    {
        kcc_filename = argv[ optind++];
    }
    else
    {
        help( self_name);
        exit( EXIT_SUCCESS);
    }

    // zweiter Dateiname
    if( optind < argc)
    {
        rom_filename = argv[ optind++];
    }
    else
    {
        std::println( "FEHLER: Keine ROM-Datei angegeben.");
        exit( EXIT_FAILURE);
    }

    if( kcc_filename == rom_filename)
    {
        std::println( "FEHLER: KCC-Datei und ROM-Datei identisch.");
        exit( EXIT_FAILURE);
    }

    // Anwesenheit prüfen
    if( !exists( kcc_filename))
    {
        std::println( "FEHLER: KCC-Datei ({}) nicht gefunden.", kcc_filename);
        exit( EXIT_FAILURE);
    }

    // Verzeichnis?
    if( std::filesystem::is_directory( kcc_filename))
    {
        std::println( "FEHLER: {} ist ein Verzeichnis und keine KCC-Datei!", kcc_filename);
        exit( EXIT_FAILURE);
    }

    // Zieldatei vorhanden?
    if( exists( rom_filename) && ( overwrite == false))
    {
        std::println( "FEHLER: ROM-Datei ({}) schon vorhanden.", rom_filename);
        std::println( "Option '-o' zum Überschreiben.");
        exit( EXIT_FAILURE);
    }

    // Segmentgröße ausgewählt?
    if( romsize == 0)
    {
        std::println( "FEHLER: Keine gültige Segmentgröße angegeben.");
        std::println( "Option '-s  8' für  8 kByte Segmentgröße");
        std::println( "Option '-s 16' für 16 kByte Segmentgröße");
        exit( EXIT_FAILURE);
    }

    convert_KCC_file( kcc_filename, rom_filename, menuwort, romsize);
}
