cmake_minimum_required(VERSION 3.11...3.24)


project(
  Tractor
  VERSION 0.1
  DESCRIPTION "My network lib"
  LANGUAGES CXX)


if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_EXTENSIONS OFF)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  include(CTest)
endif()

set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)
if(${CMAKE_VERSION} VERSION_LESS 3.14)
  include(cmake/add_FetchContent_MakeAvailable.cmake)
endif()

find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(src)
add_subdirectory(apps)

if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME )
   AND BUILD_TESTING)
  add_subdirectory(tests)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
