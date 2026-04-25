AR ?= ar
RANLIB ?= ranlib
CXX ?= c++
CXXFLAGS ?= -O2 -g -std=c++14
CPPFLAGS ?=
CPPFLAGS += -I.
DEPFLAGS = -MMD -MP

ifeq ($(COVERAGE),1)
CXXFLAGS += -fprofile-instr-generate -fcoverage-mapping
LDFLAGS += -fprofile-instr-generate
endif

LIB = libga.a
LIBSRC = base.cc chromosome.cc population.cc except.cc
LIBOBJ = $(LIBSRC:.cc=.o)
DEPS = $(LIBOBJ:.o=.d)

.PHONY: all clean depen install

all: $(LIB)

$(LIB): $(LIBOBJ)
	rm -f $@
	$(AR) rcs $@ $(LIBOBJ)
	$(RANLIB) $@

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<

depen: $(DEPS)
	cat $(DEPS) > all.depend

clean:
	rm -f *~ *.o core *.d $(LIB)

install: $(LIB)
	mkdir -p ../lib ../include
	cp $(LIB) ../lib
	cp base.hh ../include
	cp chromosome.hh ../include
	cp population.hh ../include
	cp genetic.hh ../include
	cp except.hh ../include

-include $(DEPS)
