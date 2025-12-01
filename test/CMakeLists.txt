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

cpmaddpackage("gh:ThrowTheSwitch/Unity@2.6.1")

add_executable(lowlevel-test lowlevel-test.c)
target_link_libraries(lowlevel-test PRIVATE alea unity)
add_test(NAME lowlevel COMMAND lowlevel-test)

add_executable(functionality-test functionality-test.c)
target_link_libraries(functionality-test PRIVATE alea unity)
add_test(NAME functionality COMMAND functionality-test)

# Build dudect only for x86_64
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" AND NOT MSVC)
  cpmaddpackage("gh:oreparaz/dudect#dc269651fb2567e46755cfb2a13d3875592968b5")

  add_executable(timing-test timing-test.c dudect_wrapper.c)
  target_link_libraries(timing-test PRIVATE alea unity m)

  target_include_directories(timing-test PRIVATE ${dudect_SOURCE_DIR}/src)
endif()
