{
  "includes": [ "common.gypi" ],
  'targets': [
    {
      'target_name': 'node_sysutilities',
      'sources': [ 'src/addon.cc',
                    'src/file_utilities_win.cpp',
                    'src/file_utilities_mac.mm'],

      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")",],
       'conditions': [
          ['OS!="linux"', {'sources/': [['exclude', '_linux\\.cc$']]}],
          ['OS!="mac"', {'sources/': [['exclude', '_mac\\.cc|mm?$']]}],
          ['OS=="win"', {'sources/': [
            ['include', '_win\\.cc$'],
            ['exclude', '_posix\\.cc$'],
        ]}],
       ],
      'dependencies': ["<!(node -p \"require('node-addon-api').gyp\")",
      ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'xcode_settings': {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
        'CLANG_CXX_LIBRARY': 'libc++',
        'MACOSX_DEPLOYMENT_TARGET': '10.7'
      },
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
        "VCLinkerTool": {
                "AdditionalLibraryDirectories": [
                ]
              }
      }
    }
  ]
}