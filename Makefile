UNAME := $(shell uname -s)
CC = gcc
STRIP = strip
CFLAGS=-I.
LFLAGS=-lpthread
SRCS = cscrypt/bn_ctx.c cscrypt/bn_lib.c cscrypt/bn_exp.c cscrypt/bn_sqr.c cscrypt/bn_div.c \
cscrypt/bn_asm.c cscrypt/bn_shift.c cscrypt/bn_word.c cscrypt/bn_add.c cscrypt/bn_mul.c \
cscrypt/aes.c cscrypt/i_cbc.c cscrypt/i_ecb.c cscrypt/i_skey.c cscrypt/mem.c cscrypt/des.c \
cscrypt/md5.c cscrypt/viades.c ffdecsa/ffdecsa.c \
module-emulator-st20.c module-emulator-dre2overcrypt.c via3surenc.c \
helpfunctions.c module-emulator-stream.c module-emulator-osemu.c OSEmu.c
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
BIN = OSEmu

all: OSEmu

-include $(OBJS:.o=.d)

%.o: %.c
	$(CC) -Wall -O2 -c -o $@ $< $(CFLAGS)
	$(CC) -Wall -O2 -MM $(CFLAGS) $*.c > $*.d

OSEmu: $(OBJS)
ifeq ($(UNAME),Darwin)
	$(CC) -Wall -O2 -o $(BIN) $(OBJS) $(CFLAGS) $(LFLAGS)
else ifdef ANDROID_NDK
	$(CC) -Wall -O2 -o $(BIN) $(OBJS) $(CFLAGS) $(LFLAGS)
else ifdef ANDROID_STANDALONE_TOOLCHAIN
	$(CC) -Wall -O2 -o $(BIN) $(OBJS) $(CFLAGS) $(LFLAGS)
else
	touch SoftCam.Key
	$(CC) -Wall -O2 -o $(BIN) $(OBJS) $(CFLAGS) $(LFLAGS) -Wl,--format=binary -Wl,SoftCam.Key -Wl,--format=default	
endif
	$(STRIP) $(BIN)
	
clean:
	rm -rf $(BIN) $(OBJS) $(DEPS)
	
.PHONY: OSEmu
