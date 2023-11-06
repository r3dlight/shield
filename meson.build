# SPDX-FileCopyrightText: 2023 Ledger SAS
# SPDX-License-Identifier: LicenseRef-LEDGER

project('libshield', 'c', 'cpp',
    meson_version: '>=1.2.0',
    version: run_command('dunamai', 'from', 'git', '--style', 'semver', '--dirty', check: true).stdout().strip(),
    default_options: ['c_std=gnu11', 'cpp_std=gnu++20', 'config=configs/defconfig'])

top_dir = meson.current_source_dir()

kconfig_file = meson.current_source_dir() / 'Kconfig'
kconfig_proj = subproject('kconfig', default_options: ['kconfig=@0@'.format(kconfig_file)])
kconfig_h = kconfig_proj.get_variable('kconfig_h')
kconfig_data = kconfig_proj.get_variable('kconfig_data')

# about libshield basics

shield_inc = include_directories('include')

ssmod = import('sourceset')

# starting with empty set
shield_clib_sourceset = ssmod.source_set()

add_project_arguments('-include', '@0@'.format(kconfig_h), language : ['c'])
shield_clib_sourceset.add(kconfig_h)

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
