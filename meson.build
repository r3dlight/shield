# SPDX-FileCopyrightText: 2023 Ledger SAS
# SPDX-License-Identifier: LicenseRef-LEDGER

project('xxx', 'c', 'cpp',
    meson_version: '>=1.1.0',
    version: run_command('dunamai', 'from', 'git', '--style', 'semver', '--dirty', check: true).stdout().strip(),
    default_options: ['c_std=gnu11', 'cpp_std=gnu++20' ])

top_dir = meson.current_source_dir()

# about libxxx deps

xxx_inc = include_directories('include')
xxx_src = files('src/xxx.c')

# when not a subproject (standalone build), add locally specified autogen header
# based on local defconfig. This is also included for unit testing
if not meson.is_subproject()
    subdir('configs')
    add_project_arguments(
        '-include', '@0@'.format(config_h),
        language : ['c', 'cpp']
    )
    xxx_src += config_h
endif

with_tests_opt = get_option('with_tests')
if with_tests_opt
    subdir('tests')
endif

xxx_lib = static_library(meson.project_name(),
    sources: xxx_src,
    include_directories: xxx_inc,
    link_language: 'c',
)

xxx_dep = declare_dependency(
    link_with: xxx_lib,
    include_directories: xxx_inc,
)

# about documentation building
with_doc_opt = get_option('with_doc')
if with_doc_opt
    subdir('doc')
endif
