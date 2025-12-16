# Modern GW-BASIC 64-bit - Branchless Edition

This repository contains both a **completely branchless 64-bit GW-BASIC interpreter** implementation and a historical collection of GW-BASIC programs and resources.

## Branchless 64-bit GW-BASIC Interpreter

A cutting-edge, **zero-branch** 64-bit GW-BASIC interpreter built with modern systems programming techniques.

### Key Features

- ✅ **ZERO `if`/`switch` statements** - Completely branchless implementation
- ✅ **64-bit native** - Full 64-bit integer and floating-point support
- ✅ **SIMD accelerated** - AVX-512 (x86-64) and NEON (ARM64) optimizations
- ✅ **Portable** - C99 standard, builds with gcc/clang/cosmocc
- ✅ **1,685 lines** of branchless C code
- ✅ **GW-BASIC compatible** - Runs classic GW-BASIC programs

### Quick Start

```bash
# Build the interpreter
make

# Verify branchless implementation
make verify

# Run a program
./gwbasic examples/hello.bas

# Interactive mode
./gwbasic
```

### Example Program

```basic
10 PRINT "Hello, World!"
20 PRINT "GW-BASIC in 2025!"
30 END
```

### Documentation

- **[IMPLEMENTATION.md](IMPLEMENTATION.md)** - Detailed architecture and implementation guide
- **[examples/](examples/)** - Example GW-BASIC programs
- **[src/](src/)** - Source code with extensive comments

### Verification

The implementation uses **ZERO** `if` or `switch` statements:

```bash
$ make verify
✓ Verification passed: No if/switch statements found!
```

### Architecture

```
src/
├── branchless_amd64.h      # AVX-512 intrinsics
├── branchless_aarch64.h    # NEON intrinsics  
├── branchless_scalar.h     # Portable operations
├── arena.h                 # Branchless arena allocator
├── hash_table.h            # Robin Hood hash table
├── lexer.c/h               # Branchless tokenizer
├── interpreter.c/h         # Core interpreter
├── basic_functions.c/h     # Built-in functions
└── main.c                  # Entry point
```

### Building

```bash
# Default build (gcc)
make

# Build with clang
make clang

# Build with cosmocc (fat APE binary)
make cosmo

# Clean build artifacts
make clean
```

### Testing

```bash
# Run verification
make verify

# Run example programs
./gwbasic examples/hello.bas
./gwbasic examples/counting.bas
./gwbasic examples/multiline.bas
```

---

## Historical GW-BASIC Program Collection

Hoard of GW-BASIC
=================

This repository is a collection of programs, tutorials and other resources on GW-BASIC for the PC, Tandy and PCjr.
It focuses on source code, supporting files and documentation only. Compiled binaries and sources in other BASIC dialects are generally omitted.

Materials from the following sources are included:

Programs
--------

- [KindlyRat's Geocities page](http://www.oocities.org/KindlyRat/GWBASIC.html)  
- [PeatSoft collection](http://archive.is/AUm6G)
- [Leon Peyre's Back to BASICs](http://peyre.x10.mx/GWBASIC/)  
- [Brooks deForest's Tandy 1000 EX/HX, PCjr videogames](https://web.archive.org/web/20170222075609/brooksdeforest.com/tandy1000)  
- [TVDog's Tandy 1000 Archive](http://www.oldskool.org/guides/tvdog/)
- [Phillip Bigelow's scientific programs](https://web.archive.org/web/20160810162309/http://www.scn.org/~bh162/basic_programs.html)    
- [Gary Peek's BASIC source code archive](http://www.garypeek.com/basic/gwprograms.htm)    
- [S.A. Moore's Classic BASIC Games page](http://www.moorecad.com/classicbasic/index.html)
- [Thomas C. McIntyre's GeeWhiz Collection](https://web.archive.org/web/20060410121551/http://scottserver.net/basically/geewhiz.html)  
- IBM-PC BASIC samples from the [MS-DOS source code](https://github.com/Microsoft/MS-DOS/tree/master/v1.25/bin)

Tutorials and Documentation
---------------------------

- [Microsoft GW-BASIC Manual](http://antonis.de/qbebooks/index.htm#gwbasman): an online version of the original User' Guide and User's Reference.  
- [BASIC Training](http://www.o-bizz.de/qbtuts/gw-train/): a GW-BASIC tutorial by Steve Estvanik.  
- [Blast Off With BASIC](http://www.o-bizz.de/qbtuts/blastoff/_start.htm): a GW-BASIC tutorial by Brian R. Page.  
- [Joseph Sixpack's Last Book of GW-BASIC](http://www.geocities.ws/joseph_sixpack/btoc.html)   
- [The BLUE Book about GW-BASIC and QuickBASIC](http://www.antonis.de/qbebooks/bluebas.zip): the excellent "BASIC Language User Essay" by Thomas C. McIntire is full of expert knowledge about GW-BASIC.  

Further Resources
-----------------

Other places to look for GW-BASIC material:

- [cd.textfiles.com](http://cd.textfiles.com) has tons of old shareware, among which some good GW-BASIC games.  
- [PC-SIG Library 8th Edition CD-ROM (April 1990)](https://www.pcjs.org/disks/pcx86/shareware/pcsig08/) hosts many disk images and features a "live" DOS box. 
- [gw-basic.netlify.app](https://gw-basic.netlify.app/) is another page dedicated to GW-BASIC material, tutorials, downloads and further links.
