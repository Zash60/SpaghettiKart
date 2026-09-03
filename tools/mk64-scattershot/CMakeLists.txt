cmake_minimum_required(VERSION 3.16)
project(mk64-scattershot)
set(CMAKE_CXX_STANDARD 20)
add_executable(mk64-scattershot
  main.cpp
  MK64State.cpp
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
    if(EXISTS ${_cand})
      # also add generic existence (sdl2 may be sdl2-src/include)
      # use conditional above to avoid polluting
    endif()
  endforeach()
  if(EXISTS ${CMAKE_BINARY_DIR}/_deps/${_dep}-src/include)
    target_include_directories(mk64-scattershot PRIVATE ${CMAKE_BINARY_DIR}/_deps/${_dep}-src/include)
  endif()
  if(EXISTS ${CMAKE_BINARY_DIR}/../_deps/${_dep}-src/include)
    target_include_directories(mk64-scattershot PRIVATE ${CMAKE_BINARY_DIR}/../_deps/${_dep}-src/include)
  endif()
endforeach()
if(EXISTS ${CMAKE_BINARY_DIR}/_deps/imgui-src)
  target_include_directories(mk64-scattershot PRIVATE ${CMAKE_BINARY_DIR}/_deps/imgui-src)
endif()
if(EXISTS ${CMAKE_BINARY_DIR}/../_deps/imgui-src)
  target_include_directories(mk64-scattershot PRIVATE ${CMAKE_BINARY_DIR}/../_deps/imgui-src)
endif()
if(TARGET SpaghettifyHeadless)
  target_link_libraries(mk64-scattershot PRIVATE SpaghettifyHeadless)
  target_compile_definitions(mk64-scattershot PRIVATE HEADLESS=1 F3DEX_GBI=1 VERSION_US=1 _LANGUAGE_C NDEBUG)
endif()
# standalone mock build also needs no HEADLESS
if(SCATTERSHOT_HEADLESS)
  target_compile_definitions(mk64-scattershot PRIVATE HEADLESS=1 F3DEX_GBI=1 VERSION_US=1 _LANGUAGE_C NDEBUG)
endif()
