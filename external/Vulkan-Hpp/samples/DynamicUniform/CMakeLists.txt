# Copyright(c) 2019, NVIDIA CORPORATION. All rights reserved.
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

cmake_minimum_required(VERSION 3.2)

project(DynamicUniform)

set(HEADERS
  ../utils/geometries.hpp
  ../utils/math.hpp
  ../utils/shaders.hpp
  ../utils/utils.hpp
)

set(SOURCES
  DynamicUniform.cpp
  ../utils/math.cpp
  ../utils/shaders.cpp
  ../utils/utils.cpp
  ../../glslang/StandAlone/ResourceLimits.cpp
)

source_group(headers FILES ${HEADERS})
source_group(sources FILES ${SOURCES})

add_executable(DynamicUniform
  ${HEADERS}
  ${SOURCES}
)

set_target_properties(DynamicUniform PROPERTIES FOLDER "Samples")
target_include_directories(DynamicUniform PUBLIC ${CMAKE_SOURCE_DIR}/glslang)
target_link_libraries(DynamicUniform PUBLIC glslang SPIRV "${Vulkan_LIBRARIES}")
