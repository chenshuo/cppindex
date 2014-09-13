LLVM_PATH=$HOME/download/llvm-3.4.2.src

CPP_ARGS="-I $LLVM_PATH/tools/clang/include/ \
  -I $LLVM_PATH/build/tools/clang/include/ \
  -I $LLVM_PATH/include/ \
  -I $LLVM_PATH/build/include/ \
  -I $HOME/build/release-install/include/ \
  -I $HOME/git/leveldb/include/ \
  -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS \
  -DLLVM_PATH=\"$LLVM_PATH\" "

set -x

# g++ -c -g -Wall plugin.cc $CPP_ARGS -fpic && gcc -shared -o indexer.so plugin.o

LIB=$LLVM_PATH/build/lib
#LIB=/home/schen/download/clang+llvm-3.4.2-x86_64-unknown-ubuntu12.04/lib
LIBS="-lclangTooling -lclangRewriteCore -lclangFrontend -lclangSerialization \
 -lclangDriver -lclangSema -lclangAnalysis -lclangEdit -lclangAST \
 -lclangParse -lclangSema -lclangLex -lclangBasic \
 -lLLVMBitReader -lLLVMMCParser -lLLVMMC -lLLVMTransformUtils \
 -lLLVMOption -lLLVMCore -lLLVMSupport "
LIBS=
CC=g++

$CC -std=c++11 -fno-rtti -g -Wall indexer.cc record.pb.o $CPP_ARGS \
  -L $LIB \
  -L $HOME/build/release-install/lib \
  -L $HOME/git/leveldb \
  -Wl,-rpath=$LIB \
  $LIBS -lclang -lLLVMSupport \
  -lmuduo_base -lleveldb -lprotobuf -lsnappy -ldl -ltinfo -lpthread

