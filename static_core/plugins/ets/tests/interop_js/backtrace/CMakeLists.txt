# Copyright (c) 2025 Huawei Device Co., Ltd.
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

# NOTE: Disable arm32 because dfx not support arm32 platform.
if (PANDA_TARGET_ARM32)
    return()
endif()

ani_add_gtest(backtracetest_by_ani
    CPP_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/backtrace_test.cpp
        ${PANDA_ETS_PLUGIN_SOURCE}/runtime/interop_js/jsbacktrace/backtrace.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/backtrace_test.ets
)

# Build rules for call_backtrace native shared library
panda_add_library(call_backtrace SHARED call_backtrace.cpp)
panda_target_compile_options(call_backtrace PRIVATE -Wno-invalid-offsetof)
panda_target_include_directories(call_backtrace PRIVATE
    ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani
    ${PANDA_ROOT}
    ${PANDA_ROOT}/libpandabase
    ${PANDA_ROOT}/libpandafile
    ${PANDA_ROOT}/runtime
    ${PANDA_BINARY_ROOT}
    ${PANDA_BINARY_ROOT}/libpandabase/generated
    ${PANDA_BINARY_ROOT}/libpandafile/include
    ${PANDA_BINARY_ROOT}/panda_gen_options
    ${PANDA_BINARY_ROOT}/runtime/include
)
panda_target_link_libraries(call_backtrace c_secshared)
add_dependencies(backtracetest_by_ani call_backtrace)
