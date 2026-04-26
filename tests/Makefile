OUTPUT = gak_tests
CXX ?= c++
CXXFLAGS ?= -O2 -g -std=c++14
WARNING_FLAGS = -Wall -Wextra -Wpedantic
CPPFLAGS ?=
CPPFLAGS += -I../src
CPPFLAGS += -DGAK_TESTING
DEPFLAGS = -MMD -MP
LDFLAGS ?=
LDFLAGS += -L../src
LDLIBS += -lga -lm
LLVM_PROFDATA ?= $(shell if command -v xcrun >/dev/null 2>&1; then xcrun --find llvm-profdata; else command -v llvm-profdata; fi)
LLVM_COV ?= $(shell if command -v xcrun >/dev/null 2>&1; then xcrun --find llvm-cov; else command -v llvm-cov; fi)
PROFRAW = coverage.profraw
PROFDATA = coverage.profdata
TEXT_REPORT = coverage.txt

CXXFLAGS += $(WARNING_FLAGS)

ifeq ($(COVERAGE),1)
CXXFLAGS += -fprofile-instr-generate -fcoverage-mapping
LDFLAGS += -fprofile-instr-generate
endif

SRCS = test_main.cc
OBJS = $(SRCS:.cc=.o)
DEPS = $(OBJS:.o=.d)

.PHONY: all clean run coverage

all: $(OUTPUT)

$(OUTPUT): $(OBJS) ../src/libga.a
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

../src/libga.a:
	$(MAKE) -C ../src COVERAGE=$(COVERAGE)

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
	rm -f *~ *.o *.d $(OUTPUT) $(PROFRAW) $(PROFDATA) $(TEXT_REPORT)

-include $(DEPS)
