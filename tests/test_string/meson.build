# SPDX-FileCopyrightText: 2023 Ledger SAS
# SPDX-License-Identifier: LicenseRef-LEDGER

test_string = executable(
    'test_string',
    sources: [ files('test_string.cpp'), shield_clib_sourceset_config.sources() ],
    include_directories: shield_inc,
    dependencies: [gtest_main],
    link_language: 'cpp',
    c_args: '-DTEST_MODE=1',
    cpp_args: '-DTEST_MODE=1',
)

test('string', test_string)
