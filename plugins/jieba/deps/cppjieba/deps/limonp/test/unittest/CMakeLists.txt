
if (MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

ADD_EXECUTABLE(test.run 
	gtest_main.cpp 
	TLogging.cpp  
	TArgvContext.cpp  
	TConfig.cpp
	TStringUtil.cpp 
	TStdExtension.cpp 
	TLocalVector.cpp 
	TClosure.cpp 
	TColorPrint.cpp
)
TARGET_LINK_LIBRARIES(test.run gtest)

#include(GoogleTest)
#gtest_discover_tests(test.run)
