{
  "includes": [ "common.gypi" ],
  'targets': [
    {
      'target_name': 'node_sysutilities',
      'sources': [ 'src/addon.cc',
                    'src/file_utilities_win.cc',
                    'src/file_utilities_mac.mm',
                    'src/registry_win.cc',
                    'src/wmi/wmi.cpp',
                    'src/wmi/wmiresult.cpp'],

      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")",],
       'conditions': [
          ['OS=="mac"', {'sources/': [
            ['include', '_mac\\.cc|mm?$'],
            ['exclude', '_win\\.cc$'],
            ['exclude', 'wmi\\.cpp'],
          ],
             "libraries": [
            '-framework AppKit',
            ]
          },
          ],
          ['OS=="win"', {'sources/': [
            ['include', '_win\\.cc$'],
            ['exclude', '_mac\\.cc|mm?$'],
        ], 'include_dirs': [
          './dep/openssl-include',
        ], 'link_settings': {
          'libraries': [
            '-llibcrypto.lib',
            '-llibssl.lib',
            '-lcrypt32.lib',
          ], 'library_dirs': [
            './dep/OpenSSL-Win64',
          ]
        }
        }],
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