// Rust benchmark: Math functions equivalent to examples/math_functions.bas
use std::time::Instant;

fn main() {
    let start = Instant::now();
    
    println!("SIN(1.5708) = ");
    println!("{}", 1.5708_f64.sin());
    println!("COS(0) = ");
    println!("{}", 0.0_f64.cos());
    println!("SQR(16) = ");
    println!("{}", 16.0_f64.sqrt());
    println!("ABS(-5) = ");
    println!("{}", (-5.0_f64).abs());
    println!("INT(3.7) = ");
    println!("{}", 3.7_f64.floor());
    
    let elapsed = start.elapsed();
    eprintln!("Rust execution time: {:?}", elapsed);
}
