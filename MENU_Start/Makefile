ASM_TARGET= menu_start_16k
CPP_TARGET= MENU_Start_16k

OPTS=  -std=gnu++23 -static
WINOPTS=  -lstdc++exp  -static-libgcc 

CFLAGS=-Os -ffunction-sections -fdata-sections 
LDFLAGS=-Wl,-s  -Wl,--gc-sections

all: $(CPP_TARGET)
exe: $(CPP_TARGET).exe


# Linuxprogramm
$(CPP_TARGET): $(CPP_TARGET).cpp    rom.h $(ASM_TARGET).rom
	c++    $(OPTS) $(CFLAGS) $(LDFLAGS)  $(CPP_TARGET).cpp  -o $(CPP_TARGET)
	strip  $(CPP_TARGET)

debug: $(CPP_TARGET).cpp 
	c++    $(OPTS) -ggdb  $(CPP_TARGET).cpp -o $(CPP_TARGET)


# Windowsprogramm 32 Bit
$(CPP_TARGET).exe: $(CPP_TARGET).cpp    rom.h $(ASM_TARGET).rom
	i686-w64-mingw32-c++  $(OPTS) $(CFLAGS) $(LDFLAGS)  $(CPP_TARGET).cpp $(WINOPTS) -o $(CPP_TARGET).exe
	i686-w64-mingw32-strip   $(CPP_TARGET).exe


# Assembler-Hilfsprogramm
$(ASM_TARGET).lst: $(ASM_TARGET).rom
$(ASM_TARGET).rom: $(ASM_TARGET).asm
	zasm   --z80 --opcodes  $(ASM_TARGET).asm  $(ASM_TARGET).rom

# Symbole (aus Assembler)
rom.h: $(ASM_TARGET).lst
	@echo \#define prg_dest    0x`grep prg_dest:    $(ASM_TARGET).lst | cut -f1 -d:` >> $@
	@echo \#define prg_start   0x`grep prg_start:   $(ASM_TARGET).lst | cut -f1 -d:` >> $@
	@echo \#define prg_args    0x`grep prg_args:    $(ASM_TARGET).lst | cut -f1 -d:` >> $@
	@echo \#define bl1_start   0x`grep bl1_start:   $(ASM_TARGET).lst | cut -f1 -d:` >> $@
	@echo \#define bl1_size    0x`grep bl1_size:    $(ASM_TARGET).lst | cut -f1 -d:` >> $@
	@echo \#define mwort       0x`grep mwort:       $(ASM_TARGET).lst | cut -f1 -d:` >> $@
	@echo \#define menu_cnt    0x`grep menu_cnt:    $(ASM_TARGET).lst | cut -f1 -d:` >> $@
	@echo \#define menu_addr   0x`grep menu_addr:   $(ASM_TARGET).lst | cut -f1 -d:` >> $@

clean:
	rm -f $(ASM_TARGET).lst
	rm -f $(ASM_TARGET).rom
	rm -f rom.h
	rm -f $(CPP_TARGET)
	rm -f $(CPP_TARGET).exe
