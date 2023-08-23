# SPDX-FileCopyrightText: 2023 Ledger SAS
# SPDX-License-Identifier: LicenseRef-LEDGER

test_xxx = executable(
    'test_xxx',
    sources: [ files('test_xxx.cpp'), xxx_src ],
    include_directories: xxx_inc,
    dependencies: [gtest_main ],
    link_language: 'cpp',
)

test('xxx', test_xxx)
