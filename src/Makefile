AR ?= ar
CXX ?= c++
CXXFLAGS ?= -O2 -g -std=c++14
WARNING_FLAGS = -Wall -Wextra -Wpedantic
CPPFLAGS ?=
override CPPFLAGS += -I.
DEPFLAGS = -MMD -MP

override CXXFLAGS += $(WARNING_FLAGS)

ifeq ($(COVERAGE),1)
override CXXFLAGS += -fprofile-instr-generate -fcoverage-mapping
override LDFLAGS += -fprofile-instr-generate
endif

LIB = libga.a
LIBSRC = base.cc chromosome.cc population.cc population_report.cc except.cc
LIBOBJ = $(LIBSRC:.cc=.o)
DEPS = $(LIBOBJ:.o=.d)

.PHONY: all clean

all: $(LIB)

$(LIB): $(LIBOBJ)
	rm -f $@
	$(AR) rcs $@ $(LIBOBJ)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<

clean:
	rm -f *~ *.o core *.d $(LIB)

-include $(DEPS)
