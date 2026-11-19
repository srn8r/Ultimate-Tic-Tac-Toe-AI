cmake_minimum_required(VERSION 3.16)
project(UltimateTicTacToeAI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Core engine: rules + AI, no external dependency.
add_library(uttt src/board.cpp src/ai.cpp)
target_include_directories(uttt PUBLIC include)

# Terminal game and benchmark.
add_executable(uttt-play apps/play.cpp)
target_link_libraries(uttt-play PRIVATE uttt)

# Tests.
include(CTest)
if(BUILD_TESTING)
  add_executable(uttt-tests tests/tests.cpp)
  target_link_libraries(uttt-tests PRIVATE uttt)
  add_test(NAME uttt-tests COMMAND uttt-tests)
endif()

# Optional: school arena build (Windows / MinGW). Point UTTT_COURSE_DIR to the folder
# that contains libUTTTLib.a and the Allegro 5.2 DLLs provided with the course.
set(UTTT_COURSE_DIR "" CACHE PATH "Folder with libUTTTLib.a and the Allegro DLLs")
if(UTTT_COURSE_DIR)
  add_executable(uttt-arena arena/main.cpp)
  target_link_libraries(uttt-arena PRIVATE
    uttt
    ${UTTT_COURSE_DIR}/libUTTTLib.a
    ${UTTT_COURSE_DIR}/allegro_font-5.2.dll
    ${UTTT_COURSE_DIR}/allegro_ttf-5.2.dll
    ${UTTT_COURSE_DIR}/allegro_image-5.2.dll
    ${UTTT_COURSE_DIR}/allegro_primitives-5.2.dll
    ${UTTT_COURSE_DIR}/allegro-5.2.dll)
endif()
