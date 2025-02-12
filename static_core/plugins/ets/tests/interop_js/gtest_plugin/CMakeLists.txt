# Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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


# Create ets_interop_js_gtest plugin
panda_add_library(ets_interop_js_gtest STATIC ets_interop_js_gtest.cpp)
panda_target_link_libraries(ets_interop_js_gtest gtest)
panda_target_link_libraries(ets_interop_js_gtest arkruntime)
set_target_properties(ets_interop_js_gtest PROPERTIES POSITION_INDEPENDENT_CODE ON)
panda_target_include_directories(ets_interop_js_gtest
    PUBLIC ${PANDA_ETS_PLUGIN_SOURCE}/tests/interop_js/gtest_plugin
    PUBLIC ${PANDA_ETS_PLUGIN_SOURCE}/tests/interop_js/test_helper
    PUBLIC ${PANDA_ROOT}
)

if(PANDA_TARGET_OHOS)
    panda_target_link_libraries(ets_interop_js_gtest ace_napi.z)
else()
    panda_target_include_directories(ets_interop_js_gtest SYSTEM PRIVATE ${NAPI_HEADERS_PATH})
endif()

# Link gtest_launcher_nodevm.js to <INTEROP_TESTS_DIR>
set(GTEST_LAUNCHER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/gtest_launcher_nodevm.js")
set(GTEST_LAUNCHER "${PANDA_BINARY_ROOT}/tests/ets_interop_js/gtest_launcher_nodevm.js")
add_custom_command(
    OUTPUT ${GTEST_LAUNCHER}
    COMMENT "Link ${GTEST_LAUNCHER_SOURCE} to ${GTEST_LAUNCHER}"
    COMMAND ln -sf ${GTEST_LAUNCHER_SOURCE} ${GTEST_LAUNCHER}
    DEPENDS ${GTEST_LAUNCHER_SOURCE}
)
add_custom_target(ets_interop_js_gtest_launcher
    DEPENDS ${GTEST_LAUNCHER} ets_interop_js_napi
)

if(PANDA_JS_ETS_HYBRID_MODE)
    compile_dynamic_file(ets_interop_js_gtest_launcher_arkjsvm
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/gtest_launcher_arkjsvm.js"
        OUTPUT_DIR "${INTEROP_TESTS_DIR}"
        COMPILE_OPTION --commonjs
    )

    # Make symbolic links to convinient work with requireNapiPreview
    set(SO_FILES_OUTPUT "${INTEROP_TESTS_DIR}/module/lib/")
    set(INTEROP_LIB_SOURCE "${PANDA_BINARY_ROOT}/lib/module/ets_interop_js_napi_arkjsvm.so")
    set(INTEROP_HELPER_LIB_SOURCE "${PANDA_BINARY_ROOT}/lib/arkjsvm_interop/libinterop_test_helper.so")

    add_custom_target(create_slinks_for_interop_gtest_launcher
        COMMAND mkdir -p ${SO_FILES_OUTPUT}
                && ln -sf ${INTEROP_LIB_SOURCE} ${INTEROP_HELPER_LIB_SOURCE} -t ${SO_FILES_OUTPUT}
        DEPENDS ets_interop_js_napi_arkjsvm ${INTEROP_HELPER_LIB_SOURCE}
    )

    add_dependencies(ets_interop_js_gtest_launcher_arkjsvm create_slinks_for_interop_gtest_launcher)
endif(PANDA_JS_ETS_HYBRID_MODE)
