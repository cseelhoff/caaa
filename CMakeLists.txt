cmake_minimum_required(VERSION 3.10)
project(caaa)

include(FetchContent)

FetchContent_Declare(
  cJSON
  GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
  GIT_TAG        v1.7.14 # Optionally specify a version tag or commit hash here 
)

FetchContent_MakeAvailable(cJSON)

add_executable(caaa src/main.c)
target_link_libraries(caaa PRIVATE cjson)

#target_include_directories(caaa PRIVATE ${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include)
#link_libraries(cJSON)