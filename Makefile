# Modern GW-BASIC 64-bit Interpreter Makefile

CC = gcc
AS = as
CFLAGS = -Wall -Wextra -O2 -I./include
LDFLAGS = -lm

# Detect architecture
ARCH := $(shell uname -m)

# Source files
C_SOURCES = src/main.c src/program.c src/symbol_table.c src/expression.c
OBJS = main.o program.o symbol_table.o expression.o

# Architecture-specific assembly files
ifeq ($(ARCH),x86_64)
    ASM_SOURCE = src/asm_core_amd64.S
    OBJS += asm_core_amd64.o
    ARCH_NAME = AMD64
else ifeq ($(ARCH),aarch64)
    ASM_SOURCE = src/asm_core_arm64.S
    OBJS += asm_core_arm64.o
    ARCH_NAME = ARM64
else ifeq ($(ARCH),arm64)
    ASM_SOURCE = src/asm_core_arm64.S
    OBJS += asm_core_arm64.o
    ARCH_NAME = ARM64
else
    $(error Unsupported architecture: $(ARCH))
endif

TARGET = gwbasic

.PHONY: all clean test help

all: $(TARGET)
	@echo "Built GW-BASIC interpreter for $(ARCH_NAME) ($(ARCH))"

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

main.o: src/main.c include/gwbasic.h
	$(CC) $(CFLAGS) -c src/main.c -o main.o

program.o: src/program.c include/gwbasic.h
	$(CC) $(CFLAGS) -c src/program.c -o program.o

symbol_table.o: src/symbol_table.c include/gwbasic.h
	$(CC) $(CFLAGS) -c src/symbol_table.c -o symbol_table.o

expression.o: src/expression.c include/gwbasic.h
	$(CC) $(CFLAGS) -c src/expression.c -o expression.o

asm_core_amd64.o: src/asm_core_amd64.S
	$(AS) src/asm_core_amd64.S -o asm_core_amd64.o

asm_core_arm64.o: src/asm_core_arm64.S
	$(AS) src/asm_core_arm64.S -o asm_core_arm64.o

test: $(TARGET)
	@echo "Running test program..."
	@./$(TARGET) examples/hello.bas

clean:
	rm -f $(OBJS) $(TARGET)

help:
	@echo "Modern GW-BASIC 64-bit Interpreter"
	@echo "Targets:"
	@echo "  all     - Build the interpreter (default)"
	@echo "  test    - Run test programs"
	@echo "  clean   - Remove build artifacts"
	@echo "  help    - Show this help"
	@echo ""
	@echo "Current architecture: $(ARCH_NAME) ($(ARCH))"
