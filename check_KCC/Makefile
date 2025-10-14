TARGET= check_KCC
OPTS=  -std=gnu++23 -static
WINOPTS=  -lstdc++exp  -static-libgcc 

CFLAGS=-Os -ffunction-sections -fdata-sections 
LDFLAGS=-Wl,-s  -Wl,--gc-sections

all: $(TARGET)
exe: $(TARGET).exe

# Linuxprogramm
$(TARGET): $(TARGET).cpp
	c++    $(OPTS) $(CFLAGS) $(LDFLAGS)   $(TARGET).cpp -o $(TARGET)
	strip  $(TARGET)


# Windowsprogramm 64 Bit
#$(TARGET).exe: $(TARGET).cpp
#	x86_64-w64-mingw32-c++ $(OPTS) $(TARGET).cpp  -lstdc++exp   -o $(TARGET).exe

# Windowsprogramm 32 Bit, statisch
$(TARGET).exe: $(TARGET).cpp
	i686-w64-mingw32-c++  $(OPTS) $(CFLAGS) $(LDFLAGS) $(TARGET).cpp $(WINOPTS) -o $(TARGET).exe
	i686-w64-mingw32-strip   $(TARGET).exe

clean:
	rm -f $(TARGET)
	rm -f $(TARGET).exe
