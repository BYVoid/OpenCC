PORTNAME=	opencc
DISTVERSIONPREFIX=	ver.
DISTVERSION=	1.4.2
PORTREVISION=	1
CATEGORIES=	chinese converters

MAINTAINER=	lichray@gmail.com
COMMENT=	Open Chinese Convert library and utilities
WWW=		https://github.com/BYVoid/OpenCC

LICENSE=	APACHE20

BUILD_DEPENDS=	rapidjson>=0:devel/rapidjson \
		${LOCALBASE}/include/tclap/CmdLine.h:devel/tclap12
LIB_DEPENDS=	libmarisa.so:devel/marisa-trie
TEST_DEPENDS=	googletest>=0:devel/googletest

USES=		cmake:testing compiler:c++17-lang localbase:ldflags python:build
USE_GITHUB=	yes
GH_ACCOUNT=	BYVoid
GH_PROJECT=	OpenCC
USE_LDCONFIG=	yes

CMAKE_ON=	CMAKE_DISABLE_FIND_PACKAGE_Git \
		USE_SYSTEM_MARISA \
		USE_SYSTEM_RAPIDJSON \
		USE_SYSTEM_TCLAP
CMAKE_TESTING_ON=	ENABLE_GTEST USE_SYSTEM_GTEST

OPTIONS_DEFINE=	JIEBA
OPTIONS_DEFAULT=	JIEBA
OPTIONS_SUB=	yes

JIEBA_DESC=	Jieba segmentation plugin
JIEBA_CMAKE_BOOL=	BUILD_OPENCC_JIEBA_PLUGIN

.include <bsd.port.mk>
