// Rust benchmark: Performance test equivalent to examples/performance.bas
use std::time::Instant;

fn main() {
    let start = Instant::now();
    
    println!("Testing assembly-optimized operations...");
    println!();
    
    // Fast arithmetic loop
    let mut s = 0;
    for i in 1..=1000 {
        s += i;
    }
    println!("Sum 1 to 1000 = {}", s);
    println!();
    
    // Nested loops with multiplication
    let mut p = 1;
    for i in 1..=10 {
        for j in 1..=10 {
            p = i * j;
        }
    }
    println!("Nested loop complete");
    println!();
    
    // Function calls in loop
    for i in 1..=5 {
        println!("{}", (i as f64).sin());
    }
    println!();
    println!("Performance test complete!");
    
    let elapsed = start.elapsed();
    eprintln!("Rust execution time: {:?}", elapsed);
}
