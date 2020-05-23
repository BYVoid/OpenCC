cmake -S. -Bbuild -DCMAKE_INSTALL_PREFIX:PATH=. -DENABLE_GTEST:BOOL=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug --target install
cd build
ctest --verbose -C Debug
