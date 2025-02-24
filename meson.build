project('StickfigureAnimator', 'c',
  version : '0.1',
  default_options : [
    'warning_level=3',
    'cpp_std=c++20',
    'cpp_args=-Wno-missing-field-initializers -Wno-reorder-init-list'
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

executable('StickfigureAnimator',
           'main.c',
           include_directories: ['externals'],
           dependencies: [m, raylib_dep],
           install : true)
