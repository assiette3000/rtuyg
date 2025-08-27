project('sdldemo', 'cpp',
        default_options: 'default_library=static')

sdl3_dep = dependency('sdl3')
sdl3_image_dep = dependency('sdl3-image')
#sdl2_main_dep = dependency('sdl3main')

executable('sdlprog', 'src/main.cpp',
  #win_subsystem: 'windows',
           dependencies: [sdl3_dep, sdl3_image_dep])#, sdl2_main_dep])

