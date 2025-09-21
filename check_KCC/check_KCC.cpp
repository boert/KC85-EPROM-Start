//////////////////////////////////////////////////
//
// Dieses Programm prüft KCC und KCB-Dateien.
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


//////////////////////////////////////////////////
typedef struct
{
    std::string entry;
    uint16_t    address;
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
    std::println( "Programmaufruf:");
    //std::println( "{} [Parameter] <KCC-Datei>", self_name);
    std::println( "{} <KCC-Datei(en)>", self_name);
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

    for( int index = 0; index < 11; index++)
    {
        char c = data[ index];
        if( std::isprint( c))
        {
            result.name += c;
        }
        else
        {
            if( c > 0)
            {
                result.name += '.';
            }
            else
            {
                result.name += ' ';
            }
        }

    }

    result.addrargs  = data[ 16];
    result.loadaddr  = data[ 17] + ( data[ 18] << 8);
    result.endaddr   = data[ 19] + ( data[ 20] << 8);
    result.startaddr = data[ 21] + ( data[ 22] << 8);

    result.prog_size = result.endaddr - result.loadaddr;

    return result;
}


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

    header.menu_entry.clear();
    while( index + 4 < data.size()) // 4 = minimaler Prolog
    {
search:
        // Prolog suchen
        if(( data[ index] == 0x7f) && ( data[ index + 1] == 0x7f))
        {
            std::string name;
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
                header.menu_entry.push_back({ name, address});
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
    if( addr < 0x80)       return 0xffff;
    if( addr >= kcc.size()) return 0xffff;

    uint16_t result = kcc[ addr] + ( kcc[ addr+1] << 8);
    return result;
}


// check if we start with BASIC header
bool check_sss( const std::vector<uint8_t>& kcc)
{
    if( kcc[ 0] != 0xd3) return false;
    if( kcc[ 1] != 0xd3) return false;
    if( kcc[ 2] != 0xd3) return false;
    return true;
}


// check if we start with BASIC header (UUU)
bool check_uuu( const std::vector<uint8_t>& kcc)
{
    if( kcc[ 0] != 0xd5) return false;
    if( kcc[ 1] != 0xd5) return false;
    if( kcc[ 2] != 0xd5) return false;
    return true;
}


// check if we start with BASIC header (WWW)
bool check_www( const std::vector<uint8_t>& kcc)
{
    if( kcc[ 0] != 0xd7) return false;
    if( kcc[ 1] != 0xd7) return false;
    if( kcc[ 2] != 0xd7) return false;
    return true;
}


// check if we wave a tap file with SSS header
bool check_tap( const std::vector<uint8_t>& kcc)
{
    if( kcc[ 0] != 0x01) return false;
    if( kcc[ 1] != 0xd3) return false;
    if( kcc[ 2] != 0xd3) return false;
    return true;
}


// check for basic programm (C3 C08C on address 300h)
bool check_basic( const std::vector<uint8_t>& kcc)
{
    int loadaddr = kcc[ 17] + ( kcc[ 18] << 8);
    if( loadaddr > 0x300) return false;

    int checkaddr = 0x300 - loadaddr + 0x80;
    if( checkaddr + 2 >= kcc.size()) return false;
//  std::println( "caddr: {:04X}", checkaddr);
//  std::println( "laddr: {:04X}", loadaddr);
//  std::println( "[caddr+0]: {:02X}", kcc[ checkaddr+0]);
//  std::println( "[caddr+1]: {:02X}", kcc[ checkaddr+1]);
//  std::println( "[caddr+2]: {:02X}", kcc[ checkaddr+2]);

    if( kcc[ checkaddr + 0] != 0xc3) return false;
    if(( kcc[ checkaddr + 1] != 0x89) && ( kcc[ checkaddr + 1] != 0x8c)) return false;
    if( kcc[ checkaddr + 2] != 0xc0) return false;

    return true;
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
void process_KCC_file( std::string kcc_filename)
{
    const std::string filename_short = std::filesystem::path( kcc_filename).filename().string();

    std::println();
    std::println( "Datei: {}", kcc_filename);

    // Anwesenheit prüfen
    if( !exists( kcc_filename))
    {
        std::println( "FEHLER: KCC-Datei ({}) nicht gefunden.", filename_short);
        exit( EXIT_FAILURE);
    }

    // Verzeichnis?
    if( std::filesystem::is_directory( kcc_filename))
    {
        std::println( "FEHLER: {} ist ein Verzeichnis und keine KCC-Datei!", filename_short);
        exit( EXIT_FAILURE);
    }

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

    // Dateilänge modulo 128
    if(( kcc_data.size() % 128) > 0)
    {
        //std::println( "{}", kcc_data.size() % 128);
        std::println( "WARNUNG: Die Dateilänge ist kein Vielfaches von 128 (Blockgröße).");
        // Kann mit M052 USB Probleme geben.
    }

    std::println();
    std::println( "Header (Auszug):");
    hex_dump( kcc_data, 0, 0x20);

    // BASIC Kennung abprüfen
    if( check_sss( kcc_data))
    {
        std::println( "FEHLER! BASIC-Kennung (SSS) gefunden.");
        std::println( "Keine gültige KCC-Datei!");
        exit( EXIT_FAILURE);
    }

    // TAP + BASIC Kennung abprüfen
    if( check_tap( kcc_data))
    {
        std::println( "FEHLER! TAP-Datei mit BASIC-Kennung (SSS) gefunden.");
        std::println( "Keine gültige KCC-Datei!");
        exit( EXIT_FAILURE);
    }

    // BASIC Kennung abprüfen
    if( check_uuu( kcc_data))
    {
        std::println( "FEHLER! BASIC-Kennung (UUU) gefunden.");
        std::println( "Keine gültige KCC-Datei!");
        exit( EXIT_FAILURE);
    }

    // BASIC Kennung abprüfen
    if( check_www( kcc_data))
    {
        std::println( "FEHLER! BASIC-Kennung (WWW) gefunden.");
        std::println( "Keine gültige KCC-Datei!");
        exit( EXIT_FAILURE);
    }

    // Header decodieren
    auto header = read_header( kcc_data);
    // Programmdaten abspalten
    std::vector<uint8_t> mem_data( kcc_data.begin() + 128, kcc_data.end());
    if( mem_data.size() == 0)
    {
        std::println( "FEHLER: Datei ({}) enthält keine Programmdaten!", filename_short);
        exit( EXIT_FAILURE);
    }
    // Startadressen suchen
    find_start_addresses( header, mem_data);


    std::println( "Header-Informationen");
    std::println( "Name:           {}", header.name);
    std::println( "Anzahl Adr.:    {}", header.addrargs);
    if(( header.addrargs < 2) || ( header.addrargs > 3))
    {
        std::println( "FEHLER: Anzahl der Adressargumente ({}) ungültig.", header.addrargs);
        std::println( "Keine gültige KCC-Datei!");
        exit( EXIT_FAILURE);
    }
    std::println( "Anfangsadr.:    {:04X}h", header.loadaddr);
    std::println( "Endeadr(+1):    {:04X}h", header.endaddr);
    if( header.addrargs == 3)
    {
        std::println( "Startadr.:      {:04X}h", header.startaddr);
    }
    else
    {
        std::println( "(Startadr.)     ({:04X}h)", header.startaddr);
    }
    std::println( "Programmgröße:  {:5} Bytes", header.prog_size);
    if(( header.prog_size + 128) > kcc_data.size())
    {
        std::println( "FEHLER: KCC-Datei ({}) ist kleiner als im Header beschrieben.", filename_short);
        std::print(   "erwarte: {} Bytes", header.prog_size + 0x80);
        std::println( ", aber die Datei hat nur {} Bytes", kcc_data.size());
        exit( EXIT_FAILURE);
    }
    std::println( "ungenutzt:      {} Byte(s) (am Ende)", kcc_data.size() - 128 - header.prog_size);


    bool skip_ram_size = false;
    // Zielspeicher prüfen
    if(( header.loadaddr >= 0xE000) && ( header.loadaddr <= 0xFFFF))
    {
        std::println( "WARNUNG: KCC-Datei ({}) direkt für ROM E!", filename_short);
        skip_ram_size = true;
    }

    if(( header.loadaddr == 0xC000) && ( header.loadaddr <= 0xDFFF))
    {
        std::println( "WARNUNG: KCC-Datei ({}) direkt für ROM C!", filename_short);
        skip_ram_size = true;
    }

    // RAM-Speicher prüfen
    #define pwrtwo(x) (1 << (x))
    const int _16kbyte = pwrtwo( 14) * 1;
    const int _32kbyte = pwrtwo( 14) * 2;
    const int _48kbyte = pwrtwo( 14) * 3;
    if( ! skip_ram_size)
    {
        if( header.endaddr-1 < _16kbyte)
        {
            std::println( "min. 16 kByte RAM benötigt (RAM0).");
        }
        else
        {
            if( header.endaddr-1 <= _32kbyte)
            {
                std::println( "min. 32 kByte RAM benötigt (RAM4)!");
            }
            else
            {
                if( header.endaddr-1 <= _48kbyte)
                {
                    std::println( "min. 48 kByte RAM benötigt! Prüfen ob RAM8 oder IRM benötigt wird!");
                }
                else
                {
                    std::println( "WARNUNG: Mehr als 48 kByte RAM benötigt!"); // ungewöhnlich
                }
            }
        }
    }

    std::println();
    bool check_KCB = check_basic( kcc_data);

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
    else
    {
        if( !check_KCB)
        {
            std::println( "Keine Menüeinträge gefunden!");
        }
    }

    // sind BASIC-Arbeitszellen auf Adresse 300h?
    if( check_basic( kcc_data))
    {
        std::println( "INFO: BASIC-Arbeitszellen vorhanden.");
    }
    if(( header.loadaddr == 0x300) && ( header.startaddr == 0x370) && ( header.addrargs == 3))
    {
        std::println( "Startcode für BASIC-Start:");
        hex_dump( mem_data, 0x370 - header.loadaddr, 59);
        std::println();
    }

    if( check_KCB)
    {
        std::println( "BASIC-Programminfo:");
        // 03C4H und 03B0H = Speicherende
        // 0356H = Speicherende-100h (Stackende)

        // Speicherstellen prüfen
        uint16_t word = get_basic_word( kcc_data, 0x3c4);
        // und höchste Adresse suchen
        uint16_t word_max = word;
        std::println( "Adresse 03C4h: {:04X}h (Zeiger für Strings)", word);
        word = get_basic_word( kcc_data, 0x3b0);

        word_max = word > word_max ? word : word_max;
        std::println( "Adresse 03B0h: {:04X}h (Speichergröße)",     word);

        word = get_basic_word( kcc_data, 0x356);
        word_max = word > word_max ? word : word_max;
        std::println( "Adresse 0356h: {:04X}h (RAM-Minimum)",       word);

        // Memory end?
        if( word_max < _16kbyte)
        {
            std::println( "Entwickelt mit 16 kByte BASIC-RAM.");
        }
        else
        {
            if( word_max < _32kbyte)
            {
                std::println( "Entwickelt mit 32 kByte BASIC-RAM.");
            }
            else
            {
                if( word_max < _48kbyte)
                {
                    std::println( "Entwickelt mit 48 kByte BASIC-RAM.");
                }
                else
                {
                    std::println( "WARNUNG: Entwickelt mit mehr als 48 kByte BASIC-RAM!"); // wäre ungewöhnlich
                }
            }
        }
    }

    std::println();
}


//////////////////////////////////////////////////
int main( int argc, char** argv)
{

    const std::string self_name = argv[ 0];

#if 0
    std::print( "Argumente ({}): ", argc);
    for( int index = 0; index < argc; index++)
    {
        std::print( "{} ", argv[ index]);
    }
    std::println();
#endif

    if( argc < 2)
    {
        help( self_name);
        exit( EXIT_SUCCESS);
    }

    for( int index = 1; index < argc; index++)
    {
        process_KCC_file( argv[ index]);
    }

    exit( EXIT_SUCCESS);
}
