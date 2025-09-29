TARGET=BINLOAD

OPTRIONS= --z80 --opcodes

all: $(TARGET).kcc

$(TARGET).kcc: $(TARGET).asm
	zasm   $(OPTRIONS)   $(TARGET).asm   $(TARGET).KCC

clean:
	rm -f $(TARGET).lst
	rm -f $(TARGET).kcc
