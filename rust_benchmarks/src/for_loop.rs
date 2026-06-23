// Rust benchmark: FOR loop equivalent to examples/for_loop.bas
use std::time::Instant;

fn main() {
    let start = Instant::now();
    
    println!("FOR loop test:");
    for i in 1..=10 {
        println!("{}", i);
    }
    println!("Loop complete!");
    
    let elapsed = start.elapsed();
    eprintln!("Rust execution time: {:?}", elapsed);
}
