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
)
if(TARGET SpaghettifyHeadless)
  target_link_libraries(mk64-scattershot PRIVATE SpaghettifyHeadless)
endif()
