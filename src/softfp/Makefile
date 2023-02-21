CXX=$(shell sst-config --CXX)
CXXFLAGS=$(shell sst-config --ELEMENT_CXXFLAGS) -fno-stack-protector
# CXXFLAGS+=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -MMD
LDFLAGS=$(shell sst-config --ELEMENT_LDFLAGS) -fno-stack-protector

PROGS=softfp

all: $(PROGS)

softfp: softfp.o softfloat.o
	$(CXX) $(LDFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.d *~ $(PROGS)

-include $(wildcard *.d)

