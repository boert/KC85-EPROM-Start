//////////////////////////////////////////////////
//
// Dieses Programm generiert EPROM-Inhalte, die
// mit AUTOSTART gestartet werden können
// optional erfolgt der Start per MENU
//
// 8k-Version für z.B. M025 USER PROM 8K (umgebaut auf Strkuturbyte 01h)
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
#embed "auto_start_8k.rom"
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
    std::println( "wenn folgende Bedingungen erfüllt sind:");
    std::println( " - das Modul hat das Strukturbyte 01h");
    std::println( " - das Modul steckt im Steckplatz 08 des Grundgerätes");
    std::println();
    std::println( "Alternativ kann die Software nach dem aktivieren des Moduls mit ");
    std::println( "    %SWITCH <Modulschacht> C1");
    std::println( "    und dem Menüeintrag");
    std::println( "    %START");
    std::println( "gestartet werden.");
    std::println();
    std::println();
    std::println( "Geeignet für die Kleincomputer KC85/3, KC85/4 und KC85/5.");
    std::println( "Benötigt wird ein ROM- bzw. EPROM-Modul mit 8 kByte Segmenten.");
    std::println();
    std::println( "Folgende Module sind geeignet:");
    std::println( "    M025  USER PROM 8K (modifiziert)");
    std::println( "    M045  32k segmented ROM (gejumpert auf 01)");
    std::println( "    M046  64k segmented ROM (gejumpert auf 01)");
    std::println( "    M047  128k segmented ROM (gejumpert auf 01)");
    std::println( "    M062  32k/64k seg. RAM/ROM (gejumpert auf 01)");
    std::println();
    std::println( "Achtung! Limitierung der Programmgröße auf 8 kByte,");
    std::println( "abzüglich des Hilfsprogrammes (ca. 180 Bytes).");
    std::println();
    std::println( "Programmaufruf:");
    std::println( "{} [-o|-v] [-m START] <KCC-Datei> <ROM-Datei>", self_name);
    std::println();
    std::println( "Programmptionen:");
    std::println( "-o   evtl. vorhandene ROM-Datei überschreiben");
    std::println( "-m   Menüwort (Standard: START)");
    std::println( "-v   Programmversion ausgeben");
    std::println();
}

// Version ausgeben
void version( void)
{
    std::println( "Version für 8k ROMs");
    std::println( "mit Strukturbyte 01h");
    std::println( "Autostart oder Start mit Menüwort");
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
void convert_KCC_file( std::string kcc_filename, std::string rom_filename, std::string menuwort)
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


    // wird haben Platz:
    // Block 1: C0xx-DFFF   ~8100 Bytes

    int max_prog_size = 8192 - rom_prog.size();

    // Größe prüfen
    if( mem_data.size() > max_prog_size)
    {
        std::println( "FEHLER: KCC-Datei ({}) leider zu groß!", filename_short);
        std::println( "verfügbarer Speicher: {}  Bytes", max_prog_size);
        std::println( "benötigter Speicher: {}  Bytes", header.prog_size);
        exit( EXIT_FAILURE);
    }

    // ROM anlegen
    std::vector<uint8_t> rom( 8192, 0xff);
    const uint16_t rom_offset = 0xc000;

    std::println();
    std::println( "ROM-Informationen");
    std::println( "ROM-Größe: {} Bytes", rom.size());
    std::println( "Hilfsprog: {} Bytes", rom_prog.size());
    std::println( "verfügbar: {} Bytes", max_prog_size);

    std::println();
    std::println( "Erzeuge ROM-Datei: {}", rom_filename);

    // Menüworte präparieren
    const int max_prepare_menu = 3;
    int prepare_menu = ( header.menu_entry.size() > max_prepare_menu) ? max_prepare_menu : header.menu_entry.size();
    while( prepare_menu > 0)
    {
        write_mem( mem_data, header.menu_entry[ prepare_menu - 1].prolog, (uint8_t) 0xFF);
        prepare_menu--;
    }

    uint16_t block1_start;
    uint16_t block1_length;

    // ASM-Programm in ROM kopieren
    std::copy_n( rom_prog.begin(), rom_prog.size(), rom.begin() + ( 0xC000 - rom_offset));
    int block1_max_size = rom.size() - rom_prog.size();

    // 1 Block
    block1_start = rom_prog.size();
    block1_length = mem_data.size();
    std::copy_n( mem_data.begin(), block1_length, rom.begin() + block1_start);

    block1_start += rom_offset;

    // Update Kopierinformationen
    // ab 0xC000h
    // Adresse müssen zum asm-Programm passen
    write_mem( rom, prg_dest    - rom_offset, header.loadaddr);
    write_mem( rom, prg_start   - rom_offset, header.startaddr);
    write_mem( rom, bl1_start   - rom_offset, block1_start);
    write_mem( rom, bl1_size    - rom_offset, block1_length);

    // Update Menüwortinformationen
    prepare_menu = ( header.menu_entry.size() > max_prepare_menu) ? max_prepare_menu : header.menu_entry.size();
    write_mem( rom, menu_cnt - rom_offset, (uint8_t) prepare_menu);
    uint16_t prolog_addr = menu_addr - rom_offset;
    while( prepare_menu > 0)
    {
        write_mem( rom, prolog_addr, (uint16_t)( header.menu_entry[ prepare_menu - 1].prolog + header.loadaddr));
        prepare_menu--;
        prolog_addr += 2;
    }

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
    std::println( "Block 1: {:04X}h...{:04X}h", block1_start, block1_start + block1_length - 1);
    std::println( "Menüwort: {}", menuwort);
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
    while(( c = getopt( argc, argv, "hvom:")) != -1)
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

#if 0
    std::println( "Argumente ausgewertet: ");
    std::println( "KCC-Datei: {}", kcc_filename);
    std::println( "ROM-Datei: {}", rom_filename);
    std::println( "Menüwort: {}", menuwort);
    std::println( "Überschreiben: {}", overwrite);
    std::println();
    exit( EXIT_SUCCESS);
#endif

    convert_KCC_file( kcc_filename, rom_filename, menuwort);
}
