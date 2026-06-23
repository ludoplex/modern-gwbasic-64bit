#!/bin/bash
# Benchmark GW-BASIC interpreter vs Rust

echo "=========================================="
echo "GW-BASIC vs Rust Performance Benchmark"
echo "=========================================="
echo ""

# Build Rust benchmarks
echo "Building Rust benchmarks..."
cd rust_benchmarks
cargo build --release 2>&1 | grep -E "(Compiling|Finished)"
cd ..
echo ""

# Build GW-BASIC interpreter
echo "Building GW-BASIC interpreter..."
make clean > /dev/null 2>&1
make 2>&1 | grep "Built"
echo ""

# Test 1: FOR loop
echo "Test 1: FOR Loop (1-10)"
echo "------------------------"
echo -n "GW-BASIC: "
(time ./gwbasic examples/for_loop.bas 2>&1) 2>&1 | grep "real" | awk '{print $2}'
echo -n "Rust:     "
(time ./rust_benchmarks/target/release/for_loop 2>&1) 2>&1 | grep "real" | awk '{print $2}'
echo ""

# Test 2: Math functions
echo "Test 2: Math Functions"
echo "----------------------"
echo -n "GW-BASIC: "
(time ./gwbasic examples/math_functions.bas 2>&1) 2>&1 | grep "real" | awk '{print $2}'
echo -n "Rust:     "
(time ./rust_benchmarks/target/release/math_functions 2>&1) 2>&1 | grep "real" | awk '{print $2}'
echo ""

# Test 3: String operations
echo "Test 3: String Operations"
echo "-------------------------"
echo -n "GW-BASIC: "
(time ./gwbasic examples/string_functions.bas 2>&1) 2>&1 | grep "real" | awk '{print $2}'
echo -n "Rust:     "
(time ./rust_benchmarks/target/release/string_ops 2>&1) 2>&1 | grep "real" | awk '{print $2}'
echo ""

# Test 4: Performance (1000 iterations)
echo "Test 4: Performance Test (1000 iterations)"
echo "-------------------------------------------"
echo -n "GW-BASIC: "
(time ./gwbasic examples/performance.bas 2>&1) 2>&1 | grep "real" | awk '{print $2}'
echo -n "Rust:     "
(time ./rust_benchmarks/target/release/performance 2>&1) 2>&1 | grep "real" | awk '{print $2}'
echo ""

echo "=========================================="
echo "Benchmark Complete"
echo "=========================================="
