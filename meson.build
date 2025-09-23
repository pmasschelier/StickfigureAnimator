project('StickfigureAnimator', 'c',
  version : '0.1',
  default_options : [
    'warning_level=3',
    'c_std=c23',
  ]
)

cc = meson.get_compiler('c')

cmake = import('cmake')
raylib_opts = cmake.subproject_options()
raylib_opts.set_install(false)
raylib_opts.add_cmake_defines({
  'BUILD_EXAMPLES': 'OFF',
  # Add other CMake options for Raylib, if needed
})
raylib_subproject = cmake.subproject('raylib', options: raylib_opts)
raylib_dep = raylib_subproject.dependency('raylib')

m = cc.find_library('m')
glew = dependency('glew')
opengl = dependency('opengl')

subdir('src')
executable('StickfigureAnimator',
           sources,
           include_directories: ['externals'],
           c_args: ['-DRESOURCE_PATH="' + meson.current_source_dir() / '"'],
           dependencies: [m, raylib_dep, glew, opengl],
           install : true)
