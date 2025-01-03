// SPDX-FileCopyrightText: 2024 Ledger SAS
//
// SPDX-License-Identifier: Apache-2.0 OR BSD-3-Clause

//! # shield-macros
//!
//! Procedural macros collection for generating startup glue between task entrypoint and
//! user defined main.
//! For Rust/Cargo based tasks, boot flow is the following:
//! `_start` --> `main` --> `crate::main`
//!  - `_start` is the task entrypoint, symbol name is mandatory and resolve at buildtime
//!    in by barbican after tasks relocation.
//!  - `main` (w/o mangling) is called by `_start` after shield initialization. This
//!    symbol must be able to resolve crate::main, i.e. main that is defined in task
//!    `main.rs` file.

extern crate proc_macro;

use proc_macro::TokenStream;
use quote::quote;
use syn::{parse, parse_macro_input};

/// Generate shield entrypoint function
///
/// # Usage
///
/// Procedural macro that generates a not mangled `main` function that call user
/// defined main. This macro takes no arguments and must be call from `main.rs` file.
///
/// > **NOTE**: The function is generated in an inner module named `__shield_startup`
/// > **TODO**: use an inner attribute macro once stable (this is still a nightly feature)
///
/// # Example
//
/// ```rust
/// #![no_std]
/// #![no_main]
/// extern crate shield;
///
/// shield_main!():
///
/// fn main() {
///     [...]
/// }
/// ```
#[proc_macro]
pub fn shield_main(tokens: TokenStream) -> TokenStream {
    let _ = parse_macro_input!(tokens as parse::Nothing);
    quote! {
        #[doc(hidden)]
        mod __shield_startup {
            #[doc(hidden)]
            #[no_mangle]
            pub fn main() { crate::main() }
        }
    }
    .into()
}
