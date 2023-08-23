shield_clib_sourceset.add(files('errno.c',
                                'string.c',
                                'arpa/inet.c',
                                'errno.h',
                                'pthread.c'
                                )
                         )


if config_data.get('CONFIG_WITH_SENTRY', 0) == 1
shield_clib_sourceset.add(files('support/sentry.c',
                                'support/sentry.h',
                                'entrypoint/thread_init.c',
                                'entrypoint/libc_init.c')
                         )
endif

# applying config, from local or parent
shield_clib_sourceset_config = shield_clib_sourceset.apply(config_data)

shield_clib = static_library(
    'shield_c_lib',
    sources : shield_clib_sourceset_config.sources(),
    include_directories: shield_inc,
    #link_with : rust_lib,
)
