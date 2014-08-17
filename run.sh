clang -cc1 -load ./indexer.so -plugin index ../../muduo/muduo/net/TcpServer.cc -I ../../muduo -std=c++0x -v
