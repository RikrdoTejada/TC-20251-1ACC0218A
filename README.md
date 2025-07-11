# Pasos a seguir para generar el servidor

cmake -S src -B build -G "Unix Makefiles"   -DLLVM_ENABLE_PROJECTS=""   -DCMAKE_INSTALL_PREFIX="/home/lmcs/Apps/llvm"   -DBUILD_SHARED_LIBS=on   -DCMAKE_BUILD_TYPE=Release   -DLLVM_USE_LINKER=lld   -DLLVM_PARALLEL_COMPILE_JOBS=12   -DCMAKE_CXX_COMPILER=clang++   -DCMAKE_C_COMPILER=clang

cmake --build build -j8

./build/ApiGenerator input/pruebita.ApiGenerator

clang++ -std=c++17 -Ilib output/main.cpp lib/mongoose.c -o output/servidor \
`llvm-config --cxxflags --ldflags --system-libs --libs all` -rdynamic

./output/servidor


#### En caso no exista la carpeta data, crearla
