# PCRE2 needs some settings.
SET(PCRE2_WIDTH ${WCHAR_T_BITS})
SET(PCRE2_BUILD_PCRE2_8 OFF CACHE BOOL "Build 8bit PCRE2 library")
SET(PCRE2_BUILD_PCRE2_${PCRE2_WIDTH} ON CACHE BOOL "Build ${PCRE2_WIDTH}bit PCRE2 library")
SET(PCRE2_SHOW_REPORT OFF CACHE BOOL "Show the final configuration report")
SET(PCRE2_BUILD_PCRE2GREP OFF CACHE BOOL "Build pcre2grep")


SET(PCRE2_MIN_VERSION 10.21)
FIND_LIBRARY(PCRE2_LIB pcre2-${PCRE2_WIDTH})
FIND_PATH(PCRE2_INCLUDE_DIR pcre2.h)
IF (PCRE2_LIB AND PCRE2_INCLUDE_DIR)
  MESSAGE(STATUS "Found system PCRE2 library ${PCRE2_INCLUDE_DIR}")
ELSE()
  MESSAGE(STATUS "Using bundled PCRE2 library")
  ADD_SUBDIRECTORY(pcre2-10.22 EXCLUDE_FROM_ALL)
  SET(PCRE2_INCLUDE_DIR ${CMAKE_BINARY_DIR}/pcre2-10.22/)
  SET(PCRE2_LIB pcre2-${PCRE2_WIDTH})
endif(PCRE2_LIB AND PCRE2_INCLUDE_DIR)
INCLUDE_DIRECTORIES(${PCRE2_INCLUDE_DIR})
