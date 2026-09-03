cmake_minimum_required(VERSION 3.16)
project(mk64-scattershot)
set(CMAKE_CXX_STANDARD 20)
add_executable(mk64-scattershot
  main.cpp
  MK64State.cpp
  HeadlessStubs.cpp
  Utils.cpp
  InputGen.cpp
  Evaluator.cpp
  Scattershot.cpp
  ThreadState.cpp
  ReplayWriter.cpp
)
target_include_directories(mk64-scattershot PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/libultraship/include
  ${CMAKE_SOURCE_DIR}/libultraship/include/libultraship
)
# headless tool needs threadpool/imgui/sdl headers that libultraship FetchContent provides
foreach(_dep threadpool imgui sdl2)
  foreach(_cand 
    ${CMAKE_BINARY_DIR}/_deps/${_dep}-src/include
    ${CMAKE_BINARY_DIR}/../_deps/${_dep}-src/include
    ${CMAKE_BINARY_DIR}/../../_deps/${_dep}-src/include
    ${CMAKE_SOURCE_DIR}/build-headless/_deps/${_dep}-src/include
    ${CMAKE_BINARY_DIR}/_deps/${_dep}-src/include/SDL2
  )
    if(EXISTS ${_cand}/BS_thread_pool.hpp OR EXISTS ${_cand}/imgui.h OR EXISTS ${_cand}/SDL.h)
      target_include_directories(mk64-scattershot PRIVATE ${_cand})
    endif()
  endforeach()
  if(EXISTS ${CMAKE_BINARY_DIR}/_deps/${_dep}-src/include)
    target_include_directories(mk64-scattershot PRIVATE ${CMAKE_BINARY_DIR}/_deps/${_dep}-src/include)
  endif()
  if(EXISTS ${CMAKE_BINARY_DIR}/../_deps/${_dep}-src/include)
    target_include_directories(mk64-scattershot PRIVATE ${CMAKE_BINARY_DIR}/../_deps/${_dep}-src/include)
  endif()
  if(EXISTS ${CMAKE_BINARY_DIR}/../../_deps/${_dep}-src/include)
    target_include_directories(mk64-scattershot PRIVATE ${CMAKE_BINARY_DIR}/../../_deps/${_dep}-src/include)
  endif()
endforeach()
if(EXISTS ${CMAKE_BINARY_DIR}/_deps/imgui-src)
  target_include_directories(mk64-scattershot PRIVATE ${CMAKE_BINARY_DIR}/_deps/imgui-src)
endif()
if(EXISTS ${CMAKE_BINARY_DIR}/../_deps/imgui-src)
  target_include_directories(mk64-scattershot PRIVATE ${CMAKE_BINARY_DIR}/../_deps/imgui-src)
endif()
if(EXISTS ${CMAKE_BINARY_DIR}/../../_deps/imgui-src)
  target_include_directories(mk64-scattershot PRIVATE ${CMAKE_BINARY_DIR}/../../_deps/imgui-src)
endif()
# system SDL (Docker apt libsdl2-dev) at /usr/include/SDL2
if(EXISTS /usr/include/SDL2/SDL.h)
  target_include_directories(mk64-scattershot PRIVATE /usr/include/SDL2)
endif()
if(EXISTS /usr/include/SDL2)
  target_include_directories(mk64-scattershot PRIVATE /usr/include/SDL2)
endif()
if(TARGET SpaghettifyHeadless)
  target_link_libraries(mk64-scattershot PRIVATE SpaghettifyHeadless)
  target_compile_definitions(mk64-scattershot PRIVATE HEADLESS=1 F3DEX_GBI=1 VERSION_US=1 _LANGUAGE_C NDEBUG)
endif()
if(SCATTERSHOT_HEADLESS)
  target_compile_definitions(mk64-scattershot PRIVATE HEADLESS=1 F3DEX_GBI=1 VERSION_US=1 _LANGUAGE_C NDEBUG)
endif()
# Link all deps that libultraship/Spaghettify pull (header-only inline needs link)
# Prefer targets if available, fallback to system libs
if(TARGET libultraship)
  target_link_libraries(mk64-scattershot PRIVATE libultraship)
endif()
if(TARGET fmt)
  target_link_libraries(mk64-scattershot PRIVATE fmt)
endif()
if(TARGET fmt::fmt)
  target_link_libraries(mk64-scattershot PRIVATE fmt::fmt)
endif()
if(TARGET spdlog)
  target_link_libraries(mk64-scattershot PRIVATE spdlog)
endif()
if(TARGET spdlog::spdlog)
  target_link_libraries(mk64-scattershot PRIVATE spdlog::spdlog)
endif()
if(TARGET nlohmann_json)
  target_link_libraries(mk64-scattershot PRIVATE nlohmann_json)
endif()
if(TARGET nlohmann_json::nlohmann_json)
  target_link_libraries(mk64-scattershot PRIVATE nlohmann_json::nlohmann_json)
endif()
if(TARGET tinyxml2)
  target_link_libraries(mk64-scattershot PRIVATE tinyxml2)
endif()
if(TARGET tinyxml2::tinyxml2)
  target_link_libraries(mk64-scattershot PRIVATE tinyxml2::tinyxml2)
endif()
if(TARGET zip)
  target_link_libraries(mk64-scattershot PRIVATE zip)
endif()
if(TARGET libzip::zip)
  target_link_libraries(mk64-scattershot PRIVATE libzip::zip)
endif()
# System fallback via find_package / pkg-config
find_package(fmt QUIET)
if(fmt_FOUND)
  target_link_libraries(mk64-scattershot PRIVATE fmt::fmt)
endif()
find_package(spdlog QUIET)
if(spdlog_FOUND)
  target_link_libraries(mk64-scattershot PRIVATE spdlog::spdlog)
endif()
find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(LIBZIP QUIET libzip)
  if(LIBZIP_FOUND)
    target_include_directories(mk64-scattershot PRIVATE ${LIBZIP_INCLUDE_DIRS})
    target_link_directories(mk64-scattershot PRIVATE ${LIBZIP_LIBRARY_DIRS})
    target_link_libraries(mk64-scattershot PRIVATE ${LIBZIP_LIBRARIES})
  endif()
  pkg_check_modules(TINYXML2 QUIET tinyxml2)
  if(TINYXML2_FOUND)
    target_link_libraries(mk64-scattershot PRIVATE ${TINYXML2_LIBRARIES})
  endif()
endif()
# Always link these system libs if present (Docker has them)
find_library(ZIP_LIB zip)
if(ZIP_LIB)
  target_link_libraries(mk64-scattershot PRIVATE ${ZIP_LIB})
endif()
find_library(TINYXML2_LIB tinyxml2)
if(TINYXML2_LIB)
  target_link_libraries(mk64-scattershot PRIVATE ${TINYXML2_LIB})
endif()
find_library(SDL2_LIB SDL2)
if(SDL2_LIB)
  target_link_libraries(mk64-scattershot PRIVATE ${SDL2_LIB})
endif()
# Threads
find_package(Threads QUIET)
if(Threads_FOUND)
  target_link_libraries(mk64-scattershot PRIVATE Threads::Threads)
endif()
# GC unused functions in monolithic code_80005FD0.c.o so only update_player path is kept
target_compile_options(mk64-scattershot PRIVATE -ffunction-sections -fdata-sections)
target_link_options(mk64-scattershot PRIVATE -Wl,--gc-sections)
