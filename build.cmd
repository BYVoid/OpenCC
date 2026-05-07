cmake -S. -Bbuild -DCMAKE_INSTALL_PREFIX:PATH=.
cmake --build build --config Release --target install
