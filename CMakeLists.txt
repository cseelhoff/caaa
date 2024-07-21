cmake_minimum_required(VERSION 3.10)
project(caaa VERSION 0.1.0 LANGUAGES C)

include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB SOURCES "src/*.c")
add_executable(caaa ${SOURCES})
target_include_directories(caaa PRIVATE /workspaces/caaa/include)

include(FetchContent)
FetchContent_Declare(
  cJSON
  GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
  GIT_TAG        v1.7.14
)
FetchContent_MakeAvailable(cJSON)
target_link_libraries(caaa PRIVATE cjson)
target_include_directories(caaa PRIVATE "/workspaces/caaa/out/build/mypreset/_deps/cjson-src")
