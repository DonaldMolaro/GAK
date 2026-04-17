OUTPUT = GA
CXX ?= c++
CXXFLAGS ?= -O2 -g -std=c++14
CPPFLAGS ?=
CPPFLAGS += -I../include -I../src
DEPFLAGS = -MMD -MP
LDFLAGS ?=
LDFLAGS += -L../src
LDLIBS += -lga -lm

SRCS = GA.cc spell.cc alpha.cc dome.cc ts.cc f6.cc
OBJS = $(SRCS:.cc=.o)
DEPS = $(OBJS:.o=.d)

.PHONY: all clean depen

all: $(OUTPUT)

$(OUTPUT): $(OBJS) ../src/libga.a
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

../src/libga.a:
	$(MAKE) -C ../src

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<

depen: $(DEPS)
	cat $(DEPS) > all.depend

clean:
	rm -f *~ *.o core *.d $(OUTPUT)

-include $(DEPS)
