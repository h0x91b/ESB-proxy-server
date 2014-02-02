protoc -I=src/ --cpp_out=src/ src/command.proto

V=1 node-gyp configure rebuild && node test/testCommander.js