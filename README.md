

cmake -S src -B build -G "Unix Makefiles"   -DLLVM_ENABLE_PROJECTS=""   -DCMAKE_INSTALL_PREFIX="/home/lmcs/Apps/llvm"   -DBUILD_SHARED_LIBS=on   -DCMAKE_BUILD_TYPE=Release   -DLLVM_USE_LINKER=lld   -DLLVM_PARALLEL_COMPILE_JOBS=12   -DCMAKE_CXX_COMPILER=clang++   -DCMAKE_C_COMPILER=clang

cmake --build build -j8

#Crear carpeta lib e instalar  
curl -O https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.c
curl -O https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.h

./build/ApiGenerator input/pruebita.ApiGenerator

g++ -Ilib output/main.cpp lib/mongoose.c -o output/servidor

./output/servidor