g++ -g -Wall indexer.cc -I /usr/lib/llvm-3.4/include/ -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -c -fpic && gcc -shared -o indexer.so indexer.o
