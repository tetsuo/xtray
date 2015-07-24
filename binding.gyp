{
  'targets': [
    {
      'target_name': 'app',
      'type': 'shared_library',
      'mac_bundle': 1,
      'xcode_settings': {
        'CONFIGURATION_BUILD_DIR': 'build',
        'MACOSX_DEPLOYMENT_TARGET': '10.9',
        'EMBEDDED_CONTENT_CONTAINS_SWIFT': 'YES',
        'COMBINE_HIDPI_IMAGES': 'YES',
        'LD_RUNPATH_SEARCH_PATHS': "@loader_path/Frameworks"
      },
      'mac_framework_headers': [
        'src/app.h',
        'src/next.h'
      ],
      'sources': [
        'src/app.swift',
        'src/next.c'
      ],
    },
    {
      'dependencies': [ 'app' ],
      'target_name': 'addon',
      'sources': [
        'src/addon.cc'
      ],
      'include_dirs': [
        '<!(node -e \'require("nan")\')'
      ],
      'mac_framework_dirs': [ 'build' ],
      'link_settings': {
        'xcode_settings': {
          'CONFIGURATION_BUILD_DIR': 'build',
          'MACOSX_DEPLOYMENT_TARGET': '10.9',
          'OTHER_CPLUSPLUSFLAGS': [ '-std=c++11' ],
          'OTHER_LDFLAGS': [
            '-Wl,-rpath,<(module_root_dir)/build'
          ]
        }
      }
    }
  ]
}