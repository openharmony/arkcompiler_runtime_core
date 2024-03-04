# Copyright (c) 2024 Huawei Device Co., Ltd.
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

function(add_ark_js_napi_interop_test TEST_NAME)
    # Parse arguments
    cmake_parse_arguments(
        ARG
        ""
        "MODULE"
        ""
        ${ARGN}
    )

    set(MODULE_FILE ${PANDA_BINARY_ROOT}/lib/module/lib${ARG_MODULE}.so)
    add_custom_command(
        OUTPUT ${MODULE_FILE}
        COMMAND cd ${PANDA_BINARY_ROOT}/lib/module &&
                ln -sf $<TARGET_FILE:${ARG_MODULE}> ${MODULE_FILE}
    )

    set(PREBUILT_BINARIES_DIR ${PANDA_BINARY_ROOT}/ark_js_napi_cli)
    set(JS_TEST_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.js)
    set(OUTPUT_ABC ${CMAKE_CURRENT_BINARY_DIR}/${TEST_NAME}.abc)
    set(ABC_TARGET ${TEST_NAME}_abc)

    add_custom_target(${ABC_TARGET}
        COMMAND ${PREBUILT_BINARIES_DIR}/bin/es2abc --merge-abc --extension=js --commonjs ${JS_TEST_SOURCE} --output ${OUTPUT_ABC}
    )

    set(LD_LIBRARY_PATH LD_LIBRARY_PATH=${PANDA_BINARY_ROOT}/lib:${PREBUILT_BINARIES_DIR}/lib)
    set(TARGET_NAME run_ark_js_napi_interop_${TEST_NAME})
    add_custom_target(${TARGET_NAME}
        COMMAND cd ${PANDA_BINARY_ROOT}/lib &&
                ${LD_LIBRARY_PATH} ${PREBUILT_BINARIES_DIR}/bin/ark_js_napi_cli --entry-point ${TEST_NAME} ${OUTPUT_ABC}
        DEPENDS etsstdlib ${ARG_MODULE} ${MODULE_FILE} ${ABC_TARGET}
    )
    add_dependencies(run_ark_js_napi_interop_tests ${TARGET_NAME})
endfunction()

add_custom_target(run_ark_js_napi_interop_tests COMMENT "Run ark js napi interop tests")

add_ark_js_napi_interop_test(create_etsvm_test MODULE ets_interop_js_napi)

panda_ets_interop_js_plugin(sample_function
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/sample_function.cpp
)
panda_target_include_directories(sample_function
    PUBLIC ${PANDA_ROOT}/libpandabase
)

add_ark_js_napi_interop_test(call_function_test MODULE sample_function)
