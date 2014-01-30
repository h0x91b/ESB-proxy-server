{
  "targets": [
    {
      "target_name": "proxy",
      "sources": [ "src/proxy.cc" ],
      "cflags": [
          '-I/usr/local/include/', '-std=c++0x', '-pthread'
      ],
      "xcode_settings": {
          "OTHER_CFLAGS": [
              '-I/usr/local/include/', '-std=c++0x', '-pthread'
          ]
      },
      'libraries': [
        '-lzmq', '-L/usr/local/lib/'
      ]
    }
  ]
}