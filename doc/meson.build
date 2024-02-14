# SPDX-FileCopyrightText: 2023 Ledger SAS
# SPDX-License-Identifier: LicenseRef-LEDGER

pymod = import('python')
sphinx = find_program('sphinx-build')
doxygen = find_program('doxygen')
py3 = pymod.find_installation(
    'python3',
    modules: [
      'exhale',
      'sphinx_rtd_theme',
      'sphinx_simplepdf',
    ]
  )

# used by sphinx_simplepdf
weasyprint = find_program('weasyprint', required: true)


shield_rst = files(
    'shield/posix/model.rst',
    'shield/posix/pse51/stdio.rst',
    'shield/posix/pse51/time.rst',
    'shield/posix/pse51.rst',
    'shield/posix/index.rst',
    'shield/_static/figures/outpost_fp.png',
    'shield/genindex.rst',
    'shield/tests/unittests.rst',
    'shield/tests/formal.rst',
    'shield/tests/index.rst',
    'shield/build_system/shield_build.rst',
    'shield/build_system/index.rst',
    'shield/conf.py',
    'shield/concepts/security.rst',
    'shield/concepts/index.rst',
    'shield/concepts/initial.rst',
    'shield/index.rst',
)

shield = custom_target('doc-shield',
    input: shield_rst,
    depend_files: [files('shield/conf.py')],
    output: 'shield',
    command: [sphinx, '-b', 'html', '-q', '-d',
        'build/doctrees',
        '-Dversion=' + meson.project_version(),
        join_paths(meson.current_source_dir(), 'shield'),
        'doc/shield',
    ])
doc_shield = alias_target('doc-shield', shield)

shield_singlehtml = custom_target('doc-shield-singlehtml',
    input: shield_rst,
    depend_files: [files('shield/conf.py')],
    output: 'shield_singlehtml',
    command: [sphinx, '-b', 'singlehtml', '-q', '-d',
        'build/doctrees',
        '-Dversion=' + meson.project_version(),
        join_paths(meson.current_source_dir(), 'shield'),
        'doc/shield_singlehtml',
    ])
doc_shield_singlehtml = alias_target('doc-shield-singlehtml', shield_singlehtml)


shield_pdf = custom_target('doc-shield-pdf',
    input: shield_rst,
    depend_files: [files('shield/conf.py')],
    output: 'shield_pdf',
    command: [sphinx, '-b', 'simplepdf', '-q',
        '-d', 'build/doctrees',
        '-Dversion=' + meson.project_version(),
        join_paths(meson.current_source_dir(), 'shield'),
        'doc/shield_pdf',
    ])
doc_shield_pdf = alias_target('doc-shield-pdf', shield_pdf)

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
