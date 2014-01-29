{
  "targets": [
    {
      "target_name": "proxy",
      "sources": [ "src/proxy.cc" ],
      "cflags": [
          '-I/usr/local/include/'
      ],
      "xcode_settings": {
          "OTHER_CFLAGS": [
              '-I/usr/local/include/'
          ]
      },
      'libraries': [
        '-lzmq', '-L/usr/local/lib/'
      ]
    }
  ]
}