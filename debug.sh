mkdir -p debug \
&& cd debug \
&& cmake \
	-D ENABLE_GETTEXT:BOOL=OFF \
	-D BUILD_DOCUMENTATION:BOOL=ON \
	-DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_INSTALL_PREFIX=`pwd`/root \
	.. \
&& make \
&& make install \
&& make test
