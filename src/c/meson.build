shield_clib_sourceset.add(files(
    'errno.c',
    'string.c',
    'arpa/inet.c',
    'errno.h',
    'pthread.c',
))

shield_clib_sourceset.add(
    when: 'CONFIG_WITH_SENTRY',
    if_true: files(
        'support/sentry.c',
        'support/sentry.h',
        'entrypoint/thread_init.c',
        'entrypoint/libc_init.c')
)

# applying config, from local or parent
shield_clib_sourceset_config = shield_clib_sourceset.apply(kconfig_data, strict: false)

shield_clib = static_library(
    'shield_c_lib',
    sources : shield_clib_sourceset_config.sources(),
    include_directories: shield_inc,
    dependencies: [ shield_external_deps ],
)
