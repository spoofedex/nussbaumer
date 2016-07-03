ASM       := as
CC        := gcc
CXX       := g++
LD        := g++

BUILD_DIR := ./bin/.build

WARNINGS  := -Wall -Wextra -pedantic -Wshadow -Wpointer-arith -Wcast-align \
             -Wwrite-strings -Wredundant-decls -Winline -Wno-long-long \
             -Wuninitialized -Wno-unused-parameter -Wno-unused
CXXFLAGS  := -g -I common -I lib -std=c++14 $(WARNINGS) -O3

avx2.CXXFLAGS       = -std=c++14 -O3 -I . -I common -I lib -Wall -Wextra -fomit-frame-pointer -march=corei7-avx -msse2avx
avx2.ASMFLAGS       = -mmnemonic=intel -msyntax=intel -mnaked-reg -mavxscalar=256

newhopeavx2.CFLAGS   = -Wall -Wextra -O3 -fomit-frame-pointer -msse2avx -march=corei7-avx -msse2avx
newhope.CXXFLAGS     = -g -std=c++14 -I common -I lib -O3
rlwekex.CXXFLAGS     = -g -std=c++14 -I common -I lib -O3

TEST_DIRS        := $(shell ls tests)
TESTS            := $(TEST_DIRS:%=bin/test-%)

AVX2TEST_DIRS    := $(shell ls avx2tests)
AVX2TESTS        := $(AVX2TEST_DIRS:%=bin/avx2test-%)

common.CXXFILES  := $(shell find common -maxdepth 1 -type f -name '*.cpp')
common.OBJFILES  := $(common.CXXFILES:%.cpp=$(BUILD_DIR)/%.o)
common.DEPFILES  := $(common.OBJFILES:%.o=%.d)

compat.CXXFILES  := $(shell find common/compat -type f -name '*.cpp')
compat.OBJFILES  := $(compat.CXXFILES:%.cpp=$(BUILD_DIR)/%.o)
compat.DEPFILES  := $(compat.OBJFILES:%.o=%.d)

rlwekex.CXXFILES := $(shell find lib/rlwekex -type f -name '*.cpp')
rlwekex.OBJFILES := $(rlwekex.CXXFILES:%.cpp=$(BUILD_DIR)/%.o)
rlwekex.DEPFILES := $(rlwekex.OBJFILES:%.o=%.d)

newhope.CXXFILES := $(shell find lib/newhope/ref -type f -name '*.cpp')
newhope.OBJFILES := $(newhope.CXXFILES:%.cpp=$(BUILD_DIR)/%.o)
newhope.DEPFILES := $(newhope.OBJFILES:%.o=%.d)

newhopeavx2.CFILES   := $(shell find lib/newhope/avx2 -type f -name '*.c')
newhopeavx2.ASMFILES := $(shell find lib/newhope/avx2 -type f -name '*.s')
newhopeavx2.SFILES   := $(shell find lib/newhope/avx2 -type f -name '*.S')
newhopeavx2.OBJFILES := $(newhopeavx2.CFILES:%.c=$(BUILD_DIR)/%.o) $(newhopeavx2.ASMFILES:%.s=$(BUILD_DIR)/%.o) $(newhopeavx2.SFILES:%.S=$(BUILD_DIR)/%.o)
newhopeavx2.DEPFILES := $(newhopeavx2.OBJFILES:%.o=%.d)

avx2.CXXFILES    := $(shell find avx2 -type f -name '*.cpp')
avx2.ASMFILES    := $(shell find avx2 -type f -name '*.s')
avx2.OBJFILES    := $(avx2.CXXFILES:%.cpp=$(BUILD_DIR)/%.o) $(avx2.ASMFILES:%.s=$(BUILD_DIR)/%.o)
avx2.DEPFILES    := $(avx2.OBJFILES:%.o=%.d)


.PHONY: all clean all-opcount all-avx2

all: all-opcount all-avx2

all-opcount: $(TESTS)

all-avx2: $(AVX2TESTS)

clean:
	-rm -rf bin

-include $(common.DEPFILES)
-include $(rlwekex.DEPFILES)
-include $(newhope.DEPFILES)
-include $(newhopeavx2.DEPFILES)
-include $(avx2.DEPFILES)

# Building regular files
$(BUILD_DIR)/common/%.o: common/%.cpp Makefile
	@echo "[+] Building $@"
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@
$(BUILD_DIR)/avx2/%.o: avx2/%.cpp Makefile
	@echo "[+] Building $@"
	@mkdir -p $(dir $@)
	$(CXX) $(avx2.CXXFLAGS) -MMD -MP -c $< -o $@
