CC=gcc
CFLAGS=-I.
SRCS = aes.c des.c md5.c helpfunctions.c emulator.c OSEmu.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
BIN = OSEmu

all: OSEmu

-include $(OBJS:.o=.d)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
	$(CC) -MM $(CFLAGS) $*.c > $*.d

debug: $(OBJS)
	$(CC) -g -O0 -o $(BIN).debug $(OBJS) $(CFLAGS)	
	
OSEmu: $(OBJS)
	$(CC) -O2 -o $(BIN) $(OBJS) $(CFLAGS)	
	strip $(BIN)
	
clean:
	rm -rf $(BIN) $(OBJS) $(DEPS)
	
.PHONY: OSEmu