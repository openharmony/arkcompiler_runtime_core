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

panda_ets_interop_js_gtest(ets_interop_js_backtracetest
    CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/backtrace_test.cpp
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/backtrace_test.ets
    PACKAGE_NAME "Test"
)

# Build rules for call_backtrace native shared library
panda_add_library(call_backtrace SHARED
    call_backtrace.cpp
    ${PANDA_ETS_PLUGIN_SOURCE}/runtime/interop_js/jsbacktrace/backtrace.cpp
)
panda_target_compile_options(call_backtrace PRIVATE -Wno-invalid-offsetof)
panda_target_link_libraries(call_backtrace arkruntime c_secshared)
add_dependencies(ets_interop_js_backtracetest call_backtrace)
