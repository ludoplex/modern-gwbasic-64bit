// Rust benchmark: String operations
use std::time::Instant;

fn main() {
    let start = Instant::now();
    
    let a = "Hello World";
    println!("String: {}", a);
    println!("LEN = {}", a.len());
    println!("LEFT$(5) = {}", &a[..5]);
    println!("RIGHT$(5) = {}", &a[a.len()-5..]);
    println!("MID$(7,5) = {}", &a[6..11]);
    
    let elapsed = start.elapsed();
    eprintln!("Rust execution time: {:?}", elapsed);
}
