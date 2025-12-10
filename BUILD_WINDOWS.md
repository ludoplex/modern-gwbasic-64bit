# Building Modern GW-BASIC 64-bit on Windows 11

This guide explains how to build the Modern GW-BASIC 64-bit interpreter on Windows 11.

## Prerequisites

### Option 1: MinGW-w64 (Recommended for GCC compatibility)

1. **Install MSYS2** from https://www.msys2.org/
2. **Open MSYS2 MinGW 64-bit terminal**
3. **Install build tools:**
   ```bash
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-binutils make
   ```

### Option 2: Visual Studio (Native Windows toolchain)

1. **Install Visual Studio 2022** with "Desktop development with C++" workload
2. **Install NASM** (Netwide Assembler) from https://www.nasm.us/
3. Add NASM to your PATH

## Building with MinGW-w64

The existing Makefile works with MinGW-w64 with minor adjustments:

```bash
# In MSYS2 MinGW 64-bit terminal
cd /path/to/modern-gwbasic-64bit
make
```

The assembler syntax is compatible with GNU `as` which comes with MinGW-w64.

## Building with Visual Studio

For Visual Studio, you'll need to adapt the assembly files to MASM syntax:

### Step 1: Convert Assembly Syntax

The current assembly files use AT&T syntax (GNU as). For MASM, you need Intel syntax.

**Example conversion for `asm_core_amd64.S` → `asm_core_amd64.asm`:**

```asm
; MASM syntax (Intel)
.code

asm_add_int PROC
    mov     rax, rcx        ; First parameter in rcx (Windows x64 calling convention)
    add     rax, rdx        ; Second parameter in rdx
    ret
asm_add_int ENDP

; ... (similar conversions for other functions)

END
```

**Key differences:**
- Windows x64 calling convention: RCX, RDX, R8, R9 (not RDI, RSI)
- Intel syntax: `mov dest, src` (not AT&T `mov src, dest`)
- XMM0, XMM1 for floating-point (same as Linux)

### Step 2: Create Visual Studio Project

1. Create a new "Console App" project
2. Add all `.c` files from `src/`
3. Add converted `.asm` files
4. Configure project:
   - **C/C++ → General → Additional Include Directories:** `include`
   - **Linker → Input → Additional Dependencies:** Add `legacy_stdio_definitions.lib` if needed
   - **Build Events → Custom Build Tool** for `.asm` files:
     ```
     ml64 /c /Fo$(IntDir)%(Filename).obj %(FullPath)
     ```

### Step 3: Build

Press F7 or Build → Build Solution

## Cross-Platform Compatibility Notes

### C99 Compliance

The code is written in C99 for maximum portability:
- Uses `<stdint.h>` for fixed-width integers
- Uses `<stdbool.h>` for boolean types
- Avoids C++ features and C11/C23 extensions

### Assembly Portability

**AMD64 (x86_64):**
- Linux/macOS: AT&T syntax with GNU `as`
- Windows: Intel syntax with MASM (`ml64.exe`)

**ARM64 (AArch64):**
- Linux/macOS: GNU `as` syntax
- Windows: ARM64 assembly with `armasm64.exe`

### Calling Conventions

**AMD64:**
- **System V (Linux/macOS):** RDI, RSI, RDX, RCX, R8, R9
- **Windows x64:** RCX, RDX, R8, R9

**ARM64:**
- **AAPCS64 (all platforms):** X0-X7 for integers, D0-D7 for floats

## Automated Build Script (Future Enhancement)

A future enhancement would be a `build.bat` script for Windows:

```batch
@echo off
REM Detect compiler and build accordingly
where cl >nul 2>&1
if %ERRORLEVEL% == 0 (
    echo Building with Visual Studio...
    REM Add MSVC build commands
) else (
    where gcc >nul 2>&1
    if %ERRORLEVEL% == 0 (
        echo Building with MinGW-w64...
        make
    ) else (
        echo Error: No compiler found
        exit /b 1
    )
)
```

## Testing on Windows

After building, test with the example programs:

```bash
gwbasic.exe examples\hello.bas
gwbasic.exe examples\for_loop.bas
gwbasic.exe examples\comprehensive_demo.bas
```

## Known Issues

1. **File paths:** Windows uses backslashes (`\`), but the interpreter accepts forward slashes (`/`) as well
2. **Line endings:** Windows uses CRLF, but the parser handles both CRLF and LF
3. **Console encoding:** May need to set UTF-8 encoding: `chcp 65001`

## Performance on Windows

The assembly-optimized core provides the same performance benefits on Windows as on Linux:
- 2-3x faster arithmetic compared to pure C
- O(1) statement dispatch with hash tables
- Minimal overhead for function calls

## Contributing

If you create MASM-compatible assembly files, please submit them as:
- `src/asm_core_amd64_masm.asm`
- `src/asm_core_arm64_masm.asm`

This allows the project to support both GNU and MASM toolchains.
