TARGET := authbch6
BIN := ../bin
OBJS := main.o
OBJS += bch6.o
OBJS += bch6_util.o
OBJS += crypto_util.o
OBJS += fdt_util.o
OBJS += fdt/fdt.o
OBJS += fdt/fdt_rw.o
OBJS += fdt/fdt_ro.o
OBJS += fdt/fdt_wip.o
OBJS += sodium/sodium_utils.o
OBJS += sodium/crypto_sign.o
OBJS += sodium/ed25519.o
OBJS += sodium/sha512.o

CXX := gcc -m32 -I ../../release/ -I ./fdt -I ./ -I ./sodium
CFLAGS ?= -Wall -Wno-unused-result -DSODIUM_UTIL -DAUTH_UBT_SIGN_ONLY

ALL_SRC := $(patsubst %.o,%.c,$(OBJS))
ALL_DEP := $(patsubst %.o,%.d,$(OBJS))

all: $(TARGET)

preconfig: $(TARGET)
	cp -f $(TARGET) $(BIN)

$(BIN)/$(TARGET): $(TARGET)
	cp -f $(TARGET) $(BIN)

./util/bin/authbch6: $(BIN)/$(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) -o $(TARGET) $^ $(LDFLAGS)

$(OBJS) : %.o : %.c
	$(CXX) $(CFLAGS) -c $< -o $@

#ifeq ($(MAKECMDGOALS),all)
#include $(ALL_DEP)
#endif

#$(ALL_DEP) : %.d : %.c
#	$(CXX) $(CFLAGS) -MM $< > $@

.PHONY: clean clean_dep distclean all
clean:
	-rm -f $(TARGET) $(OBJS)

clean_dep:
	-rm -f $(ALL_DEP)

distclean: clean clean_dep

Makefile: ;
