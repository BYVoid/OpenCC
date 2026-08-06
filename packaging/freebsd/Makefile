PORTNAME=	opencc
DISTVERSIONPREFIX=	ver.
DISTVERSION=	1.4.1
CATEGORIES=	chinese converters

MAINTAINER=	lichray@gmail.com
COMMENT=	Open Chinese Convert library and utilities
WWW=		https://github.com/BYVoid/OpenCC

LICENSE=	APACHE20

USES=		cmake:testing compiler:c++17-lang python:build
USE_GITHUB=	yes
USE_LDCONFIG=	yes
GH_ACCOUNT=	BYVoid
GH_PROJECT=	OpenCC

CMAKE_ARGS=	-DPython3_EXECUTABLE=${PYTHON_CMD}
CMAKE_TESTING_ON=	ENABLE_GTEST

TEST_TARGET=	test

OPTIONS_DEFINE=	JIEBA
OPTIONS_DEFAULT=	JIEBA
OPTIONS_SUB=	yes

JIEBA_DESC=	Jieba segmentation plugin
JIEBA_CMAKE_BOOL=	BUILD_OPENCC_JIEBA_PLUGIN

.include <bsd.port.mk>
