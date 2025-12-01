# ~~~
# Copyright 2025 CryptoLab, Inc.
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
# ~~~

cmake_minimum_required(VERSION 3.21)
project(
  alea
  VERSION 0.1.5
  LANGUAGES C
  DESCRIPTION "CryptoLab's official pseudo-random number generator")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

option(BUILD_SHARED_LIBS "Build a shared library instead of a static one." OFF)
option(ALEA_BUILD_TEST "Build the test suite." ON)
option(ALEA_BUILD_DOXYGEN "Build the documentation with Doxygen." OFF)
option(ALEA_INSTALL "Install the alea library and headers." ON)

include(cmake/warnings.cmake)
include(cmake/CPM.cmake)

add_library(alea)
target_sources(alea PRIVATE include/alea/alea.h include/alea/algorithms.h
                            src/alea.c src/alea-internal.h src/alea-hkdf.c)
set_my_project_warnings(alea)
target_compile_definitions(alea PUBLIC ALEA_EXPORTS)
target_link_libraries(alea PRIVATE ${CRYPTO_LIB_NAME} m)
target_include_directories(
  alea PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
              $<INSTALL_INTERFACE:include>)

set(CRYPTO_LIB_NAME "")
set(CRYPTO_LIB_COMPILE_DEFINITION "ALEA_BUILTIN")
target_sources(alea PRIVATE src/fips202.h src/fips202.c src/alea-builtin.h
                            src/alea-builtin.c)

target_compile_definitions(alea PRIVATE ${CRYPTO_LIB_COMPILE_DEFINITION})

if(ALEA_BUILD_TEST)
  enable_testing()
  add_subdirectory(test/)
endif()

# Build the documentation
if(ALEA_BUILD_DOXYGEN)
  find_package(Doxygen)
  if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/alea_doc")
    set(DOXYGEN_PROJECT_NAME "Alea")
    set(DOXYGEN_PROJECT_NUMBER "${PROJECT_VERSION}")
    set(DOXYGEN_PROJECT_BRIEF
        "CryptoLab's official pseudo-random number generator")
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_GENERATE_LATEX NO)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_USE_MDFILE_AS_MAINPAGE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

    doxygen_add_docs(
      alea_doc ${CMAKE_CURRENT_SOURCE_DIR}/include/alea/alea.h
      ${CMAKE_CURRENT_SOURCE_DIR}/include/alea/algorithms.h
      ${CMAKE_CURRENT_SOURCE_DIR}/README.md ALL
      COMMENT "Generate documentation with Doxygen")
  else()
    message(STATUS "Doxygen not found, documentation will not be generated.")
  endif()
endif()

# Install the library
if(ALEA_INSTALL)
  install(
    TARGETS alea
    EXPORT aleaTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    INCLUDES
    DESTINATION include)

  # Install header files
  install(DIRECTORY include/ DESTINATION include)

  # Export target for use with find_package
  install(
    EXPORT aleaTargets
    FILE aleaTargets.cmake
    NAMESPACE alea::
    DESTINATION lib/cmake/alea)

  include(CMakePackageConfigHelpers)

  configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/aleaConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/aleaConfig.cmake
    INSTALL_DESTINATION lib/cmake/alea
    PATH_VARS CMAKE_INSTALL_PREFIX
  )

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/aleaConfig.cmake
    DESTINATION lib/cmake/alea
  )
endif()
