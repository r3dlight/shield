# SPDX-FileCopyrightText: 2023 Ledger SAS
# SPDX-License-Identifier: LicenseRef-LEDGER

keyval = import('keyval')
defconfig = files(top_dir / 'configs/defconfig')

message('generating .config from defconfig file')
dotconfig = configure_file(
    input: defconfig,
    output: '.config',
    command: ['cp', '@INPUT@', '@OUTPUT@'],
)

# In standalone mode, a header is generated from the generated .config, so that it can be
# included if needed
config_data = configuration_data(keyval.load(dotconfig))

# Fixup: convert .config entry set as `y` to boolean 10 (i.e. 1: true, 0: false)
# TODO: use true/false boolean instead
foreach k: config_data.keys()
    if config_data.get(k) == 'y'
        config_data.set10(k, true)
    endif
endforeach

# Generate configuration header from config_data
config_h = configure_file(
    configuration: config_data,
    output: 'config.h'
)
