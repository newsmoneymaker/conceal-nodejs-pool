# ccxhash: CryptoNight-GPU (CN-GPU) share validation helper of the CCX pool. Sources in ccx-src/ are
# poolpayminer's own vendored copy of this hash (see ccxhash.cpp's header comment and ccx-project.md),
# not a second independent vendoring -- keeps the pool's share check and the miner's PoW identical.
#   make            (STATIC=-static makes a binary that runs on any Linux)
CXX ?= g++
CC ?= gcc
STATIC ?=

CCX_DIR = ccx-src
ISA_FLAGS = -maes -mavx2 -msse4.1 -mssse3

C_SOURCES = keccak.c aux_hash.c
C_OBJS = $(addprefix $(CCX_DIR)/,$(C_SOURCES:.c=.o))

CXX_SOURCES = ccx_gpu_hash.cpp cn_slow_hash_soft.cpp cn_slow_hash_hard_intel.cpp cn_slow_hash_intel_avx2.cpp
CXX_OBJS = $(addprefix $(CCX_DIR)/,$(CXX_SOURCES:.cpp=.o))

CFLAGS = -O2 -w -std=gnu11 $(ISA_FLAGS) -include $(CCX_DIR)/ccxgpu_rename.h -I$(CCX_DIR)
CXXFLAGS_VENDORED = -O2 $(ISA_FLAGS) -std=c++14 -include $(CCX_DIR)/ccxgpu_rename.h -I$(CCX_DIR)
CXXFLAGS = -O2 -std=c++14 -I$(CCX_DIR)

ccxhash: ccxhash.o $(C_OBJS) $(CXX_OBJS)
	$(CXX) -O2 $(STATIC) -o $@ ccxhash.o $(C_OBJS) $(CXX_OBJS) -lpthread

ccxhash.o: ccxhash.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(CCX_DIR)/%.o: $(CCX_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(CCX_DIR)/%.o: $(CCX_DIR)/%.cpp
	$(CXX) $(CXXFLAGS_VENDORED) -c -o $@ $<

clean:
	rm -f ccxhash ccxhash.o $(C_OBJS) $(CXX_OBJS)
