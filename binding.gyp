{
  "includes": [ "common.gypi" ],
  'targets': [
    {
      'target_name': 'node_sysutilities',
      'sources': [ 'src/addon.cc',
                    'src/file_utilities_win.cc',
                    'src/file_utilities_mac.mm'],

      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")",],
       'conditions': [
          ['OS=="mac"', {'sources/': [
            ['include', '_mac\\.cc|mm?$'],
            ['exclude', '_win\\.cc$']
          ],
             "libraries": [
            '-framework AppKit',
            ]
          },
          ],
          ['OS=="win"', {'sources/': [
            ['include', '_win\\.cc$'],
            ['exclude', '_mac\\.cc|mm?$'],
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