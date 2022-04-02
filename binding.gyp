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
                    'src/wmi/wmiresult.cpp',
                    'src/restclient/connection.cc',
                    'src/restclient/helpers.cc',
                    'src/restclient/restclient.cc',
                    ],

      'include_dirs': ["<!@(node -p \"require('node-addon-api').include\")",],
      'defines': ['CURL_STATICLIB', 'HTTP_ONLY'],
       'conditions': [
          ['OS=="mac"', {'sources/': [
            ['include', '_mac\\.cc|mm?$'],
            ['exclude', '_win\\.cc$'],
            ['exclude', 'wmi\\.cpp'],
            ['exclude', 'WinHttpClient\\.cpp'],
          ],
             "libraries": [
            '-framework AppKit',
            ],
            "include_dirs": [
              '/usr/local/opt/openssl@1.1/include'
            ],
            'link_settings': {
              'libraries': [
                # This statically links libcrypto, whereas -lcrypto would dynamically link it
                '/Users/xy/node-sysutilities/dep/curl-Darwin64/libcrypto.a',
                '/Users/xy/node-sysutilities/dep/curl-Darwin64/libcurl.a',
                '/Users/xy/node-sysutilities/dep/curl-Darwin64/libssl.a'
              ]
            }
          },
          ],
          ['OS=="win"', {'sources/': [
            ['include', '_win\\.cc$'],
            ['exclude', '_mac\\.cc|mm?$'],
        ], 
          'sources': [ 
            'src/WinHttpClient/RegExp.cpp',
            'src/WinHttpClient/StringProcess.cpp',
            'src/WinHttpClient/WinHttpClient.cpp'
          ],
        'include_dirs': [
          './dep/openssl-include',
          './dep/curl-include'
        ], 'link_settings': {
          'libraries': [
            '-llibcrypto.lib',
            '-llibssl.lib',
            '-lcrypt32.lib',
            '-lwldap32.lib',
            '-llibcurl.lib',
            '-lws2_32.lib'
          ], 'library_dirs': [
            './dep/OpenSSL-Win64',
            './dep/curl-Win64',
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
        'VCCLCompilerTool': {
          'ExceptionHandling': 1,
          'RuntimeLibrary': 0
        },
        "VCLinkerTool": {
                "AdditionalLibraryDirectories": [
                ],
                'AdditionalOptions': [ '/NODEFAULTLIB:LIBCMT.lib' ],
              }
      }
    }
  ]
}