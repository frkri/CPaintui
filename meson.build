project('CPaintui', 'c',
  version : '0.1',
  default_options : ['warning_level=3', 'c_std=c99'])

src = ['src/main.c', 'src/win.c', 'src/modal.c', 'src/logger.c', 'src/fs.c', 'src/cursor.c', 'src/util.c']

executable('cpaintui',
  src,
  dependencies : [
    dependency('ncursesw')
  ],
  install : true)
