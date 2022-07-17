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

add_library(arkinspector SHARED
    asio_server.cpp
    breakpoint_hooks.cpp
    endpoint.cpp
    error.cpp
    event_loop.cpp
    init.cpp
    inspector.cpp
    inspector_hooks.cpp
    source_file.cpp
    source_manager.cpp
    step_hooks.cpp
    ws_logger.cpp
)

add_dependencies(arkinspector arkruntime)

include_directories(
    ${PANDA_ROOT}/runtime/include/
    ${PANDA_THIRD_PARTY_SOURCES_DIR}/asio/asio/include/
    ${PANDA_THIRD_PARTY_SOURCES_DIR}/websocketpp/
)

target_link_libraries(arkinspector
    arkbase
    arkfile
    arkruntime
)

panda_add_sanitizers(TARGET arkinspector SANITIZERS ${PANDA_SANITIZERS_LIST})

panda_add_to_clang_tidy(TARGET arkinspector CHECKS
    -cppcoreguidelines-pro-bounds-pointer-arithmetic
    -fuchsia-multiple-inheritance
    -fuchsia-virtual-inheritance

    # libpandabase
    -bugprone-exception-escape  # See #8186
)

panda_add_gtest(NAME arkinspector_tests
    NO_CORES
    SOURCES
        tests/breakpoint_test.cpp
        tests/client.cpp
        tests/combined_event_loop.cpp
        tests/expect_pauses.cpp
        tests/inspector_test_base.cpp
        tests/inspector_test.cpp
        tests/instruction_pointer.cpp
        tests/json_property_test.cpp
        tests/server_test.cpp
        tests/step_test.cpp
        tests/test_debugger.cpp
        tests/test_event_loop.cpp
        tests/test_logger.cpp
        tests/test_method.cpp
        tests/test_server.cpp
        tests/ws_logger_test.cpp
    LIBRARIES
        arkassembler
        arkinspector
        arkruntime
        gmock
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
)

panda_add_to_clang_tidy(TARGET arkinspector_tests CHECKS
    -cppcoreguidelines-pro-bounds-pointer-arithmetic
    -fuchsia-multiple-inheritance
    -fuchsia-virtual-inheritance
    -readability-magic-numbers

    # libpandabase
    -bugprone-exception-escape  # See #8186

    # assembler
    -cert-dcl21-cpp
    -cppcoreguidelines-macro-usage

    # GoogleTest
    -cert-err58-cpp
    -cppcoreguidelines-avoid-goto
    -cppcoreguidelines-macro-usage
    -cppcoreguidelines-pro-type-vararg
    -fuchsia-statically-constructed-objects
    -hicpp-avoid-goto
    -hicpp-deprecated-headers
    -misc-non-private-member-variables-in-classes
    -modernize-deprecated-headers
    -readability-redundant-string-init
)
