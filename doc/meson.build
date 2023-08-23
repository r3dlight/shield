# SPDX-FileCopyrightText: 2023 Ledger SAS
# SPDX-License-Identifier: LicenseRef-LEDGER

pymod = import('python')
sphinx = find_program('sphinx-build')
doxygen = find_program('doxygen')
py3 = pymod.find_installation('python3', modules: ['exhale','sphinx_rtd_theme'])

api_rst = [
  'sphinx/deps.rst',
  'sphinx/index.rst',
  ]
api = custom_target('sphinx-doc',
    input: api_rst,
    depend_files: [files('sphinx/conf.py')],
    output: 'api',
    command: [sphinx, '-b', 'html', '-q', '-d',
        'build/doctrees',
        '-Dversion=' + meson.project_version(),
        meson.current_source_dir() / 'sphinx',
        'doc/api',
    ])
doc = alias_target('doc', api)
