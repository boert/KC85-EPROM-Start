ASM_TARGET= auto_start_8k
CPP_TARGET= AUTO_Start_8k

OPTS=  -std=gnu++23 -static
WINOPTS=  -lstdc++exp  -static-libgcc 

all: $(CPP_TARGET)
exe: $(CPP_TARGET).exe


# Linuxprogramm
$(CPP_TARGET): $(CPP_TARGET).cpp     $(ASM_TARGET).rom
	c++    $(OPTS)   $(CPP_TARGET).cpp -o $(CPP_TARGET)
	strip  $(CPP_TARGET)

debug: $(CPP_TARGET).cpp 
	c++    $(OPTS) -ggdb  $(CPP_TARGET).cpp -o $(CPP_TARGET)


# Windowsprogramm 32 Bit
$(CPP_TARGET).exe: $(CPP_TARGET).cpp     $(ASM_TARGET).rom
	i686-w64-mingw32-c++  $(OPTS) $(CPP_TARGET).cpp $(WINOPTS) -o $(CPP_TARGET).exe
	i686-w64-mingw32-strip   $(CPP_TARGET).exe


# Assembler-Hilfsprogramm
$(ASM_TARGET).rom: $(ASM_TARGET).asm
	zasm   --z80 --opcodes --labels  $(ASM_TARGET).asm  $(ASM_TARGET).rom


clean:
	rm -f $(ASM_TARGET).lst
	rm -f $(ASM_TARGET).rom
	rm -f $(CPP_TARGET)
	rm -f $(CPP_TARGET).exe
