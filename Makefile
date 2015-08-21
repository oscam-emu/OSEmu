UNAME := $(shell uname -s)
CC ?= gcc
STRIP ?= strip
TARGETHELP := $(shell $(CC) --target-help 2>&1)

ifneq (,$(findstring sse2,$(TARGETHELP)))
CFLAGS=-I. -O3 -funroll-loops -fomit-frame-pointer -mmmx -msse -msse2 -msse3 -DPARALLEL_MODE=PARALLEL_128_SSE2
else ifneq (,$(findstring mplt,$(TARGETHELP)))
CFLAGS=-I. -O3 -funroll-loops -fomit-frame-pointer -mplt -DPARALLEL_MODE=PARALLEL_64_LONG
else ifneq (,$(findstring m4,$(TARGETHELP)))
CFLAGS=-I. -O2 -funroll-loops -fomit-frame-pointer -m4-300 -DPARALLEL_MODE=PARALLEL_32_INT -DMEMALIGN_VAL=4 -DCOPY_UNALIGNED_PKT=1
else
CFLAGS=-I. -O2 -funroll-loops
endif

LFLAGS=-lpthread
CC_WARN=-W -Wall -Wshadow -Wredundant-decls
SRCS = cscrypt/bn_ctx.c cscrypt/bn_lib.c cscrypt/bn_exp.c cscrypt/bn_sqr.c cscrypt/bn_div.c \
cscrypt/bn_asm.c cscrypt/bn_shift.c cscrypt/bn_word.c cscrypt/bn_add.c cscrypt/bn_mul.c \
cscrypt/aes.c cscrypt/i_cbc.c cscrypt/i_ecb.c cscrypt/i_skey.c cscrypt/mem.c cscrypt/des.c \
cscrypt/md5.c cscrypt/viades.c ffdecsa/ffdecsa.c \
module-emulator-st20.c module-emulator-dre2overcrypt.c via3surenc.c \
helpfunctions.c module-emulator-stream.c module-emulator-osemu.c OSEmu.c
Q = @
SAY = @echo
OBJS = $(SRCS:.c=.o)
DEPS = $(SRCS:.c=.d)
BIN = OSEmu

all: OSEmu

-include $(OBJS:.o=.d)

%.o: %.c
	$(Q)$(CC) $(CC_WARN) -c -o $@ $< $(CFLAGS)
	$(SAY) "CC	$<"
	$(Q)$(CC) $(CC_WARN) -MM $(CFLAGS) $*.c > $*.d

OSEmu: $(OBJS)
ifeq ($(UNAME),Darwin)
	$(Q)$(CC) $(CC_WARN) -o $(BIN) $(OBJS) $(CFLAGS) $(LFLAGS)
else ifdef ANDROID_NDK
	$(Q)$(CC) $(CC_WARN) -o $(BIN) $(OBJS) $(CFLAGS) $(LFLAGS)
else ifdef ANDROID_STANDALONE_TOOLCHAIN
	$(Q)$(CC) $(CC_WARN) -o $(BIN) $(OBJS) $(CFLAGS) $(LFLAGS)
else
	touch SoftCam.Key
	$(Q)$(CC) $(CC_WARN) -o $(BIN) $(OBJS) $(CFLAGS) $(LFLAGS) -Wl,--format=binary -Wl,SoftCam.Key -Wl,--format=default	
endif
	$(STRIP) $(BIN)
	
clean:
	rm -rf $(BIN) $(OBJS) $(DEPS)
	
.PHONY: OSEmu
