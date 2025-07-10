TARGET = onebin
OBJS = main.o bch6.o ecc.o

CXX = g++

# machine flag: should be -m32 or empty, which means 32-bit-elf and 64-bit-elf, respectively.
MACH_FLAG := -m32
CFLAGS = -Wall -O0 -g $(MACH_FLAG)
#CFLAGS = -Wall -O3 
LDFLAGS = -L/usr/lib -lm

ALL_CPP = $(patsubst %.o,%.cpp,$(OBJS))
ALL_DEP = $(patsubst %.o,%.d,$(OBJS))

all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CXX) $(CFLAGS) -o $(TARGET) $^ $(LDFLAGS)

$(OBJS) : %.o : %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

ifneq ($(MAKECMDGOALS),clean)
include $(ALL_DEP)
endif

$(ALL_DEP) : %.d : %.cpp
	$(CXX) $(CFLAGS) -MM $< > $@

.PHONY: clean clean_dep
clean :
	-rm -rf $(TARGET) $(OBJS)
	-rm -f $(ALL_DEP)

clean_dep :
	-rm -f $(ALL_DEP)

