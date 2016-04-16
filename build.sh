LLVM_PATH=$HOME/llvm-3.5.0
MUDUO_PATH=$HOME/build/release-install
LEVELDB_PATH=$HOME/git/leveldb
LLVM_BUILD=build

CPP_ARGS="-I $LLVM_PATH/tools/clang/include/ \
  -I $LLVM_PATH/$LLVM_BUILD/tools/clang/include/ \
  -I $LLVM_PATH/include/ \
  -I $LLVM_PATH/$LLVM_BUILD/include/ \
  -I $MUDUO_PATH/include/ \
  -I $LEVELDB_PATH/include/ \
  -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS \
  -DLLVM_PATH=\"$LLVM_PATH\" "

set -x

# g++ -c -g -Wall plugin.cc $CPP_ARGS -fpic && gcc -shared -o indexer.so plugin.o

LIB=$LLVM_PATH/$LLVM_BUILD/lib
#LIB=/home/schen/download/clang+llvm-3.4.2-x86_64-unknown-ubuntu12.04/lib
LIBS="-lclangTooling -lclangRewriteCore -lclangFrontend -lclangSerialization \
 -lclangDriver -lclangSema -lclangAnalysis -lclangEdit -lclangAST \
 -lclangParse -lclangSema -lclangLex -lclangBasic \
 -lLLVMBitReader -lLLVMMCParser -lLLVMMC -lLLVMTransformUtils \
 -lLLVMOption -lLLVMCore -lLLVMSupport "
LIBS=
CC=/home/schen/download/llvm-3.4.2.src/build-O2/bin/clang++
CC=$HOME/downloads/clang+llvm-3.4.2-x86_64-unknown-ubuntu12.04/bin/clang++
CC=g++

$CC -std=c++11 -fno-rtti -g -Wall indexer.cc record.pb.o $CPP_ARGS \
  -L $LIB \
  -L $MUDUO_PATH/lib \
  -L $LEVELDB_PATH \
  -Wl,-rpath=$LIB \
  $LIBS -lclang -lLLVMSupport \
  -lmuduo_base -lleveldb -lprotobuf -lsnappy -ldl -ltinfo -lpthread

$CC -std=c++11 -fno-rtti -g -Wall -o b.out printer.cc record.pb.o $CPP_ARGS \
  -L $LIB \
  -L $MUDUO_PATH/lib \
  -L $LEVELDB_PATH \
  -Wl,-rpath=$LIB \
  $LIBS -lclang -lLLVMSupport \
  -lmuduo_base -lleveldb -lprotobuf -lsnappy -ldl -ltinfo -lpthread

$CC -std=c++11 -g -Wall -o dump ldb.cc record.pb.o $CPP_ARGS \
  -L $LEVELDB_PATH \
  -lleveldb -lsnappy -lpthread -lprotobuf

