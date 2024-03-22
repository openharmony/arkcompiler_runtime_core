# Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

set(ARKINSPECTOR_ROOTS
    connection/endpoint_base.cpp
    connection/event_loop.cpp
    connection/server_endpoint_base.cpp
    debug_info_cache.cpp
    debuggable_thread.cpp
    error.cpp
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
)
set(ARKINSPECTOR_LINKED_LIBRARIES
    arkbase
    arkdisassembler
    arkfile
    arkruntime
)
set(ARKINSPECTOR_DEFINITIONS
    ""
)

# required for arkcompiler_toolchain compilation
if(PANDA_TARGET_OHOS)
    list(APPEND ARKINSPECTOR_DEFINITIONS -DOHOS_PLATFORM)
endif()

if(PANDA_TOOLING_ASIO)
    list(APPEND ARKINSPECTOR_ROOTS
        connection/asio/asio_server.cpp
        connection/asio/ws_logger.cpp
    )
    set(ARKINSPECTOR_INCLUDES
        ${PANDA_THIRD_PARTY_SOURCES_DIR}/asio/asio/include/
        ${PANDA_THIRD_PARTY_SOURCES_DIR}/websocketpp/
    )
    list(APPEND ARKINSPECTOR_DEFINITIONS
        -DPANDA_TOOLING_ASIO
        -DASIO_NO_TYPEID
        -DASIO_STANDALONE
    )
else()
    list(APPEND ARKINSPECTOR_ROOTS
        connection/ohos_ws/ohos_ws_server_endpoint.cpp
        connection/ohos_ws/ohos_ws_server.cpp
    )
    list(APPEND ARKINSPECTOR_LINKED_LIBRARIES toolchain_websocket)
    set(ARKINSPECTOR_INCLUDES
        ${PANDA_THIRD_PARTY_SOURCES_DIR}/arkcompiler_toolchain/
    )
endif()

add_definitions(${ARKINSPECTOR_DEFINITIONS})

panda_add_library(arkinspector SHARED
    ${ARKINSPECTOR_ROOTS}
)

add_dependencies(arkinspector arkruntime)

include_directories(
    ${PANDA_ROOT}/runtime/include/
)

include_directories(SYSTEM
    ${ARKINSPECTOR_INCLUDES}
)

panda_target_link_libraries(arkinspector
    ${ARKINSPECTOR_LINKED_LIBRARIES}
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
