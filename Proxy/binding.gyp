{
  'targets': [
    {
      'target_name': 'proxy',
      'sources': [ 'src/command.pb.cc', 'src/proxy.cc' ],
      'cflags': [
          '-I/usr/local/include/', '-std=c++0x', '-pthread'
      ],
      'xcode_settings': {
          'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
          'OTHER_CFLAGS': [
              '-I/usr/local/include/', 
              '-pthread', 
              '-O3', 
              '-mmacosx-version-min=10.7',
              '-std=c++11',
              '-stdlib=libc++',
          ],
          'OTHER_CPLUSPLUSFLAGS': [
              '-I/usr/local/include/', 
              '-pthread', 
              '-O3', 
              '-mmacosx-version-min=10.7',
              '-std=c++11',
              '-stdlib=libc++',
          ]
      },
      'libraries': [
        '-lzmq', '-lprotobuf', '-L/usr/local/lib/'
      ]
    }
  ]
}