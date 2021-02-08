SOURCES := iow-i2c.cpp iowkit.c
HEADERS := iowarrior.h iowkit.h

all:     iow-i2c

iow-i2c:     $(SOURCES) $(HEADERS)
	        $(CROSS_COMPILE)g++ $(LFLAGS) -Wall -DUSE_HOSTCC $(SOURCES) -o iow-i2c

clean:
	        rm -f iow-i2c

