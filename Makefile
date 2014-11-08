#
# Open Chinese Convert
#
# Copyright 2010-2014 BYVoid <byvoid@byvoid.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

.PHONY: build clean node test xcode-build

build:
	mkdir -p build/rel
	(cd build/rel; cmake \
	-DBUILD_DOCUMENTATION:BOOL=ON \
	-DCMAKE_BUILD_TYPE=Release \
	-DCMAKE_INSTALL_PREFIX=/usr \
	../..)
	make -C build/rel

package: build
	make -C build/rel package_source

test:
	mkdir -p build/dbg/root
	(cd build/dbg; cmake \
	-DBUILD_DOCUMENTATION:BOOL=OFF \
	-DCMAKE_BUILD_TYPE=Debug \
	-DCMAKE_INSTALL_PREFIX=`pwd`/root \
	../..)
	make -C build/dbg
	make -C build/dbg test
	make -C build/dbg install

node:
	node-gyp configure
	node-gyp build

node-test: node
	npm test

xcode-build:
	mkdir -p xcode
	(cd xcode; cmake \
	-G "Xcode" \
	-DBUILD_DOCUMENTATION:BOOL=OFF \
	..; \
	xcodebuild build)

test-all: test node-test

clean:
	rm -rf build xcode

install: build
	make -C build/rel install

dist: release
	make -C build/rel package_source
