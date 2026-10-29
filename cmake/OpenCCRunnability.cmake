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

# opencc_host_tool_env: the command prefix that sets up the environment for
# running a host-built tool — "" on a Windows host (the installed layout
# keeps the DLLs next to the executable), otherwise `cmake -E env` plus
# LD_LIBRARY_PATH/DYLD_LIBRARY_PATH pointing at the lib directory of the
# installation the tool comes from (two levels above the executable), for
# tools installed to a non-default prefix that carry no rpath. The env
# variable must be chosen for the HOST platform, not the target: the tool
# runs on the host (e.g. building for a Linux target from macOS still needs
# DYLD_LIBRARY_PATH for the host opencc_dict). Used to wrap whole build-time
# invocations whose tool path is passed on as an argument (the ST phrase
# generator's --opencc): a multi-token command cannot be carried through
# such an argument, but the spawned tool inherits the environment.
function(opencc_host_tool_env output_var tool_path)
  if(CMAKE_HOST_WIN32)
    set(${output_var} "" PARENT_SCOPE)
  else()
    get_filename_component(_opencc_tool_bindir "${tool_path}" DIRECTORY)
    get_filename_component(_opencc_tool_prefix "${_opencc_tool_bindir}" DIRECTORY)
    if(CMAKE_HOST_APPLE)
      set(_opencc_tool_libpath
          "DYLD_LIBRARY_PATH=${_opencc_tool_prefix}/lib:$ENV{DYLD_LIBRARY_PATH}")
    else()
      set(_opencc_tool_libpath
          "LD_LIBRARY_PATH=${_opencc_tool_prefix}/lib:$ENV{LD_LIBRARY_PATH}")
    endif()
    set(${output_var} ${CMAKE_COMMAND} -E env "${_opencc_tool_libpath}" PARENT_SCOPE)
  endif()
endfunction()

# opencc_host_tool_command: a host-built tool executable wrapped into a
# ready-to-run command list (the env prefix above plus the tool itself).
function(opencc_host_tool_command output_var tool_path)
  opencc_host_tool_env(_opencc_tool_env "${tool_path}")
  set(${output_var} ${_opencc_tool_env} "${tool_path}" PARENT_SCOPE)
endfunction()

# OPENCC_CAN_RUN_TARGET_TOOLS: TRUE when the host can plausibly execute
# binaries built for the target architecture. This covers native builds,
# x86 targets on x64 Windows hosts (including the Visual Studio generator,
# which does not set CMAKE_CROSSCOMPILING for -A platforms), and cross
# builds with CMAKE_CROSSCOMPILING_EMULATOR (e.g. qemu); emulation such as
# Rosetta 2 (x64 targets on Apple Silicon) is deliberately not detected —
# set OPENCC_BUILD_DATA explicitly when relying on it.
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
endif()
# The host/target architecture comparison must stay outside the
# CMAKE_CROSSCOMPILING check: the Visual Studio generator builds -A ARM64
# (or Win32) without setting CMAKE_CROSSCOMPILING, so the architectures are
# the only signal there. A mismatched architecture always downgrades to
# FALSE; a runnable one upgrades back to TRUE only on a Windows host, where
# the comparison means WOW64. Cross builds from other hosts to Windows
# (e.g. mingw triplets on Linux/macOS) produce PE binaries the host cannot
# execute even when the architectures match.
if(WIN32 AND OPENCC_TARGET_ARCH AND OPENCC_HOST_ARCH)
  opencc_can_run_target_on_host(_opencc_can_run_target
                                "${OPENCC_HOST_ARCH}"
                                "${OPENCC_TARGET_ARCH}")
  if(NOT _opencc_can_run_target)
    set(OPENCC_CAN_RUN_TARGET_TOOLS FALSE)
  elseif(CMAKE_HOST_WIN32)
    set(OPENCC_CAN_RUN_TARGET_TOOLS TRUE)
  endif()
endif()
# A cross-compiling emulator (e.g. qemu, set via CMAKE_CROSSCOMPILING_EMULATOR)
# lets the host execute target binaries as well.
if(CMAKE_CROSSCOMPILING_EMULATOR)
  set(OPENCC_CAN_RUN_TARGET_TOOLS TRUE)
endif()
