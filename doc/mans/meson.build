pandoc = find_program('pandoc')

mandb = []

foreach posix_api_impl: posix_api
target_name = 'man_gen_syscall_' + fs.name(posix_api_impl)

mandb += custom_target(
  target_name,
  input: posix_api_impl,
  output: '@BASENAME@.3',
  command: [
     pandoc,
     '-f', 'rst',
     '-t', 'man',
     '-s',
     '-V', 'header=Shield programmer\'s manual',
     '-V', 'section=3',
     '-V', 'adjusting=both',
     '-o', '@OUTPUT@',
     '@INPUT@',
  ],
  install_tag: 'man',
  install : true,
  install_dir : join_paths(get_option('mandir'), 'man3')
)
endforeach

# man pages build can be trigger by mandb target
man_tgt = alias_target('mandb', mandb)
