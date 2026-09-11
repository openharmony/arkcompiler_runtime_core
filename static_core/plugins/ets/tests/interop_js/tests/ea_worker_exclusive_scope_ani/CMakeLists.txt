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

set(TEST_TARGET ets_interop_js_ea_worker_exclusive_scope_ani)
set(TEST_PACKAGE ${TEST_TARGET}_package)
set(TEST_RUNNER ets_ani_hybrid_runner)
set(TEST_MODULE_DIR ${CMAKE_CURRENT_BINARY_DIR}/module)
set(TEST_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TEST_TARGET}_output.txt)

panda_ets_package(${TEST_PACKAGE}
    ETS_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/exclusive_scope_plan.ets
)

compile_dynamic_file(${TEST_TARGET}_js_modules
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/exclusive_scope_helper.js
    COMPILE_OPTION --module --merge-abc --enable-ets-implements
)

compile_dynamic_file(${TEST_TARGET}_launcher
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ani_hybrid_launcher.js
    OUTPUT_ABC_PATHS TEST_LAUNCHER_ABC
    COMPILE_OPTION --commonjs
)

panda_ets_interop_js_plugin(${TEST_RUNNER}
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ani_hybrid_runner.cpp
    LIBRARIES arkruntime
    OUTPUT_SUFFIX ".so"
)
panda_target_include_directories(${TEST_RUNNER}
    PRIVATE ${PANDA_ETS_PLUGIN_SOURCE}/runtime/ani
)

add_custom_target(${TEST_TARGET}_modules
    COMMAND mkdir -p ${TEST_MODULE_DIR}
    COMMAND ln -sf ${PANDA_BINARY_ROOT}/lib/module/ets_interop_js_napi.so
            ${TEST_MODULE_DIR}/libets_interop_js_napi.so
    COMMAND ln -sf $<TARGET_FILE:${TEST_RUNNER}>
            ${TEST_MODULE_DIR}/libets_ani_hybrid_runner.so
    DEPENDS ets_interop_js_napi ${TEST_RUNNER}
)

add_custom_target(${TEST_TARGET}
    COMMAND
        "LD_LIBRARY_PATH=${PANDA_BINARY_ROOT}/lib/interop_js/:${PANDA_BINARY_ROOT}/lib/"
        "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
        "ARK_ETS_TEST_ABC_PATH=${PANDA_BINARY_ROOT}/abc/${TEST_PACKAGE}.zip"
        ${ARK_JS_NAPI_CLI}
        --log-level=fatal
        --stub-file=${ARK_JS_STUB_FILE}
        --enable-force-gc=false
        --entry-point=ani_hybrid_launcher
        ${TEST_LAUNCHER_ABC}
        > ${TEST_OUTPUT} 2>&1 || (cat ${TEST_OUTPUT} && false)
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
        ${TEST_PACKAGE}
        ${TEST_TARGET}_js_modules
        ${TEST_TARGET}_launcher
        ${TEST_TARGET}_modules
)

add_dependencies(ets_interop_js_tests ${TEST_TARGET})
