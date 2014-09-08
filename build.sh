LLVM_PATH=$HOME/download/clang+llvm-3.4.2-x86_64-unknown-ubuntu12.04

CPP_ARGS="-I $LLVM_PATH/tools/clang/include/ \
  -I $LLVM_PATH/build/tools/clang/include/ \
  -I $LLVM_PATH/include/ \
  -I $LLVM_PATH/build/include/ \
  -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS \
  -std=c++11"

set -x

g++ -c -g -Wall plugin.cc $CPP_ARGS -fpic && gcc -shared -o indexer.so plugin.o

g++ -fno-rtti -g -Wall indexer.cc $CPP_ARGS \
  -L $LLVM_PATH/lib \
  -lclangTooling -lclangRewriteCore -lclangFrontend -lclangSerialization \
  -lclangDriver -lclangSema -lclangAnalysis -lclangEdit -lclangAST \
  -lclangParse -lclangSema -lclangLex -lclangBasic \
  -lLLVMBitReader -lLLVMMCParser -lLLVMMC -lLLVMTransformUtils \
  -lLLVMOption -lLLVMCore -lLLVMSupport \
  -ldl -ltinfo -lpthread

