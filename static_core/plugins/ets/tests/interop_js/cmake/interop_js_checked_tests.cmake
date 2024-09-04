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

set(INTEROP_TESTS_GENERATED_DIR "${PANDA_BINARY_ROOT}/plugins/ets/tests/ets_interop_js/generated")
set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/tests/checked/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/checked/arktsconfig.in.json ${ETS_CONFIG})



add_custom_target(ets_interop_js_checked_tests COMMENT "Run ets_interop_js checked tests")
add_dependencies(ets_tests ets_interop_js_checked_tests)
add_dependencies(ets_interop_tests ets_interop_js_checked_tests)

function(panda_ets_interop_js_checked_test)
    cmake_parse_arguments(ARG "" "FILE" "" ${ARGN})
    if (NOT DEFINED ARG_FILE)
        message(FATAL_ERROR "Mandatory FILE argument is not defined.")
    endif()
    get_filename_component(TARGET "${ARG_FILE}" NAME_WE)
    set(TARGET "interop_${TARGET}.checked")

    set(TARGET_TEST_PACKAGE ${TARGET}_test_package)
    # NOTE (kkonsw): temporary disabling all other checks
    set(ETS_VERIFICATOR_ERRORS "NodeHasParentForAll,EveryChildHasValidParentForAll,VariableHasScopeForAll,NodeHasTypeForAll,IdentifierHasVariableForAll,ReferenceTypeAnnotationIsNullForAll,ArithmeticOperationValidForAll,SequenceExpressionHasLastTypeForAll,ForLoopCorrectlyInitializedForAll,VariableHasEnclosingScopeForAll,ModifierAccessValidForAll,ImportExportAccessValid")
    panda_ets_package(${TARGET_TEST_PACKAGE}
        ETS_SOURCES ${ARG_FILE}
        ETS_CONFIG ${ETS_CONFIG}
        ETS_VERIFICATOR_ERRORS ${ETS_VERIFICATOR_ERRORS}
    )

    set(CHECKER "${PANDA_ROOT}/tests/checked/checker.rb")

    if (PANDA_TARGET_AMD64)
        set(ARCHITECTURE "x64")
    elseif (PANDA_TARGET_ARM64)
        set(ARCHITECTURE "arm64")
    else()
        set(ARCHITECTURE "arm32")
    endif()

    set(JS_LAUNCHER ${CMAKE_CURRENT_LIST_DIR}/run_checked_test.js)
    set(ARK_ETS_INTEROP_JS_PACKAGE_PATH ${PANDA_BINARY_ROOT}/abc/${TARGET_TEST_PACKAGE}.zip)
    set(RUN_COMMAND
                "/usr/bin/env" "MODULE_PATH=${CMAKE_BINARY_DIR}/lib/module"
                "NODE_PATH=${PANDA_BINARY_ROOT}/plugins/ets/tests/ets_interop_js/generated"
                "ARK_ETS_INTEROP_JS_TEST_ABC_PATH=${ARK_ETS_INTEROP_JS_PACKAGE_PATH}"
                "ARK_ETS_STDLIB_PATH=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc"
                ${NODE_BINARY} ${JS_LAUNCHER})
    set(TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}")
    file(MAKE_DIRECTORY "${TEST_DIR}")
    SET(OPTIONS "--run-gc-in-place")
    set(PAOC_OPTIONS ${OPTIONS} "--load-runtimes=ets" "--boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc")
    set(ES2PANDA_OPTIONS --thread=0 --extension=sts --arktsconfig=${ETS_CONFIG} --verifier-errors=${ETS_VERIFICATOR_ERRORS})

    if (PANDA_LLVM_AOT)
        set(WITH_LLVM "--with-llvm")
    endif()

    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
        set(RELEASE_OPT "--release")
    endif()

    add_custom_target(${TARGET}
        COMMAND ${CHECKER} --source ${ARG_FILE}
                        --panda \"${RUN_COMMAND}\"
                        --paoc $<TARGET_FILE:ark_aot>
                        --frontend ${es2panda_bin}
                        --run-prefix \"${PANDA_RUN_PREFIX}\"
                        --test-file ${ARK_ETS_INTEROP_JS_PACKAGE_PATH}
                        --panda-options \"${OPTIONS}\"
                        --paoc-options \"${PAOC_OPTIONS}\"
                        --frontend-options \"${ES2PANDA_OPTIONS}\"
                        --command-token \"//!\"
                        --arch ${ARCHITECTURE}
                        ${WITH_LLVM}
                        ${RELEASE_OPT}
        WORKING_DIRECTORY ${TEST_DIR}
        COMMENT "Running ${TARGET} checked test"
        DEPENDS ${JS_LAUNCHER} ${TARGET_TEST_PACKAGE} ets_interop_js_napi ${ETS_CONFIG})

    add_dependencies(ets_interop_js_checked_tests ${TARGET})
    add_dependencies(ets_checked_tests ${TARGET})
endfunction(panda_ets_interop_js_checked_test)
