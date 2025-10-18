//////////////////////////////////////////////////
//
// Dieses Programm generiert EPROM-Inhalte, die
// mit JUMP gestartet werden können
//
// 16k-Version für z.B. M028 16 K EPROM
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


#include "rom.h"        // Symbole aus ROM-Datei

//////////////////////////////////////////////////
// ROM-Datei einbinden
const std::vector<uint8_t> rom_prog
{
#embed "jump_start_16k.rom"
};


//////////////////////////////////////////////////
typedef struct
{
    std::string                 name;
    uint8_t                     addrargs;
    uint16_t                    loadaddr;
    uint16_t                    endaddr;
    uint16_t                    startaddr;
    uint16_t                    prog_size;
} kcc_header_s;



//////////////////////////////////////////////////
// Hilfetext ausgeben
void help(  const std::string& self_name)
{
    std::println();
    std::println( "Konverter, um eine KCC/KCB-Datei in ein startfähiges ROM umzuwandeln.");
    std::println( "Der Start erfolgt mit");
    std::println();
    std::println( "    JUMP <Modulschacht>");
    std::println();
    std::println();
    std::println( "Geeignet für die Kleincomputer KC85/3, KC85/4 und KC85/5.");
    std::println( "Benötigt wird ein ROM- bzw. EPROM-Modul mit 16 kByte Segmenten.");
    std::println();
    std::println( "Folgende Module sind geeignet:");
    std::println( "    M028  16 K EPROM");
    std::println( "    M040  USER PROM 16k");
    std::println( "    M048  256k segmented ROM");
    std::println();
    std::println( "Achtung! Limitierung der Programmgröße auf 16 kByte,");
    std::println( "abzüglich des Hilfsprogrammes (ca. 80 Bytes).");
    std::println( "Die erzeugte ROM-Datei muß im letzten Segement gespeichert werden.");
    std::println();
    std::println( "Programmaufruf:");
    std::println( "{} [-o|-v] <KCC-Datei> <ROM-Datei>", self_name);
    std::println();
    std::println( "Programmptionen:");
    std::println( "-o   evtl. vorhandene ROM-Datei überschreiben");
    std::println( "-v   Programmversion ausgeben");
    std::println();
}

// Version ausgeben
void version( void)
{
    std::println( "Version für 16k ROMs");
    std::println( "Start mit JUMP");
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
void convert_KCC_file( std::string kcc_filename, std::string rom_filename)
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


    // wird haben Platz:
    // Block 1: F0xx-FFFF   ~3968 Bytes
    // Block 2: C000-F000   12228 Bytes

    int max_prog_size = 16384 - rom_prog.size();

    // Größe prüfen
    if( mem_data.size() > max_prog_size)
    {
        std::println( "FEHLER: KCC-Datei ({}) leider zu groß!", filename_short);
        std::println( "verfügbarer Speicher: {}  Bytes", max_prog_size);
        std::println( "benötigter Speicher: {}  Bytes", header.prog_size);
        exit( EXIT_FAILURE);
    }

    // ROM anlegen
    std::vector<uint8_t> rom( 16384, 0xff);
    const uint16_t rom_offset = 0xc000;

    std::println();
    std::println( "ROM-Informationen");
    std::println( "ROM-Größe: {} Bytes", rom.size());
    std::println( "Hilfsprog: {} Bytes", rom_prog.size());
    std::println( "verfügbar: {} Bytes", max_prog_size);

    std::println();
    std::println( "Erzeuge ROM-Datei: {}", rom_filename);

    uint16_t block1_start;
    uint16_t block1_length;
    uint16_t block2_start;
    uint16_t block2_length;

    // ASM-Programm in ROM kopieren
    std::copy_n( rom_prog.begin(), rom_prog.size(), rom.begin() + ( 0xf000 - rom_offset));
    int block1_max_size = 4096 - rom_prog.size();

    if( mem_data.size() > block1_max_size)
    {
        // 2 Blöcke
        block1_start = 0x3000 + rom_prog.size();
        block1_length = block1_max_size;
        std::copy_n( mem_data.begin(), block1_length, rom.begin() + block1_start);

        block2_start = 0x0000;
        block2_length = mem_data.size() - block1_length;
        std::copy_n( mem_data.begin() + block1_length, block2_length, rom.begin() + block2_start);
    }
    else
    {
        // 1 Block
        block1_start = 0x3000 + rom_prog.size();
        block1_length = mem_data.size();
        std::copy_n( mem_data.begin(), block1_length, rom.begin() + block1_start);

        block2_length = 0;
        block2_start = 0;
    }

    block1_start += rom_offset;
    block2_start += rom_offset;

    // Update Kopierinformationen
    // ab 0xF000h
    // Adresse müssen zum asm-Programm passen
    write_mem( rom, prg_dest    - rom_offset, header.loadaddr);
    write_mem( rom, prg_start   - rom_offset, header.startaddr);
    write_mem( rom, bl1_start   - rom_offset, block1_start);
    write_mem( rom, bl1_size    - rom_offset, block1_length);
    write_mem( rom, bl2_start   - rom_offset, block2_start);
    write_mem( rom, bl2_size    - rom_offset, block2_length);

    // Statusinformationen
    std::println( "Block 1: {:04X}h...{:04X}h", block1_start, block1_start + block1_length - 1);
    std::println( "Block 2: {:04X}h...{:04X}h", block2_start, block2_start + block2_length - 1);
    std::println( "frei: {} Bytes", max_prog_size - mem_data.size());

    // Datei abspeichern
    std::ofstream output( rom_filename, std::ios::binary | std::ios::out);
    std::copy( rom.begin(), rom.end(), std::ostream_iterator<uint8_t>( output));
}


//////////////////////////////////////////////////
int main( int argc, char** argv)
{

    const std::string self_name = argv[ 0];
    bool overwrite = false;
    int c;
    std::string kcc_filename;
    std::string rom_filename;

#if 0
    std::print( "Argumente ({}): ", argc);
    for( int index = 0; index < argc; index++)
    {
        std::print( "{} ", argv[ index]);
    }
    std::println();
#endif

    // Argumente auswerten
    while(( c = getopt( argc, argv, "hvo")) != -1)
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

    convert_KCC_file( kcc_filename, rom_filename);
}
