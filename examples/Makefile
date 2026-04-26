OUTPUT = GA
TEST_OUTPUT = example_tests
CXX ?= c++
CXXFLAGS ?= -O2 -g -std=c++14
CPPFLAGS ?=
CPPFLAGS += -I../include -I../src
DEPFLAGS = -MMD -MP
LDFLAGS ?=
LDFLAGS += -L../src
LDLIBS += -lga -lm

SRCS = GA.cc spell.cc alpha.cc dome.cc ts.cc f6.cc nqueens.cc knapsack.cc latinsquare.cc sudoku.cc sudoku_constrained.cc
OBJS = $(SRCS:.cc=.o)
EXAMPLE_SRCS = spell.cc alpha.cc dome.cc ts.cc f6.cc nqueens.cc knapsack.cc latinsquare.cc sudoku.cc sudoku_constrained.cc test_main.cc
EXAMPLE_OBJS = $(EXAMPLE_SRCS:.cc=.o)
DEPS = $(OBJS:.o=.d) $(EXAMPLE_OBJS:.o=.d)

.PHONY: all clean test run-tests

all: $(OUTPUT)

test: $(TEST_OUTPUT) run-tests

$(OUTPUT): $(OBJS) ../src/libga.a
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(TEST_OUTPUT): $(EXAMPLE_OBJS) ../src/libga.a
	$(CXX) $(LDFLAGS) -o $@ $(EXAMPLE_OBJS) $(LDLIBS)

../src/libga.a:
	$(MAKE) -C ../src

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<

run-tests: $(TEST_OUTPUT)
	./$(TEST_OUTPUT)

clean:
	rm -f *~ *.o core *.d $(OUTPUT) $(TEST_OUTPUT)

-include $(DEPS)
