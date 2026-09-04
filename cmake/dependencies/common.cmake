# =================== SSE2NEON ===================
set(SSE2NEON_DIR ${CMAKE_BINARY_DIR}/_deps/sse2neon)
# Only download when missing/empty: the fetch returns 0 bytes in sandboxed
# networks, and re-configure must not clobber a good file.
if(NOT EXISTS "${SSE2NEON_DIR}/sse2neon.h")
  file(
    DOWNLOAD
    "https://raw.githubusercontent.com/DLTcollab/sse2neon/refs/heads/master/sse2neon.h"
    "${SSE2NEON_DIR}/sse2neon.h")
endif()
file(SIZE "${SSE2NEON_DIR}/sse2neon.h" _sse2neon_size)
if(_sse2neon_size EQUAL 0)
  message(FATAL_ERROR "sse2neon.h is empty (network fetch failed); place a valid copy at ${SSE2NEON_DIR}/sse2neon.h and re-configure")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${SSE2NEON_DIR})

# ================== SEMVER ===================
set(SEMVER_DIR ${CMAKE_BINARY_DIR}/_deps/semver)
if(NOT EXISTS "${SEMVER_DIR}/semver.hpp")
  file(
    DOWNLOAD
    "https://raw.githubusercontent.com/Neargye/semver/refs/tags/v1.0.0-rc/include/semver.hpp"
    "${SEMVER_DIR}/semver.hpp")
endif()
file(SIZE "${SEMVER_DIR}/semver.hpp" _semver_size)
if(_semver_size EQUAL 0)
  message(FATAL_ERROR "semver.hpp is empty (network fetch failed); place a valid copy at ${SEMVER_DIR}/semver.hpp and re-configure")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${SEMVER_DIR})

# =================== DRLibs ===================
FetchContent_Declare(
  dr_libs
  GIT_REPOSITORY https://github.com/mackron/dr_libs.git
  GIT_TAG da35f9d6c7374a95353fd1df1d394d44ab66cf01)
FetchContent_MakeAvailable(dr_libs)

target_include_directories(${PROJECT_NAME} PRIVATE ${dr_libs_SOURCE_DIR})

# =================== tomlplusplus ===================
include(FetchContent)
FetchContent_Declare(
  tomlplusplus
  GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
  GIT_TAG v3.4.0)
FetchContent_MakeAvailable(tomlplusplus)
target_link_libraries(${PROJECT_NAME} PRIVATE tomlplusplus::tomlplusplus)

# libultraship
# Removes MPQ/OTR support
set(EXCLUDE_MPQ_SUPPORT TRUE CACHE BOOL "")
set(ENABLE_EXP_AUTO_CONFIGURE_CONTROLLERS ON CACHE BOOL "")
target_compile_definitions(${PROJECT_NAME} PRIVATE EXCLUDE_MPQ_SUPPORT)

target_include_directories(
  ${PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/include
  ${CMAKE_CURRENT_SOURCE_DIR}/libultraship/include/libultraship)

add_subdirectory(libultraship ${CMAKE_BINARY_DIR}/libultraship)
add_dependencies(${PROJECT_NAME} libultraship)
target_link_libraries(${PROJECT_NAME} PRIVATE libultraship)

# Torch
option(USE_STANDALONE "Build as a standalone executable" OFF)
option(BUILD_STORMLIB "Build with StormLib support" OFF)

option(BUILD_SM64 "Build with Super Mario 64 support" OFF)
option(BUILD_MK64 "Build with Mario Kart 64 support" ON)
option(BUILD_SF64 "Build with Star Fox 64 support" OFF)
option(BUILD_FZERO "Build with F-Zero X support" OFF)
option(BUILD_MARIO_ARTIST "Build with Mario Artist support" OFF)
# Companion.cpp uses AudioManager unconditionally, so preserve Torch's ON
# default until its non-NAudio build is fixed upstream.
option(BUILD_NAUDIO "Build with NAudio support" ON)

add_subdirectory(torch)
