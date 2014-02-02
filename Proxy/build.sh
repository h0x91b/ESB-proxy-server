#!/bin/bash

export PATH=$PATH:/usr/local/bin/

case $1 in
  clean)
    V=1 node-gyp clean
    ;;
  *)
    echo "Flag: $1"
    protoc -I=src/ --cpp_out=src/ src/command.proto
    V=1 node-gyp --debug configure rebuild
    ;;
esac

