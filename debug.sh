mkdir --parents debug \
&& cd debug

cmake \
	-D ENABLE_GETTEXT:BOOL=ON \
	-DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_INSTALL_PREFIX=`pwd`/root \
	.. \
&& make \
&& make install \
&& make test
