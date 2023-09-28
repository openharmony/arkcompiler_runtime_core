# Copyright (c) 2022 Huawei Device Co., Ltd.
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

project(arkinspector)

add_compile_options(-fexceptions -Wno-invalid-offsetof)

add_definitions(-DASIO_NO_TYPEID -DASIO_STANDALONE)

panda_add_library(arkinspector SHARED
    asio_server.cpp
    debug_info_cache.cpp
    debuggable_thread.cpp
    endpoint.cpp
    error.cpp
    event_loop.cpp
    init.cpp
    inspector.cpp
    inspector_server.cpp
    object_repository.cpp
    session_manager.cpp
    source_manager.cpp
    thread_state.cpp
    types/location.cpp
    types/remote_object.cpp
    types/scope.cpp
    ws_logger.cpp
)

add_dependencies(arkinspector arkruntime)

include_directories(
    ${PANDA_ROOT}/runtime/include/
)

include_directories(SYSTEM
    ${PANDA_THIRD_PARTY_SOURCES_DIR}/asio/asio/include/
    ${PANDA_THIRD_PARTY_SOURCES_DIR}/websocketpp/
)

panda_target_link_libraries(arkinspector
    arkbase
    arkdisassembler
    arkfile
    arkruntime
)

panda_add_sanitizers(TARGET arkinspector SANITIZERS ${PANDA_SANITIZERS_LIST})

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0.0)
    panda_target_compile_options(arkinspector PUBLIC "-Wno-null-pointer-subtraction")
endif()

panda_target_compile_options(arkinspector PUBLIC "-Wno-deprecated-declarations")

panda_add_gtest(
    NAME arkinspector_tests
    NO_CORES
    SOURCES
        tests/source_manager.cpp
        tests/session_manager.cpp
        tests/debug_info_cache.cpp
        tests/thread_state.cpp
        tests/inspector_server.cpp
        tests/object_repository.cpp
    INCLUDE_DIRS
        ${PANDA_ROOT}/runtime/tooling/inspector 
    LIBRARIES
        arkassembler
        arkinspector
        arkruntime
        gmock
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
)
