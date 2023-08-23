# SPDX-FileCopyrightText: 2023 Ledger SAS
# SPDX-License-Identifier: LicenseRef-LEDGER

project('libshield', 'c',
    meson_version: '>=1.1.0',
    version: run_command('dunamai', 'from', 'git', '--style', 'semver', '--dirty', check: true).stdout().strip(),
    default_options: ['c_std=gnu11', 'cpp_std=gnu++20' ])

top_dir = meson.current_source_dir()

# about libshield basics

shield_inc = include_directories('include')

# starting with empty set
shield_clib_src = files()

# when not a subproject (standalone build), add locally specified autogen header
# based on local defconfig. This is also included for unit testing
if not meson.is_subproject()
    subdir('configs')
    add_project_arguments(
        '-include', '@0@'.format(config_h),
        language : ['c']
    )
    shield_clib_src += config_h
endif

subdir('src/c')


with_tests_opt = get_option('with_tests')
if with_tests_opt
    subdir('tests')
endif

# about documentation building
with_doc_opt = get_option('with_doc')
if with_doc_opt
    subdir('doc')
endif
