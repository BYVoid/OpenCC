cmake -S. -Bbuild -DCMAKE_INSTALL_PREFIX:PATH=. -DENABLE_GTEST:BOOL=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug --target install
cd build
ctest --verbose -C Debug > ctest.log 2>&1
set TEST_ERROR=%ERRORLEVEL%
type ctest.log
exit /b %TEST_ERROR%
