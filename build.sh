LLVM_PATH=$HOME/llvm-3.4.2.src

CPP_ARGS="-I $LLVM_PATH/tools/clang/include/ \
  -I $LLVM_PATH/build/tools/clang/include/ \
  -I $LLVM_PATH/include/ \
  -I $LLVM_PATH/build/include/ \
  -I $HOME/git/leveldb/include/ \
  -I $HOME/build/release-install/include/ \
  -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS \
  -std=c++0x"

set -x

# g++ -c -g -Wall plugin.cc $CPP_ARGS -fpic && gcc -shared -o indexer.so plugin.o

g++ -fno-rtti -g -Wall indexer.cc $CPP_ARGS \
  -L $LLVM_PATH/build/lib \
  -L $HOME/build/release-install/lib \
  -L $HOME/git/leveldb \
  -Wl,-rpath=$LLVM_PATH/build/lib \
  -lclang -lmuduo_base -lleveldb -ldl -ltinfo -lpthread

