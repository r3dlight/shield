# SPDX-FileCopyrightText: 2023 Ledger SAS
# SPDX-License-Identifier: LicenseRef-LEDGER

project('libshield', 'c', 'cpp', 'rust',
    meson_version: '>=1.3.0',
    version: run_command('dunamai', 'from', 'git', '--style', 'semver', '--dirty', check: true).stdout().strip(),
    default_options: [
        'c_std=gnu11', 'cpp_std=gnu++20', 'rust_std=2021',
        'warning_level=3', 'optimization=s',
        'default_library=static', 'b_pie=false', 'b_staticpic=false',
        'licensedir=share/info/libshield',
        'pkgconfig.relocatable=true',
        'config=configs/defconfig'
])

pkg = import('pkgconfig')

kconfig_file = meson.current_source_dir() / 'Kconfig'
kconfig_proj = subproject('kconfig', default_options: ['kconfig=@0@'.format(kconfig_file)])
kconfig_h = kconfig_proj.get_variable('kconfig_h')
kconfig_data = kconfig_proj.get_variable('kconfig_data')

shield_external_deps = []
# XXX
shield_link_args = [ '-lgcc' ]

# Only build uapi, if configured in Kconfig **and** while cross compiling
if meson.is_cross_build()
if kconfig_data.get('CONFIG_WITH_SENTRY', 0) == 1

# in case of fallback to subproject, pass default_options to subproject (i.e. sentry)
# As sentry wrap file declares as `uapi `provider, fallback arg is implicit
uapi_dep = dependency('uapi',
    default_options: ['with_kernel=false', 'with_idle=false'],
)

shield_external_deps += uapi_dep

# XXX: **do not** add gc-section here, it will be done at final link step w/ kernel and other apps
shield_link_args += [
    '-Wl,--no-undefined',
    '-z', 'noexecstack',
    '-nostdlib',
    '-nodefaultlibs',
    '-nostartfiles'
]
endif # w/ sentry
endif # cross build

# about libshield basics

ssmod = import('sourceset')
shield_clib_sourceset = ssmod.source_set()

add_project_arguments('-include', '@0@'.format(kconfig_h), language : ['c'])
shield_clib_sourceset.add(kconfig_h)

subdir('include')
subdir('src')

shield_c_dep = declare_dependency(
    include_directories: [ shield_inc ],
    link_with: [ shield_clib ],
    dependencies: [ shield_external_deps ],
    link_args: [ shield_link_args ],
)

pkg.generate(shield_clib,
    filebase: 'shield',
    libraries: shield_link_args,
)


with_tests_opt = get_option('with_tests')
if with_tests_opt
    subdir('tests')
endif

# about documentation building
with_doc_opt = get_option('with_doc')
if with_doc_opt
    subdir('doc')
endif
