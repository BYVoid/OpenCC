root=$PWD

# build data and share with wasm
cmake . -B build/native \
  -DCMAKE_INSTALL_PREFIX:PATH=$root/build/sysroot/usr
make -C build/native/data install

# build wasm libopencc.a and libmarisa.a
emcmake cmake . -B build/wasm \
  -DBUILD_SHARED_LIBS:BOOL=OFF \
  -DCMAKE_BUILD_TYPE:STRING="Release" \
  -DCMAKE_INSTALL_PREFIX:PATH=/usr
make DESTDIR=$root/build/sysroot -C build/wasm install
cp build/wasm/deps/marisa-0.2.6/libmarisa.a build/sysroot/usr/lib

# build demo.js
em++ -std=c++14 \
  -I build/sysroot/usr/include/opencc \
  --preload-file build/sysroot/usr/share/opencc@/usr/share/opencc \
  -o wasm/demo.js \
  wasm/demo.cpp \
  -L build/sysroot/usr/lib \
  -l opencc -l marisa

# test demo.js
expected="網路"
actual=$(cd wasm; node demo.js)
if [[ $actual != $expected ]]; then
  echo Test failed.
  echo expected: $expected
  echo actual: $actual
  exit 1
else
  echo Test passed.
fi
