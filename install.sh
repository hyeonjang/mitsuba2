export CC=clang-9
export CXX=clang++-9

cd build
cmake -GNinja ..
ninja

