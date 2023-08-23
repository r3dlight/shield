# SPDX-FileCopyrightText: 2023 Ledger SAS
# SPDX-License-Identifier: LicenseRef-LEDGER

hostcc = meson.get_compiler('c', native: true)

gtest_main = dependency('gtest_main', version: '>=1.13.0',
                         fallback : ['gtest', 'gtest_main_dep'],
                         native: true)


# Extra flags if coverage is enabled
if get_option('b_coverage')
add_project_arguments(
[
    '-fprofile-abs-path',
    '-fsanitize=undefined',
    '-fsanitize-address-use-after-scope',
    '-fstack-protector-strong',
    '-fno-inline',
    '-fno-inline-small-functions',
    '-fno-default-inline',
],
language: ['c', 'cpp'],
)

add_project_link_arguments(
[
    '-fsanitize=undefined',
    '-fsanitize-address-use-after-scope',
    '-fstack-protector-strong',
],
language: ['c', 'cpp'],
)
endif

subdir('test_xxx')
