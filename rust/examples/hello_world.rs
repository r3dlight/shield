#![cfg_attr(target_os = "none", no_std)]
#![cfg_attr(target_os = "none", no_main)]

extern crate shield;
use shield::println;

#[cfg(target_os = "none")]
shield::shield_main!();

fn main() {
    println!("Hello, World !");
}
