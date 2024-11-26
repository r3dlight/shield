// SPDX-FileCopyrightText: 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0

#![cfg_attr(target_os = "none", no_std)]
#![cfg_attr(target_os = "none", no_main)]

extern crate shield;
use shield::println;

#[cfg(target_os = "none")]
shield::shield_main!();

fn main() {
    println!("Hello, World !");
}
