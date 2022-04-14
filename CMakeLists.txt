# Copyright 2018 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# cmake build file for C++ multiplayer_scene example.
# Assumes protobuf and gRPC have been installed using cmake.
# See cmake_externalproject/CMakeLists.txt for all-in-one cmake build
# that automatically builds all the dependencies before building multiplayer_scene.

# .
cmake_minimum_required(VERSION 3.5.1)

project(MULTIPLAYER_SCENE C CXX)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   set(THREADS_PREFER_PTHREAD_FLAG ON)
   find_package(Threads REQUIRED)
endif()

include(FetchContent)

# glm
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG        0.9.9.8
)
FetchContent_Populate(glm)

# glfw
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG        3.3.2
)
FetchContent_GetProperties(glfw)

# stbimage
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
)
if(NOT stb_POPULATED)
    FetchContent_Populate(stb)
endif()

# visualisations
FetchContent_Declare(
  visualisations
  GIT_REPOSITORY https://github.com/billguastalla/visualisations.git
  GIT_TAG        master
)
FetchContent_GetProperties(visualisations)
if(NOT visualisations_POPULATED)
    FetchContent_Populate(visualisations)
endif()

# from https://jonathanhamberg.com/post/2019-01-27-cmake-dependencies-fetch-content/
if(NOT glfw_POPULATED)
    FetchContent_Populate(glfw)

    set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
    set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
    set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
    set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")

    add_subdirectory(${glfw_SOURCE_DIR} ${glfw_BINARY_DIR})
endif()

# glad
FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
)
# from https://jonathanhamberg.com/post/2019-01-27-cmake-dependencies-fetch-content/
FetchContent_GetProperties(glad)
if(NOT glad_POPULATED)
    FetchContent_Populate(glad)
    set(GLAD_PROFILE "core" CACHE STRING "OpenGL profile")
    set(GLAD_API "gl=3.3" CACHE STRING "API type/version pairs, like \"gl=3.2,gles=\", no version means latest")
    set(GLAD_GENERATOR "c" CACHE STRING "Language to generate the binding for")
    add_subdirectory(${glad_SOURCE_DIR} ${glad_BINARY_DIR})
endif()

find_package(Boost REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${BOOST_LIBRARYDIR})
endif()

add_subdirectory(src) 
 
  