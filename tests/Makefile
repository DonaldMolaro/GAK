OUTPUT = gak_tests
CXX ?= c++
CXXFLAGS ?= -O2 -g -std=c++14
WARNING_FLAGS = -Wall -Wextra -Wpedantic
CPPFLAGS ?=
override CPPFLAGS += -I../src
override CPPFLAGS += -DGAK_TESTING
DEPFLAGS = -MMD -MP
LDFLAGS ?=
override LDFLAGS += -L../src
override LDLIBS += -lga -lm
LLVM_PROFDATA ?= $(shell if command -v xcrun >/dev/null 2>&1; then xcrun --find llvm-profdata; else command -v llvm-profdata; fi)
LLVM_COV ?= $(shell if command -v xcrun >/dev/null 2>&1; then xcrun --find llvm-cov; else command -v llvm-cov; fi)
PROFRAW = coverage.profraw
PROFDATA = coverage.profdata
TEXT_REPORT = coverage.txt

override CXXFLAGS += $(WARNING_FLAGS)
# Never fuse a*b + c into one rounding step: GCC and Clang fuse differently,
# which changes fitness values in the last bit and makes fixed-seed runs
# diverge between compilers.
override CXXFLAGS += -ffp-contract=off

ifeq ($(COVERAGE),1)
override CXXFLAGS += -fprofile-instr-generate -fcoverage-mapping
override LDFLAGS += -fprofile-instr-generate
endif

SRCS = test_main.cc
OBJS = $(SRCS:.cc=.o)
DEPS = $(OBJS:.o=.d)

.PHONY: all clean FORCE run coverage

all: $(OUTPUT)

$(OUTPUT): $(OBJS) ../src/libga.a
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

# Always ask the library's own (incremental) Makefile to bring libga.a up
# to date. Without FORCE this rule ran only when libga.a was missing, so
# building here after the library sources changed linked against a stale
# library whose structure layouts no longer matched the headers.
../src/libga.a: FORCE
	$(MAKE) -C ../src COVERAGE=$(COVERAGE)

FORCE:

%.o: %.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c -o $@ $<

run: $(OUTPUT)
	./$(OUTPUT)

coverage: $(OUTPUT)
	LLVM_PROFILE_FILE=$(PROFRAW) ./$(OUTPUT)
	$(LLVM_PROFDATA) merge -sparse $(PROFRAW) -o $(PROFDATA)
	$(LLVM_COV) report ./$(OUTPUT) -instr-profile=$(PROFDATA)
	$(LLVM_COV) show ./$(OUTPUT) -instr-profile=$(PROFDATA) > $(TEXT_REPORT)

clean:
	rm -f *~ *.o *.d $(OUTPUT) $(PROFRAW) $(PROFDATA) $(TEXT_REPORT) *.gcda *.gcno *.gcov

-include $(DEPS)
