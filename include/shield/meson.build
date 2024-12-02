# SPDX-FileCopyrightText: 2023 - 2024 Ledger SAS
# SPDX-License-Identifier: LicenseRef-LEDGER

fs = import('fs')

shield_headers += files([
    'errno.h',
    'pthread.h',
    'signal.h',
    'stdio.h',
    'stdlib.h',
    'string.h',
    'time.h',
])

subdir('arpa')
subdir('sys')

foreach header: shield_headers
    install_headers(fs.relative_to(header, meson.current_source_dir()),
        preserve_path: true,
        follow_symlinks: false,
    )
endforeach
