shield_clib_src += files('errno.c',
                         'string.c',
                         'arpa/inet.c',
                         'errno.h',
                         'support/sentry.c',
                         'support/sentry.h')

shield_clib = static_library(
    'shield_c_lib',
    sources : shield_clib_src,
    include_directories: shield_inc,
    #link_with : rust_lib,
)
