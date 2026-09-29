# Helpers to decide whether binaries built for the target architecture can
# be executed on the build host. The dictionary build (data/) and the Jieba
# plugin (plugins/jieba/) run the just-built tools at build time, so they
# need this to skip or redirect tool execution when cross compiling.
#
# Included from the top-level CMakeLists.txt (integrated builds) and from
# plugins/jieba/CMakeLists.txt (standalone plugin builds, with an inline
# fallback when the parent source tree is not available).

function(opencc_normalize_arch output_var input_value)
  string(TOUPPER "${input_value}" _arch)
  if (_arch STREQUAL "AMD64" OR _arch STREQUAL "X86_64")
    set(_arch "X64")
  elseif (_arch STREQUAL "AARCH64")
    set(_arch "ARM64")
  elseif (_arch MATCHES "^I[3-6]86$" OR _arch STREQUAL "X86" OR
          _arch STREQUAL "WIN32")
    set(_arch "X86")
  endif()
  set(${output_var} "${_arch}" PARENT_SCOPE)
endfunction()

function(opencc_can_run_target_on_host output_var host_arch target_arch)
  set(_can_run FALSE)
  if ("${host_arch}" STREQUAL "${target_arch}")
    set(_can_run TRUE)
  elseif ("${host_arch}" STREQUAL "X64" AND "${target_arch}" STREQUAL "X86")
    # 32-bit Windows binaries run on x64 Windows hosts via WOW64.
    set(_can_run TRUE)
  endif()
  set(${output_var} "${_can_run}" PARENT_SCOPE)
endfunction()

# OPENCC_CAN_RUN_TARGET_TOOLS: TRUE when the host can plausibly execute
# binaries built for the target architecture. This covers native builds and
# x86 targets on x64 Windows hosts; emulation such as Rosetta 2 (x64 targets
# on Apple Silicon) is deliberately not detected — set the relevant BUILD_*
# options explicitly when relying on it.
if(DEFINED CMAKE_VS_PLATFORM_NAME AND NOT CMAKE_VS_PLATFORM_NAME STREQUAL "")
  set(_OPENCC_TARGET_ARCH_RAW "${CMAKE_VS_PLATFORM_NAME}")
else()
  set(_OPENCC_TARGET_ARCH_RAW "${CMAKE_SYSTEM_PROCESSOR}")
endif()
opencc_normalize_arch(OPENCC_TARGET_ARCH "${_OPENCC_TARGET_ARCH_RAW}")
opencc_normalize_arch(OPENCC_HOST_ARCH "${CMAKE_HOST_SYSTEM_PROCESSOR}")

set(OPENCC_CAN_RUN_TARGET_TOOLS TRUE)
if(CMAKE_CROSSCOMPILING)
  set(OPENCC_CAN_RUN_TARGET_TOOLS FALSE)
  if(WIN32 AND OPENCC_TARGET_ARCH AND OPENCC_HOST_ARCH)
    opencc_can_run_target_on_host(_opencc_can_run_target
                                  "${OPENCC_HOST_ARCH}"
                                  "${OPENCC_TARGET_ARCH}")
    if(_opencc_can_run_target)
      set(OPENCC_CAN_RUN_TARGET_TOOLS TRUE)
    endif()
  endif()
endif()
