mkdir -p release \
&& cd release \
&& cmake \
        -D ENABLE_GETTEXT:BOOL=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        .. \
&& make \
&& make test \
&& make package_source
