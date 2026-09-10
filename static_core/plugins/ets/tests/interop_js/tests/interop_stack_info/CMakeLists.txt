# Copyright (c) 2026 Huawei Device Co., Ltd.
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

if(PANDA_JS_ETS_HYBRID_MODE)
    panda_ets_interop_js_gtest(ets_interop_js_interop_stack_info
        CPP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/interop_stack_info_test.cpp
        ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/interop_stack_info_test.ets
        JS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/interop_stack_info_helpers.js
        TS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/interop_stack_info_test.ts
    )

    panda_target_compile_definitions(ets_interop_js_interop_stack_info PRIVATE PANDA_JS_ETS_HYBRID_MODE)
    panda_target_include_directories(ets_interop_js_interop_stack_info SYSTEM PRIVATE ${ARKUI_NAPI_HEADERS_PATH})

    add_custom_target(ets_interop_js_interop_stack_info_runtime_modules
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_BINARY_DIR}/interop_stack_info_helpers.abc
            ${INTEROP_TESTS_DIR}/interop_stack_info_helpers.abc
        DEPENDS ets_interop_js_interop_stack_info_dynamic_modules
    )
    add_dependencies(ets_interop_js_interop_stack_info_gtests ets_interop_js_interop_stack_info_runtime_modules)
endif()
