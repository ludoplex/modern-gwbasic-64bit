# Modern GW-BASIC 64-bit Makefile
# Supports cosmocc, gcc, and clang

CC ?= gcc
COSMOCC ?= cosmocc
CFLAGS = -std=c99 -O3 -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable
LDFLAGS = -lm

# Source files
SRCS = src/main.c \
       src/interpreter.c \
       src/lexer.c \
       src/basic_functions.c

OBJS = $(SRCS:.c=.o)
TARGET = gwbasic

# Architecture detection
ARCH := $(shell uname -m)
ifeq ($(ARCH),x86_64)
    CFLAGS += -mavx512f -mavx512dq
endif
ifeq ($(ARCH),aarch64)
    CFLAGS += -march=armv8-a+simd
endif

# Default target
all: $(TARGET)

# Build with default compiler
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
	@echo "Built $(TARGET) successfully!"

# Build with cosmocc for fat APE binary
cosmo: CC = $(COSMOCC)
cosmo: CFLAGS = -std=c99 -O3 -Wall
cosmo: clean
	$(COSMOCC) $(CFLAGS) $(SRCS) -o $(TARGET).com $(LDFLAGS)
	@echo "Built fat APE binary $(TARGET).com with cosmocc!"

# Individual object compilation
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build with gcc
gcc: CC = gcc
gcc: clean all

# Build with clang
clang: CC = clang
clang: clean all

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).com
	@echo "Cleaned build artifacts"

# Verify no if/switch statements and check assembly for branches
verify:
	@./scripts/verify-branchless.sh

# Check assembly output for branch instructions
check-asm: src/lexer.o
	@echo "Checking assembly for branch instructions in lexer..."
	@objdump -d src/lexer.o > /tmp/lexer.asm
	@echo "Searching for conditional branch instructions in hot path functions..."
	@echo ""
	@# Check for conditional jumps that indicate branches
	@if grep -E '(j[eng]|jl[eg]?|jg[eg]?|ja[eg]?|jb[eg]?|jn[abceglopz]|jc|jo|jp|js)\s' /tmp/lexer.asm | grep -v "jmp\s" | head -20; then \
		echo ""; \
		echo "⚠️  WARNING: Found conditional branch instructions above"; \
		echo "    (Note: Some branches may be from function calls or necessary for dispatch)"; \
		echo "    Review /tmp/lexer.asm to verify these are only in dispatch table"; \
	else \
		echo "✓ No conditional branch instructions found in main lexer functions"; \
		echo "  (Indirect jumps for function pointer dispatch are allowed)"; \
	fi
	@echo ""
	@echo "Full disassembly saved to /tmp/lexer.asm for manual review"

# Run tests
test: $(TARGET)
	@echo "Running basic tests..."
	@echo '10 PRINT "Hello, World!"' > /tmp/test.bas
	./$(TARGET) /tmp/test.bas
	@rm -f /tmp/test.bas

# Install
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

# Help
help:
	@echo "Modern GW-BASIC 64-bit Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build with default compiler (gcc)"
	@echo "  cosmo     - Build fat APE binary with cosmocc"
	@echo "  gcc       - Build with gcc"
	@echo "  clang     - Build with clang"
	@echo "  clean     - Remove build artifacts"
	@echo "  verify    - Verify no control flow keywords in code"
	@echo "  check-asm - Check assembly output for branch instructions"
	@echo "  test      - Run basic tests"
	@echo "  install   - Install to /usr/local/bin"
	@echo "  help      - Show this help message"

.PHONY: all cosmo gcc clang clean verify check-asm test install help
