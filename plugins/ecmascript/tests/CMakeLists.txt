# Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

function(add_test_file_ecma)
    set(prefix ARG)
    set(noValues DISABLE_LIMIT_STD_ALLOC SKIP_AOT SKIP_OSR SKIP_VERIFICATION VERIFIER_FAIL_TEST)
    set(singleValues FILE DEBUG_LOG_MESSAGE DEBUG_LOG_MESSAGE_ENFORCED_JIT GC_OPTIONS)
    set(multiValues EXPECTED_STDOUT VERIFIER_EXPECTED_STDOUT VERIFIER_SEARCH_STDERR ARGUMENTS RUNTIME_OPTIONS COMPILER_OPTIONS PRLIMIT_OPTIONS)
    cmake_parse_arguments(${prefix}
                          "${noValues}"
                          "${singleValues}"
                          "${multiValues}"
                          ${ARGN})


    if (NOT DEFINED ARG_FILE)
        message(FATAL_ERROR "Mandatory FILE argument is not defined.")
    endif()

    get_filename_component(target "${ARG_FILE}" NAME)
    get_filename_component(path "${ARG_FILE}" DIRECTORY)
    get_filename_component(suite ${path} NAME)

    if (NOT ARG_DISABLE_LIMIT_STD_ALLOC)
        set(ARG_RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS} --limit-standard-alloc=true")
    endif()

    if(DEFINED ARG_GC_OPTIONS)
        set(ARG_RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS}" "${ARG_GC_OPTIONS}")
    endif()

    if (PANDA_ENABLE_EVENTS)
        set(ARG_RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS} --events-output=csv")
    endif()

    #TODO: add test suite to check with PANDA_COMPILER_TARGET
    if (((PANDA_TARGET_ARM64) OR (PANDA_TARGET_ARM32) OR (PANDA_TARGET_AMD64)) AND DEFINED ARG_DEBUG_LOG_MESSAGE_ENFORCED_JIT)
      set(CUR_RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS}" "--log-level=debug")
      set(CUR_JIT_DEBUG_LOG_MESSAGE "${ARG_DEBUG_LOG_MESSAGE_ENFORCED_JIT}")
    elseif(DEFINED ARG_DEBUG_LOG_MESSAGE)
      set(CUR_RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS}" "--log-level=debug")
      set(CUR_JIT_DEBUG_LOG_MESSAGE "${ARG_DEBUG_LOG_MESSAGE}")
      set(CUR_DEBUG_LOG_MESSAGE "${ARG_DEBUG_LOG_MESSAGE}")
    else()
      set(CUR_RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS}")
    endif()

    set(language_context "ecmascript")
    set(ARG_SKIP_AOT TRUE) # aot mode is not supported yet
    set(ARG_SKIP_VERIFICATION TRUE) #verification is not supported yet
    set(ARG_SKIP_OSR TRUE) #osr is not supported yet
    set(CUR_RUNTIME_OPTIONS "${ARG_RUNTIME_OPTIONS}" "--run-gc-in-place=true") # ecma vm doesn't support concurrent GC

    if (ARG_VERIFIER_FAIL_TEST)
        set(VERIFIER_FAIL_TEST VERIFIER_FAIL_TEST)
    else()
        set(VERIFIER_FAIL_TEST)
    endif()

    if (NOT ARG_SKIP_VERIFICATION)
        verifier_add_test_run(
            FILE "${ARG_FILE}"
            TARGET ${target}-verifier
            SUBDIR ${suite}-verifier
            ${VERIFIER_FAIL_TEST}
            EXPECTED_STDOUT "${ARG_VERIFIER_EXPECTED_STDOUT}"
            SEARCH_STDERR "${ARG_VERIFIER_SEARCH_STDERR}"
            LANGUAGE_CONTEXT "${language_context}"
        )
        add_dependencies(${suite} ${target}-verifier)
    endif()

    panda_add_test_run(
        FILE "${ARG_FILE}"
        TARGET ${target}-stw
        SUBDIR ${suite}-stw
        EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
        SEARCH_DEBUG_STDERR ${CUR_DEBUG_LOG_MESSAGE}
        RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-enable-jit=false --gc-type=stw"
        COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
        ARGUMENTS "${ARG_ARGUMENTS}"
        LANGUAGE_CONTEXT "${language_context}"
        AOT_MODE FALSE
        PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
    )
    add_dependencies(${suite} ${target}-stw)


    panda_add_test_run(
        FILE "${ARG_FILE}"
        TARGET ${target}-gengc
        SUBDIR ${suite}-gengc
        EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
        SEARCH_DEBUG_STDERR ${CUR_DEBUG_LOG_MESSAGE}
        RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-enable-jit=false --gc-type=gen-gc"
        COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
        ARGUMENTS "${ARG_ARGUMENTS}"
        LANGUAGE_CONTEXT "${language_context}"
        AOT_MODE FALSE
        PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
    )
    add_dependencies(${suite} ${target}-gengc)

    # TODO(dtrubenkov): remove this if after enabling G1GC
    if (PANDA_ENABLE_G1GC_TESTS)
    panda_add_test_run(
            FILE "${ARG_FILE}"
            TARGET ${target}-g1gc
            SUBDIR ${suite}-g1gc
            EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
            SEARCH_DEBUG_STDERR ${CUR_DEBUG_LOG_MESSAGE}
            RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-enable-jit=false --gc-type=g1-gc"
            COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
            ARGUMENTS "${ARG_ARGUMENTS}"
            LANGUAGE_CONTEXT "${language_context}"
            AOT_MODE FALSE
            PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
    )
    add_dependencies(${suite} ${target}-g1gc)
    endif()

    # Check if JIT-compilation wasn't turned off explicitly:
    if (PANDA_COMPILER_ENABLE AND NOT CUR_RUNTIME_OPTIONS MATCHES "^.*--compiler-enable-jit=false.*$")
        panda_add_test_run(
            FILE "${ARG_FILE}"
            TARGET ${target}-enforce-jit-compiler
            SUBDIR ${suite}-enforce-jit-compiler
            EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
            SEARCH_DEBUG_STDERR ${CUR_JIT_DEBUG_LOG_MESSAGE}
            RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-enable-jit=true --compiler-hotness-threshold=0 --no-async-jit=true"
            COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
            ARGUMENTS "${ARG_ARGUMENTS}"
            LANGUAGE_CONTEXT "${language_context}"
            AOT_MODE FALSE
            PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
        )
        add_dependencies(${suite} ${target}-enforce-jit-compiler)
        add_dependencies(cts-jit-tests ${target}-enforce-jit-compiler)

        if (PANDA_TARGET_ARM64 AND NOT ARG_SKIP_OSR)
            panda_add_test_run(
                FILE "${ARG_FILE}"
                TARGET ${target}-osr-jit
                SUBDIR ${suite}-osr-jit
                EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
                RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-enable-jit=true --compiler-hotness-threshold=2 --compiler-enable-osr=true"
                COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
                ARGUMENTS "${ARG_ARGUMENTS}"
                LANGUAGE_CONTEXT "${language_context}"
                AOT_MODE FALSE
                PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
            )
            add_dependencies(${suite} ${target}-osr-jit)
            add_dependencies(cts-jit-tests ${target}-osr-jit)
        endif()
    endif()

    if (PANDA_TARGET_AMD64 AND PANDA_CI_TESTING_MODE STREQUAL "Nightly")
        panda_add_test_run(
            FILE "${ARG_FILE}"
            TARGET ${target}-arm32
            SUBDIR ${suite}-arm32
            EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
            SEARCH_DEBUG_STDERR ${CUR_DEBUG_LOG_MESSAGE}
            RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-cross-arch=arm"
            COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
            ARGUMENTS "${ARG_ARGUMENTS}"
            LANGUAGE_CONTEXT "${language_context}"
            AOT_MODE FALSE
            PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
        )
        add_dependencies(${suite} ${target}-arm32)

        panda_add_test_run(
            FILE "${ARG_FILE}"
            TARGET ${target}-arm32-enforce-jit-compiler
            SUBDIR ${suite}-arm32-enforce-jit-compiler
            EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
            SEARCH_DEBUG_STDERR ${CUR_JIT_DEBUG_LOG_MESSAGE}
            RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-hotness-threshold=0 --no-async-jit=true --compiler-cross-arch=arm"
            COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
            ARGUMENTS "${ARG_ARGUMENTS}"
            LANGUAGE_CONTEXT "${language_context}"
            AOT_MODE FALSE
            PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
        )
        add_dependencies(${suite} ${target}-arm32-enforce-jit-compiler)

        panda_add_test_run(
            FILE "${ARG_FILE}"
            TARGET ${target}-arm64
            SUBDIR ${suite}-arm64
            EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
            SEARCH_DEBUG_STDERR ${CUR_DEBUG_LOG_MESSAGE}
            RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-cross-arch=arm64"
            COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
            ARGUMENTS "${ARG_ARGUMENTS}"
            LANGUAGE_CONTEXT "${language_context}"
            AOT_MODE FALSE
            PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
        )
        add_dependencies(${suite} ${target}-arm64)

        panda_add_test_run(
            FILE "${ARG_FILE}"
            TARGET ${target}-arm64-enforce-jit-compiler
            SUBDIR ${suite}-arm64-enforce-jit-compiler
            EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
            SEARCH_DEBUG_STDERR ${CUR_JIT_DEBUG_LOG_MESSAGE}
            RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}" "--compiler-hotness-threshold=0 --no-async-jit=true --compiler-cross-arch=arm64"
            COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
            ARGUMENTS "${ARG_ARGUMENTS}"
            LANGUAGE_CONTEXT "${language_context}"
            AOT_MODE FALSE
            PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
        )
        add_dependencies(${suite} ${target}-arm64-enforce-jit-compiler)
    endif()

    if ((PANDA_TARGET_ARM64 OR PANDA_TARGET_AMD64) AND NOT ARG_SKIP_AOT)
        panda_add_test_run(
            FILE "${ARG_FILE}"
            TARGET ${target}-aot
            SUBDIR ${suite}-aot
            EXPECTED_STDOUT "${ARG_EXPECTED_STDOUT}"
            RUNTIME_OPTIONS "${CUR_RUNTIME_OPTIONS}"
            COMPILER_OPTIONS "${ARG_COMPILER_OPTIONS}"
            GC_OPTIONS "${ARG_GC_OPTIONS}"
            ARGUMENTS "${ARG_ARGUMENTS}"
            LANGUAGE_CONTEXT "${language_context}"
            AOT_MODE TRUE
            PRLIMIT_OPTIONS "${ARG_PRLIMIT_OPTIONS}"
        )
        add_dependencies(${suite} ${target}-aot)
        add_dependencies(cts-aot-tests ${target}-aot)
    endif()

endfunction()

if(CMAKE_CROSSCOMPILING)
    ExternalProject_Get_Property(panda_host_tools binary_dir)
    set(es2panda_target build_host_tools)
    set(es2panda_bin    "${binary_dir}/plugins/ecmascript/es2panda/aot/es2panda")
else()
    set(es2panda_target es2panda)
    set(es2panda_bin    $<TARGET_FILE:${es2panda_target}>)
endif()

if (CMAKE_CROSSCOMPILING AND PANDA_TARGET_ARM64)
# ecmascript tests
# add_test_file_ecma(FILE "${CMAKE_CURRENT_SOURCE_DIR}/ecmascript-tests/js-bitops-bitwise-and.pa" COMPILER_OPTIONS --compiler-hotness-threshold=0)
endif()

add_subdirectory(runtime)

if(PANDA_WITH_COMPILER)
    add_subdirectory(compiler)
    add_subdirectory(bytecode_optimizer)
endif()

add_subdirectory(checked)
add_subdirectory(assembler)
add_subdirectory(disassembler)
