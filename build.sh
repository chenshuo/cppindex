LLVM_PATH=$HOME/llvm-3.4.2.src
CPP_ARGS="-I $LLVM_PATH/tools/clang/include/ \
  -I $LLVM_PATH/build/tools/clang/include/ \
  -I $LLVM_PATH/include/ \
  -I $LLVM_PATH/build/include/ \
  -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS"

set -x

g++ -c -g -Wall plugin.cc $CPP_ARGS -fpic && gcc -shared -o indexer.so indexer.o

g++ -fno-rtti -g -Wall indexer.cc $CPP_ARGS \
  -L $LLVM_PATH/build/lib \
  -lclangTooling -lclangFrontend -lclangSerialization \
  -lclangDriver -lclangSema -lclangAnalysis -lclangEdit -lclangAST \
  -lclangParse -lclangSema -lclangLex -lclangBasic \
  -lLLVMBitReader -lLLVMMCParser -lLLVMMC -lLLVMTransformUtils \
  -lLLVMOption -lLLVMCore -lLLVMSupport \
  -ldl -ltinfo -lpthread

