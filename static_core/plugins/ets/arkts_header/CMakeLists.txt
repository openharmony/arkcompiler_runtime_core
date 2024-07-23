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

cmake_minimum_required(VERSION 3.10)

project(arkts_header)

set(HEADER_CREATOR_SOURCES
    arkts_header.cpp
    header_writer.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/runtime/napi/ets_mangle.cpp
)

panda_add_executable(arkts_header ${HEADER_CREATOR_SOURCES})

panda_target_link_libraries(arkts_header arkbase arkfile)

panda_target_include_directories(arkts_header
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/generated
    PUBLIC ${PANDA_ETS_PLUGIN_SOURCE}/runtime/
    PUBLIC ${PANDA_BINARY_ROOT}/
)

panda_add_sanitizers(TARGET arkts_header SANITIZERS ${PANDA_SANITIZERS_LIST})
