
g++ -g -Wall plugin.cc   -I /home/schen/llvm-3.4.2.src/tools/clang/include/ -I /home/schen/llvm-3.4.2.src/build/tools/clang/include/ -I /home/schen/llvm-3.4.2.src/include/ -I /home/schen/llvm-3.4.2.src/build/include/ -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS -c -fpic && gcc -shared -o indexer.so indexer.o

g++ -Wall -g  indexer.cc -I /home/schen/llvm-3.4.2.src/tools/clang/include/ -I /home/schen/llvm-3.4.2.src/build/tools/clang/include/ -I /home/schen/llvm-3.4.2.src/include/ -I /home/schen/llvm-3.4.2.src/build/include/ -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS /home/schen/llvm-3.4.2.src/build/lib/libclang.so -fno-rtti

