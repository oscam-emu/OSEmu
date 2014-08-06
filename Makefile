UNAME := $(shell uname -s)
CC = gcc
STRIP = strip
CFLAGS=-I.
SRCS = bn_ctx.c bn_lib.c bn_exp.c bn_sqr.c bn_div.c bn_asm.c bn_shift.c bn_word.c bn_add.c bn_mul.c \
 aes.c  i_cbc.c i_ecb.c i_skey.c mem.c des_enc.c set_key.c des.c md5.c \
 helpfunctions.c via3surenc.c emulator.c OSEmu.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
BIN = OSEmu

all: OSEmu

-include $(OBJS:.o=.d)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)
	$(CC) -MM $(CFLAGS) $*.c > $*.d

OSEmu: $(OBJS)
ifeq ($(UNAME),Darwin)
	$(CC) -O2 -o $(BIN) $(OBJS) $(CFLAGS)
else
	$(CC) -O2 -o $(BIN) $(OBJS) $(CFLAGS) -Wl,--format=binary -Wl,SoftCam.Key -Wl,--format=default	
endif
	$(STRIP) $(BIN)
	
clean:
	rm -rf $(BIN) $(OBJS) $(DEPS)
	
.PHONY: OSEmu
