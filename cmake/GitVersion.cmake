#
# GitVersion.cmake — Derive project version from Git tags.
#
# Sets in parent scope:
#   OPENCC_VERSION           Full version string (e.g. "1.2.1" or "1.2.1.dev4+gabc1234")
#   OPENCC_VERSION_MAJOR     Major component
#   OPENCC_VERSION_MINOR     Minor component
#   OPENCC_VERSION_REVISION  Patch/revision component
#   OPENCC_VERSION_SUFFIX    Everything after M.m.p (e.g. ".dev4+gabc1234.dirty") or ""
#   OPENCC_VERSION_IS_RELEASE  TRUE if this is an exact-tag release build
#
# Falls back to hardcoded default when Git is unavailable.
#

set(_OPENCC_FALLBACK_MAJOR 1)
set(_OPENCC_FALLBACK_MINOR 3)
set(_OPENCC_FALLBACK_REVISION 1)

find_package(Git QUIET)

set(_git_version_computed FALSE)

if(GIT_FOUND)
  # Try git describe --tags --long --always
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" describe --tags --long --always
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    OUTPUT_VARIABLE _git_raw
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _git_result
  )

  if(_git_result EQUAL 0 AND _git_raw)
    # Detect dirty working tree
    set(_dirty_suffix "")
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" diff --quiet
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
      RESULT_VARIABLE _diff_result
    )
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" diff --cached --quiet
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
      RESULT_VARIABLE _diff_cached_result
    )
    if(NOT _diff_result EQUAL 0 OR NOT _diff_cached_result EQUAL 0)
      set(_dirty_suffix ".dirty")
    endif()

    # Case 1: Exact tag — v1.2.3-0-g<sha> or ver.1.2.3-0-g<sha>
    # NOTE: version tag patterns here must be kept in sync with scripts/compute-version.sh
    string(REGEX MATCH "^(v|ver\\.)([0-9]+)\\.([0-9]+)\\.([0-9]+)-0-g[0-9a-f]+$" _match_tag "${_git_raw}")
    if(_match_tag)
      set(OPENCC_VERSION_MAJOR "${CMAKE_MATCH_2}")
      set(OPENCC_VERSION_MINOR "${CMAKE_MATCH_3}")
      set(OPENCC_VERSION_REVISION "${CMAKE_MATCH_4}")
      set(OPENCC_VERSION_SUFFIX "${_dirty_suffix}")
      set(OPENCC_VERSION_IS_RELEASE TRUE)
      set(_git_version_computed TRUE)
    endif()

    # Case 2: Dev build — v1.2.3-N-g<sha> or ver.1.2.3-N-g<sha>  (N > 0)
    if(NOT _git_version_computed)
      string(REGEX MATCH "^(v|ver\\.)([0-9]+)\\.([0-9]+)\\.([0-9]+)-([0-9]+)-g([0-9a-f]+)$" _match_dev "${_git_raw}")
      if(_match_dev)
        set(OPENCC_VERSION_MAJOR "${CMAKE_MATCH_2}")
        set(OPENCC_VERSION_MINOR "${CMAKE_MATCH_3}")
        set(OPENCC_VERSION_REVISION "${CMAKE_MATCH_4}")
        set(OPENCC_VERSION_SUFFIX ".dev${CMAKE_MATCH_5}+g${CMAKE_MATCH_6}${_dirty_suffix}")
        set(OPENCC_VERSION_IS_RELEASE FALSE)
        set(_git_version_computed TRUE)
      endif()
    endif()

    # Case 3: No tag — raw SHA only
    if(NOT _git_version_computed)
      set(OPENCC_VERSION_MAJOR "${_OPENCC_FALLBACK_MAJOR}")
      set(OPENCC_VERSION_MINOR "${_OPENCC_FALLBACK_MINOR}")
      set(OPENCC_VERSION_REVISION "${_OPENCC_FALLBACK_REVISION}")
      set(OPENCC_VERSION_SUFFIX "+g${_git_raw}${_dirty_suffix}")
      set(OPENCC_VERSION_IS_RELEASE FALSE)
      set(_git_version_computed TRUE)
    endif()
  endif()
endif()

# Fallback: no Git or git describe failed
if(NOT _git_version_computed)
  message(STATUS "Git not available — using fallback version ${_OPENCC_FALLBACK_MAJOR}.${_OPENCC_FALLBACK_MINOR}.${_OPENCC_FALLBACK_REVISION}")
  set(OPENCC_VERSION_MAJOR "${_OPENCC_FALLBACK_MAJOR}")
  set(OPENCC_VERSION_MINOR "${_OPENCC_FALLBACK_MINOR}")
  set(OPENCC_VERSION_REVISION "${_OPENCC_FALLBACK_REVISION}")
  set(OPENCC_VERSION_SUFFIX "")
  set(OPENCC_VERSION_IS_RELEASE FALSE)
endif()

# Compose full version string
set(OPENCC_VERSION "${OPENCC_VERSION_MAJOR}.${OPENCC_VERSION_MINOR}.${OPENCC_VERSION_REVISION}${OPENCC_VERSION_SUFFIX}")

message(STATUS "OpenCC version: ${OPENCC_VERSION}")
