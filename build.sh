#!/bin/bash

export PATH=$PATH:/usr/local/bin/

case $1 in
  clean)
    V=1 node-gyp clean
    ;;
  *)
    echo "Flag: $1"
    protoc -I=src/ --cpp_out=src/ src/command.proto
    protoc src/command.proto -o src/command.desc
    #V=1 node-gyp --debug reconfigure rebuild
    V=1 node-gyp reconfigure rebuild
    ;;
esac

