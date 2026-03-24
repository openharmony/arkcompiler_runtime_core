# Copyright (c) 2026 Huawei Device Co., Ltd.
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

project(aptool)

set(APTOOL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(APTOOL_COMMON_INCLUDE_DIRS
    ${PANDA_ROOT}
    ${PANDA_ROOT}/runtime
    ${PANDA_ROOT}/libarkfile
    ${PANDA_ROOT}/assembler
    ${APTOOL_SOURCE_DIR}
)
set(APTOOL_COMMON_LIBRARIES
    arkbase
    arkruntime
    arkfile
    arkassembler
)

panda_add_executable(ark_aptool
    main.cpp
    common/utils.cpp
    common/profile_loader.cpp
    common/abc_metadata.cpp
)

add_subdirectory(dump)
add_subdirectory(merge)
panda_target_sources(ark_aptool PRIVATE
    $<TARGET_OBJECTS:ark_aptool_dump>
    $<TARGET_OBJECTS:ark_aptool_merge>
)

panda_target_link_libraries(ark_aptool
    ${APTOOL_COMMON_LIBRARIES}
)

panda_target_include_directories(ark_aptool
    PUBLIC ${APTOOL_COMMON_INCLUDE_DIRS}
)

# CMake builds always include dump (development environment)
panda_target_compile_options(ark_aptool PUBLIC -DAPTOOL_INCLUDE_DUMP)

panda_add_sanitizers(TARGET ark_aptool SANITIZERS ${PANDA_SANITIZERS_LIST})
