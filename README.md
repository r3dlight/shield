<!--
SPDX-FileCopyrightText: 2023-2024 Ledger SAS
SPDX-License-Identifier: Apache-2.0
-->

# libShield

Sentry kernel and embedded hardened micro-libC

This library aim to support the Sentry kernel API exported by Sentry and to propose
a complete enough POSIX-compliant API for C runtime and a Rust support to support libCore and
potentially libstd.

The libShield implementation is built with security in mind, targetting hardened, fault
resilient implementation, and delivers what is required from an embedded runtime that supports
userspace tasks, including task environment initialisation and configuration abstraction,
standard symbols and OSS tooling compliance, and usual build, test and delivery best practices.

## Building libShield

Configuring the library
```console
$ meson setup builddir
$ cd builddir
$ ninja
```

## Running unit tests

```console
$ meson setup -Dwith_tests=true builddir
$ cd builddir
$ ninja test
```
