# SPDX-FileCopyrightText: 2023 Ledger SAS
# SPDX-License-Identifier: LicenseRef-LEDGER

fs = import('fs')

shield_headers += files([
    'errno.h',
    'pthread.h',
    'string.h',
    'sys/msg.h',
])

subdir('arpa')

foreach header: shield_headers
    install_headers(fs.relative_to(header, meson.current_source_dir()),
        preserve_path: true,
        follow_symlinks: false,
    )
endforeach
