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
LLVM_PROFDATA ?= $(shell xcrun --find llvm-profdata)
LLVM_COV ?= $(shell xcrun --find llvm-cov)
PROFRAW = coverage.profraw
PROFDATA = coverage.profdata
TEXT_REPORT = coverage.txt

ifeq ($(COVERAGE),1)
CXXFLAGS += -fprofile-instr-generate -fcoverage-mapping
LDFLAGS += -fprofile-instr-generate
endif

SRCS = GA.cc spell.cc alpha.cc dome.cc ts.cc f6.cc nqueens.cc knapsack.cc latinsquare.cc sudoku.cc sudoku_constrained.cc
OBJS = $(SRCS:.cc=.o)
EXAMPLE_SRCS = spell.cc alpha.cc dome.cc ts.cc f6.cc nqueens.cc knapsack.cc latinsquare.cc sudoku.cc sudoku_constrained.cc test_main.cc
EXAMPLE_OBJS = $(EXAMPLE_SRCS:.cc=.o)
DEPS = $(OBJS:.o=.d) $(EXAMPLE_OBJS:.o=.d)

.PHONY: all clean test run-tests coverage

all: $(OUTPUT)

test: $(TEST_OUTPUT) run-tests

$(OUTPUT): $(OBJS) ../src/libga.a
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(TEST_OUTPUT): $(EXAMPLE_OBJS) ../src/libga.a
	$(CXX) $(LDFLAGS) -o $@ $(EXAMPLE_OBJS) $(LDLIBS)

../src/libga.a:
	$(MAKE) -C ../src COVERAGE=$(COVERAGE)

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<

test_main.o: CPPFLAGS += -DGAK_TESTING

run-tests: $(TEST_OUTPUT)
	./$(TEST_OUTPUT)

coverage: $(TEST_OUTPUT)
	LLVM_PROFILE_FILE=$(PROFRAW) ./$(TEST_OUTPUT)
	$(LLVM_PROFDATA) merge -sparse $(PROFRAW) -o $(PROFDATA)
	$(LLVM_COV) report ./$(TEST_OUTPUT) -instr-profile=$(PROFDATA)
	$(LLVM_COV) show ./$(TEST_OUTPUT) -instr-profile=$(PROFDATA) > $(TEXT_REPORT)

clean:
	rm -f *~ *.o core *.d $(OUTPUT) $(TEST_OUTPUT) $(PROFRAW) $(PROFDATA) $(TEXT_REPORT)

-include $(DEPS)