$(BUILD_DIR)/avx2/%.o: avx2/%.s Makefile
	@echo "[+] Building $@"
	@mkdir -p $(dir $@)
	$(ASM) $(avx2.ASMFLAGS) $< -o $@
$(BUILD_DIR)/lib/newhope/ref/%.o: lib/newhope/ref/%.cpp Makefile
	@echo "[+] Building $@"
	@mkdir -p $(dir $@)
	$(CXX) $(newhope.CXXFLAGS) -MMD -MP -c $< -o $@
$(BUILD_DIR)/lib/rlwekex/%.o: lib/rlwekex/%.cpp Makefile
	@echo "[+] Building $@"
	@mkdir -p $(dir $@)
	$(CXX) $(rlwekex.CXXFLAGS) -MMD -MP -c $< -o $@
$(BUILD_DIR)/lib/newhope/avx2/%.o: lib/newhope/avx2/%.c Makefile
	@echo "[+] Building $@"
	@mkdir -p $(dir $@)
	$(CC) $(newhopeavx2.CFLAGS) -MMD -MP -c $< -o $@
$(BUILD_DIR)/lib/newhope/avx2/%.o: lib/newhope/avx2/%.s Makefile
	@echo "[+] Building $@"
	@mkdir -p $(dir $@)
	$(CC) $(newhopeavx2.CFLAGS) -MMD -MP -c $< -o $@
$(BUILD_DIR)/lib/newhope/avx2/crypto_stream_aes256ctr.o: lib/newhope/avx2/crypto_stream_aes256ctr.c Makefile
	@echo "[+] Building $@"
	@mkdir -p $(dir $@)
	$(CC) -m64 -march=native -mtune=native -O3 -fomit-frame-pointer -MMD -MP -c $< -o $@

# Macro for building the tests
define MAKE_TEST

test$1.CXXFILES  := $$(shell find tests/$1 -type f -name '*.cpp')
test$1.OBJFILES  := $$(test$1.CXXFILES:%.cpp=$(BUILD_DIR)/%.o)
test$1.DEPFILES  := $$(test$1.OBJFILES:%.o=%.d)

-include $$(test$1.DEPFILES)

$(BUILD_DIR)/tests/%.o: tests/%.cpp Makefile
	@echo "[+] Building "$$(@:$(BUILD_DIR)/%=%)
	@mkdir -p $$(dir $$@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $$< -o $$@

bin/test-$1: $(common.OBJFILES) $(newhope.OBJFILES) $(compat.OBJFILES) $$(test$1.OBJFILES) $$(rlwekex.OBJFILES) Makefile
	@echo "[+] Building "$$(@:$(BUILD_DIR)/%=%)
	$(LD) -o $$@ $(common.OBJFILES) $(newhope.OBJFILES) $(compat.OBJFILES) $$(test$1.OBJFILES) $$(rlwekex.OBJFILES)

endef

define MAKE_AVX2TEST

avx2test$1.CXXFILES  := $$(shell find avx2tests/$1 -type f -name '*.cpp')
avx2test$1.OBJFILES  := $$(avx2test$1.CXXFILES:%.cpp=$(BUILD_DIR)/%.o)
avx2test$1.DEPFILES  := $$(avx2test$1.OBJFILES:%.o=%.d)

-include $$(avx2test$1.DEPFILES)

$(BUILD_DIR)/avx2tests/%.o: avx2tests/%.cpp Makefile
	@echo "[+] Building "$$(@:$(BUILD_DIR)/%=%)
	@mkdir -p $$(dir $$@)
	$(CXX) $(avx2.CXXFLAGS) -MMD -MP -c $$< -o $$@

bin/avx2test-$1: $(common.OBJFILES) $(newhopeavx2.OBJFILES) $$(avx2.OBJFILES) $$(avx2test$1.OBJFILES) Makefile
	@echo "[+] Building "$$(@:$(BUILD_DIR)/%=%)
	$(LD) -o $$@ $(common.OBJFILES) $(newhopeavx2.OBJFILES) $$(avx2.OBJFILES) $$(avx2test$1.OBJFILES)

endef

# Expand a macro for each test
$(foreach TEST_DIR,$(TEST_DIRS),$(eval $(call MAKE_TEST,$(TEST_DIR))))
$(foreach AVX2TEST_DIR,$(AVX2TEST_DIRS),$(eval $(call MAKE_AVX2TEST,$(AVX2TEST_DIR))))
