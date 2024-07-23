# Copyright (c) 2021-2024 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.3.2 FATAL_ERROR)
project(linker CXX)

set(SOURCES
    linker.cpp
    linker_context.cpp
    linker_context_misc.cpp
    linker_code_parser_context.cpp
)

panda_add_library(arklinker ${PANDA_DEFAULT_LIB_TYPE} ${SOURCES})

panda_target_compile_options(arklinker PUBLIC -Wno-invalid-offsetof)

add_dependencies(arklinker
    arkfile
    arkbase
)

panda_target_link_libraries(arklinker PUBLIC
    arkbase
)

panda_target_link_libraries(arklinker PRIVATE
    arkfile
)

panda_target_include_directories(arklinker PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}/libpandabase
    ${PANDA_ROOT}
    ${CMAKE_SOURCE_DIR}/libpandafile
)

panda_add_executable(ark_link link.cpp)

panda_target_compile_options(ark_link PUBLIC -Wno-invalid-offsetof)

panda_gen_options(TARGET ark_link YAML_FILE options.yaml GENERATED_HEADER link_options.h)

panda_add_sanitizers(TARGET arklinker SANITIZERS ${PANDA_SANITIZERS_LIST})
panda_add_sanitizers(TARGET ark_link SANITIZERS ${PANDA_SANITIZERS_LIST})

add_dependencies(ark_link arklinker)
panda_target_link_libraries(ark_link arklinker arkbase)

panda_target_include_directories(ark_link PUBLIC
    ${PANDA_ROOT}
)

include_directories(${PANDA_ROOT}/libpandabase/)
include_directories(${CMAKE_BINARY_DIR}/libpandafile/include/)

if (PANDA_WITH_TESTS)
    add_subdirectory(tests)
endif()
