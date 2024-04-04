# for private headers, only compile time
subdir('include')

shield_clib_sourceset.add(files(
    'errno.c',
    'string.c',
    'arpa/inet.c',
    'pthread.c',
    'time.c',
    'printf.c',
    'printf_lexer.c',
    'rand.c',
    'sys/msg.c',
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
    sources : [ shield_clib_sourceset_config.sources(), shield_private_headers ],
    include_directories: [ shield_inc, shield_private_inc ],
    dependencies: [ shield_external_deps ],
    install: true,
)
