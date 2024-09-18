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

if (NOT TARGET es2panda)
    return()
    message(WARNING "es2panda not found for ets tests")
endif()

add_custom_target(ets_gtests)
add_dependencies(ets_tests ets_gtests)

include(cmake/native_gtest.cmake)

add_panda_assembly(TARGET mock_stdlib SOURCE integrational/mock_stdlib.pa)

function(panda_ets_add_gtest)
    panda_add_gtest(
        TEST_GROUP ets_tests
        STASH_LIST ets_stash_list
        ${ARGV}
    )
endfunction()

panda_ets_add_gtest(
    NO_CORES
    NAME ets_tests_runtime
    SOURCES
        runtime/ets_vm_test.cpp
        runtime/ets_vm_init_prealloc_test.cpp
    LIBRARIES
        arkbase arkfile arkruntime
    INCLUDE_DIRS
        ${PANDA_ETS_PLUGIN_SOURCE}/runtime
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
    PANDA_STD_LIB
        $<TARGET_PROPERTY:mock_stdlib,FILE>
    DEPS_TARGETS
        mock_stdlib
)

panda_add_asm_file(
    FILE ${PANDA_ETS_PLUGIN_SOURCE}/tests/integrational/empty_program.pa
    TARGET ets_tests_empty_program
    DEPENDS mock_stdlib
    LANGUAGE_CONTEXT ets
    ADDITIONAL_STDLIBS $<TARGET_PROPERTY:mock_stdlib,FILE>
)

add_dependencies(tests ets_tests)

function(run_deterministic_tests ETS_SRC TARGET)
    set(oneValueArgs ITER_NUM)

    cmake_parse_arguments(ARG "" "${oneValueArgs}" "" ${ARGN})

    if (NOT DEFINED ARG_ITER_NUM)
        SET(ARG_ITER_NUM 10)
    endif()

    get_filename_component(SRC_FNAME ${ETS_SRC} NAME)

    set(TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/${SRC_FNAME}")
    file(MAKE_DIRECTORY "${TEST_DIR}")
    set(DEPENDS_ON "")

    set(FIRST_ABC_APP_TARGET ${TARGET}-${SRC_FNAME}-0-ets-es2panda)

    foreach(i RANGE ${ARG_ITER_NUM})
        set(APP_FILE_OUTPUT "${TEST_DIR}/${i}.abc")
        set(ABC_APP_TARGET ${TARGET}-${SRC_FNAME}-${i}-ets-es2panda)
        list(APPEND all_bin_abc ${APP_FILE_OUTPUT})
        list(APPEND DEPENDS_ON "${ABC_APP_TARGET}")
        compile_ets_code(${ETS_SRC} ${APP_FILE_OUTPUT} ${ABC_APP_TARGET})

        set(target_name ${TARGET}_${SRC_FNAME}_${i})
        list(GET all_bin_abc 0 first)
        add_custom_target(
            ${target_name}
            COMMAND bash -c "diff ${APP_FILE_OUTPUT} ${first}"
            WORKING_DIRECTORY ${TEST_DIR}
            DEPENDS ${ABC_APP_TARGET} ${FIRST_ABC_APP_TARGET}
        )
        list(APPEND dependencies ${target_name})
    endforeach()
    add_custom_target(${TARGET}
        COMMENT "Running ${ETS_SRC} determenistic compilation test"
        DEPENDS ${dependencies}
    )
    add_dependencies(ets_tests ${TARGET})
endfunction()

function(run_ets_code_verifier ETS_SRC WORK_DIR TARGET)
    if (NOT TARGET verifier)
        return()
    endif()

    get_filename_component(SRC_FNAME ${ETS_SRC} NAME)
    set(OUTPUT_ABC ${WORK_DIR}/${SRC_FNAME}.abc)
    set(ABC_TARGET ${TARGET}-ets-es2panda)
    compile_ets_code(${ETS_SRC} ${OUTPUT_ABC} ${ABC_TARGET} OPT_LEVEL ${ARG_OPT_LEVEL})

    set(VERIFIER_ARGUMENTS
        --boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
        --load-runtimes=ets
        ${OUTPUT_ABC}
    )

    add_custom_target(${TARGET}
        COMMENT "Running verifier for ets file: ${OUTPUT_ABC}"
        COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:verifier> ${VERIFIER_ARGUMENTS}
        DEPENDS verifier etsstdlib ${ABC_TARGET}
    )

    add_dependencies(ets_tests ${TARGET})
endfunction()

function(compile_int_ets_export_files INTERPRETER_TYPE WORK_DIR TARGET EXTRA_BOOT_PANDAFILES)
    set(oneValueArgs OPT_LEVEL)
    set(multiValueArgs EXPORT_FILES)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(COUNTER 0)
    set(RESULT "")
    foreach(ARG_EXPORT_FILE IN LISTS ARG_EXPORT_FILES)
        get_filename_component(SRC_EXPORT_FNAME ${ARG_EXPORT_FILE} NAME)
        set(OUTPUT_EXPORT_ABC ${WORK_DIR}/${SRC_EXPORT_FNAME}-${INTERPRETER_TYPE}.abc)
        set(ABC_EXPORT_TARGET ${TARGET}-export-ets-es2panda_${COUNTER})
        math(EXPR COUNTER "${COUNTER} + 1")
        compile_ets_code(${ARG_EXPORT_FILE} ${OUTPUT_EXPORT_ABC} ${ABC_EXPORT_TARGET} OPT_LEVEL ${ARG_OPT_LEVEL} COMPILER_EXTRA_OPTIONS "--ets-module")
        set(RESULT ${RESULT}:${OUTPUT_EXPORT_ABC})
    endforeach()

    set(${EXTRA_BOOT_PANDAFILES} ${RESULT} PARENT_SCOPE)
endfunction()

function(run_int_ets_code_impl INTERPRETER_TYPE ETS_SRC WORK_DIR TARGET)
    if (NOT TARGET es2panda)
        return()
    endif()

    set(oneValueArgs OPT_LEVEL EXPORT_FILE)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS EXPORT_FILES)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    get_filename_component(SRC_FNAME ${ETS_SRC} NAME)
    set(OUTPUT_ABC ${WORK_DIR}/${SRC_FNAME}-${INTERPRETER_TYPE}.abc)
    set(ABC_TARGET ${TARGET}-ets-es2panda)
    compile_ets_code(${ETS_SRC} ${OUTPUT_ABC} ${ABC_TARGET} OPT_LEVEL ${ARG_OPT_LEVEL})

    if (DEFINED ARG_EXPORT_FILES)
        compile_int_ets_export_files(${INTERPRETER_TYPE} ${WORK_DIR} ${TARGET} EXTRA_BOOT_PANDAFILES OPT_LEVEL ${ARG_OPT_LEVEL} EXPORT_FILES ${ARG_EXPORT_FILES})
    endif()

    set(RUNTIME_ARGUMENTS
        --boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc${EXTRA_BOOT_PANDAFILES}
        --load-runtimes=ets
        --compiler-enable-jit=false
        --interpreter-type=${INTERPRETER_TYPE}
        ${ARG_RUNTIME_EXTRA_OPTIONS}
        ${OUTPUT_ABC}
        ETSGLOBAL::main
    )

    add_custom_target(${TARGET}
        COMMENT "Running ark INT(${INTERPRETER_TYPE}) for ets file: ${OUTPUT_ABC}"
        COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS}
        DEPENDS ark etsstdlib ${ABC_TARGET}
    )

    string(FIND "${RUNTIME_ARGUMENTS}" "--gc-type" GC_TYPE_SET)
    if(NOT PANDA_LLVM_INTERPRETER AND ${GC_TYPE_SET} EQUAL -1)
        add_custom_target(${TARGET}-gc
            COMMENT "Running ark INT(${INTERPRETER_TYPE}) for ets file: ${OUTPUT_ABC}"
            COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS} --gc-type=gen-gc
            COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS} --gc-type=stw
            DEPENDS ark etsstdlib ${ABC_TARGET}
        )
        add_dependencies(ets_tests ${TARGET}-gc)
    endif()

    set(COUNTER 0)
    foreach(ARG_EXPORT_FILE IN LISTS ARG_EXPORT_FILES)
        set(ABC_EXPORT_TARGET ${TARGET}-export-ets-es2panda_${COUNTER})
        math(EXPR COUNTER "${COUNTER} + 1")
        add_dependencies(${TARGET} ${ABC_EXPORT_TARGET})
        if(NOT PANDA_LLVM_INTERPRETER AND ${GC_TYPE_SET} EQUAL -1)
            add_dependencies(${TARGET}-gc ${ABC_EXPORT_TARGET})
        endif()
    endforeach()

    add_dependencies(ets_tests ${TARGET})
endfunction()

function(run_int_ets_code ETS_SRC WORK_DIR TARGET)
    set(oneValueArgs OPT_LEVEL)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS EXPORT_FILES)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_custom_target(${TARGET})
    add_dependencies(ets_tests ${TARGET})

    run_int_ets_code_impl(cpp ${ETS_SRC} ${WORK_DIR} ${TARGET}-cpp OPT_LEVEL ${ARG_OPT_LEVEL} RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS} EXPORT_FILES ${ARG_EXPORT_FILES})
    run_int_ets_code_impl(irtoc ${ETS_SRC} ${WORK_DIR} ${TARGET}-irtoc OPT_LEVEL ${ARG_OPT_LEVEL} RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS} EXPORT_FILES ${ARG_EXPORT_FILES})
    add_dependencies(${TARGET} ${TARGET}-irtoc ${TARGET}-cpp)
    if (PANDA_LLVM_INTERPRETER)
        run_int_ets_code_impl(llvm ${ETS_SRC} ${WORK_DIR} ${TARGET}-llvmirtoc OPT_LEVEL ${ARG_OPT_LEVEL} RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS} EXPORT_FILES ${ARG_EXPORT_FILES})
        add_dependencies(${TARGET} ${TARGET}-llvmirtoc)
    endif()
endfunction()

function(run_jit_ets_code ETS_SRC WORK_DIR TARGET)
    set(oneValueArgs OPT_LEVEL)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    get_filename_component(SRC_FNAME ${ETS_SRC} NAME)
    set(OUTPUT_ABC ${WORK_DIR}/${SRC_FNAME}-jit.abc)
    set(ABC_TARGET ${TARGET}-ets-es2panda)
    compile_ets_code(${ETS_SRC} ${OUTPUT_ABC} ${ABC_TARGET} OPT_LEVEL ${ARG_OPT_LEVEL})

    set(RUNTIME_ARGUMENTS
        --boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
        --load-runtimes=ets
        --compiler-enable-jit=true
        --no-async-jit=true
        --compiler-hotness-threshold=0
        --compiler-ignore-failures=false
        ${ARG_RUNTIME_EXTRA_OPTIONS}
        ${OUTPUT_ABC}
        ETSGLOBAL::main
    )

    add_custom_target(${TARGET}
        COMMENT "Running ark JIT for ets file: ${OUTPUT_ABC}"
        COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS}
        DEPENDS ark etsstdlib ${ABC_TARGET}
    )

    add_dependencies(ets_tests ${TARGET})
endfunction()

function(run_jit_osr_ets_code ETS_SRC WORK_DIR TARGET)
    set(oneValueArgs OPT_LEVEL)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    get_filename_component(SRC_FNAME ${ETS_SRC} NAME)
    set(OUTPUT_ABC ${WORK_DIR}/${SRC_FNAME}-jit.abc)
    set(ABC_TARGET ${TARGET}-ets-es2panda)
    compile_ets_code(${ETS_SRC} ${OUTPUT_ABC} ${ABC_TARGET} OPT_LEVEL ${ARG_OPT_LEVEL})

    set(RUNTIME_ARGUMENTS
        --boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
        --load-runtimes=ets
        --compiler-enable-jit=true
        --compiler-hotness-threshold=2
        --compiler-enable-osr=true
        --compiler-ignore-failures=true
        ${ARG_RUNTIME_EXTRA_OPTIONS}
        ${OUTPUT_ABC}
        ETSGLOBAL::main
    )

    add_custom_target(${TARGET}
        COMMENT "Running ark JIT for ets file: ${OUTPUT_ABC}"
        COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS}
        DEPENDS ark etsstdlib ${ABC_TARGET}
    )

    add_dependencies(ets_tests ${TARGET})
endfunction()

function(compile_aot_ets_code PAOC_MODE ETS_ABC OUTPUT_AN TARGET ABC_TARGET)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "" "${multiValueArgs}" ${ARGN})

    set(AOT_RUNTIME_ARGUMENTS
        --boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
        --load-runtimes=ets
        --compiler-ignore-failures=false
        --paoc-panda-files=${ETS_ABC}
        --paoc-output=${OUTPUT_AN}
        --paoc-mode=${PAOC_MODE}
        ${ARG_RUNTIME_EXTRA_OPTIONS}
    )

    add_custom_target(${TARGET}
        COMMENT "Compile ark AOT(${PAOC_MODE}) for ets file: ${ETS_ABC}"
        COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark_aot> ${AOT_RUNTIME_ARGUMENTS}
        DEPENDS ark_aot etsstdlib ${ABC_TARGET}
    )
endfunction()

function(run_aot_ets_code PAOC_MODE ETS_SRC WORK_DIR TARGET)
    if (CMAKE_CROSSCOMPILING)
        return()
    endif()

    set(oneValueArgs OPT_LEVEL)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    get_filename_component(SRC_FNAME ${ETS_SRC} NAME)
    set(OUTPUT_ABC ${WORK_DIR}/${SRC_FNAME}-${PAOC_MODE}.abc)
    set(ABC_TARGET ${TARGET}-ets-es2panda)
    compile_ets_code(${ETS_SRC} ${OUTPUT_ABC} ${ABC_TARGET} OPT_LEVEL ${ARG_OPT_LEVEL})

    set(OUTPUT_AN ${WORK_DIR}/${SRC_FNAME}-${PAOC_MODE}.an)
    set(TARGET_AOT ${TARGET}-${PAOC_MODE}-files)
    compile_aot_ets_code(${PAOC_MODE} ${OUTPUT_ABC} ${OUTPUT_AN} ${TARGET_AOT} ${ABC_TARGET} RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS})

    set(RUNTIME_ARGUMENTS
        --boot-panda-files=${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc
        --load-runtimes=ets
        --aot-file=${OUTPUT_AN}
        ${ARG_RUNTIME_EXTRA_OPTIONS}
        ${OUTPUT_ABC}
        ETSGLOBAL::main
    )

    add_custom_target(${TARGET}
        COMMENT "Running ark AOT(${PAOC_MODE}) for ets file: ${OUTPUT_ABC}"
        COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark> ${RUNTIME_ARGUMENTS}
        DEPENDS ark etsstdlib ${TARGET_AOT}
    )

    add_dependencies(ets_tests ${TARGET})
endfunction()

function(run_deterministic_test ETS_SRC WORK_DIR TARGET)
    # deterministic binary
    run_deterministic_tests(${ETS_SRC} ${TARGET}-determ)
    if(TARGET ${TARGET}-determ)
        add_dependencies(${TARGET} ${TARGET}-determ)
    endif()
endfunction()

function(run_int_jit_aot_ets_code ETS_SRC WORK_DIR TARGET)
    set(oneValueArgs OPT_LEVEL)
    set(noValues SKIP_ARM32_COMPILER)
    set(multiValueArgs RUNTIME_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "${noValues}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_custom_target(${TARGET})

    # int
    run_int_ets_code(${ETS_SRC} ${WORK_DIR} ${TARGET}-ets-int OPT_LEVEL ${ARG_OPT_LEVEL} RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS})
    if(TARGET ${TARGET}-ets-int)
        add_dependencies(${TARGET} ${TARGET}-ets-int)
    endif()
    # Compiler doesn't support some instruction in arm32 mode(launch)
    if (ARG_SKIP_ARM32_COMPILER AND PANDA_TARGET_ARM32)
        return()
    endif()
    # jit
    run_jit_ets_code(${ETS_SRC} ${WORK_DIR} ${TARGET}-ets-jit OPT_LEVEL ${ARG_OPT_LEVEL} RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS})
    if(TARGET ${TARGET}-ets-jit)
        add_dependencies(${TARGET} ${TARGET}-ets-jit)
    endif()
    # aot
    run_aot_ets_code(aot ${ETS_SRC} ${WORK_DIR} ${TARGET}-ets-aot OPT_LEVEL ${ARG_OPT_LEVEL} RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS})
    if(TARGET ${TARGET}-ets-aot)
        add_dependencies(${TARGET} ${TARGET}-ets-aot)
    endif()
    # llvm-aot
    if (PANDA_LLVM_AOT)
        run_aot_ets_code(llvm ${ETS_SRC} ${WORK_DIR} ${TARGET}-ets-llvmaot OPT_LEVEL ${ARG_OPT_LEVEL} RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS})
        if(TARGET ${TARGET}-ets-llvmaot)
            add_dependencies(${TARGET} ${TARGET}-ets-llvmaot)
        endif()
    endif()
    # jit-osr
    if (PANDA_TARGET_ARM64)
        run_jit_osr_ets_code(${ETS_SRC} ${WORK_DIR} ${TARGET}-ets-jit-osr OPT_LEVEL ${ARG_OPT_LEVEL} RUNTIME_EXTRA_OPTIONS ${ARG_RUNTIME_EXTRA_OPTIONS})
        if(TARGET ${TARGET}-ets-jit-osr)
            add_dependencies(${TARGET} ${TARGET}-ets-jit-osr)
        endif()
    endif()
endfunction()

function(run_int_jit_aot_ets_code_foreach_gc PARENT_TARGET ETS_SRC WORK_DIR TARGET)
    set(gc_types g1-gc gen-gc stw)

    foreach(gc_type ${gc_types})
        set(extra_options RUNTIME_EXTRA_OPTIONS "--gc-type=${gc_type}")
        run_int_jit_aot_ets_code(${ETS_SRC} ${WORK_DIR}-${gc_type} ${TARGET}-${gc_type} ${extra_options})
        add_dependencies(${PARENT_TARGET} ${TARGET}-${gc_type})
    endforeach()
endfunction()

set (COMPILER_OPTIONS_OSR "--compiler-ignore-failures=true" ${COMPILER_OPTIONS})

function(compile_stdlib TARGET_ARCH)
    add_custom_target(ets-compile-stdlib-aot-${TARGET_ARCH}
        COMMENT "Running ark_aot compilation for etsstdlib for ${TARGET_ARCH} with default options"
        COMMAND ${ARK_AOT_RUNNER} --binary-dir=${CMAKE_BINARY_DIR} --target-arch=${TARGET_ARCH} --paoc-mode=aot -compiler-options="${COMPILER_OPTIONS}" -paoc-output=etsstdlib_aot_${TARGET_ARCH}.an
        DEPENDS ark_aot etsstdlib
    )
    add_custom_target(ets-compile-stdlib-jit-${TARGET_ARCH}
        COMMENT "Running ark_aot compilation for etsstdlib for ${TARGET_ARCH} with jit mode"
        COMMAND ${ARK_AOT_RUNNER} --binary-dir=${CMAKE_BINARY_DIR} --target-arch=${TARGET_ARCH} --paoc-mode=jit -compiler-options="${COMPILER_OPTIONS}" -paoc-output=etsstdlib_jit_${TARGET_ARCH}.an
        DEPENDS ark_aot etsstdlib
    )
    add_custom_target(ets-compile-stdlib-aot-${TARGET_ARCH}-no-inline
        COMMENT "Running ark_aot compilation for etsstdlib for ${TARGET_ARCH} without inlining"
        COMMAND ${ARK_AOT_RUNNER} --binary-dir=${CMAKE_BINARY_DIR} --target-arch=${TARGET_ARCH} --paoc-mode=aot -compiler-options="${COMPILER_OPTIONS_NO_INLINE}" -paoc-output=etsstdlib_aot_${TARGET_ARCH}_no_inline.an
        DEPENDS ark_aot etsstdlib
    )
    add_custom_target(ets-compile-stdlib-aot-${TARGET_ARCH}-stw
        COMMENT "Running ark_aot compilation for etsstdlib for ${TARGET_ARCH} with gc stw"
        COMMAND ${ARK_AOT_RUNNER} --binary-dir=${CMAKE_BINARY_DIR} --target-arch=${TARGET_ARCH} --paoc-mode=aot -compiler-options="${COMPILER_OPTIONS_GC_STW}" -paoc-output=etsstdlib_aot_${TARGET_ARCH}_stw.an
        DEPENDS ark_aot etsstdlib
    )
    add_dependencies(ets-compile-stdlib ets-compile-stdlib-aot-${TARGET_ARCH} ets-compile-stdlib-jit-${TARGET_ARCH} ets-compile-stdlib-aot-${TARGET_ARCH}-no-inline ets-compile-stdlib-aot-${TARGET_ARCH}-stw)
    if (PANDA_LLVM_AOT)
        add_custom_target(ets-compile-stdlib-llvm-${TARGET_ARCH}
            COMMENT "Running ark_aot compilation for etsstdlib for ${TARGET_ARCH} with llvm"
            COMMAND ${ARK_AOT_RUNNER} --binary-dir=${CMAKE_BINARY_DIR} --target-arch=${TARGET_ARCH} --paoc-mode=llvm -compiler-options="${COMPILER_OPTIONS_LLVM}" -paoc-output=etsstdlib_llvmaot_${TARGET_ARCH}.an
            DEPENDS ark_aot etsstdlib
        )
        add_custom_target(ets-compile-stdlib-llvm-pre-opt-2-${TARGET_ARCH}
            COMMENT "Running ark_aot compilation for etsstdlib for ${TARGET_ARCH} with llvm pre-opt"
            COMMAND ${ARK_AOT_RUNNER} --binary-dir=${CMAKE_BINARY_DIR} --target-arch=${TARGET_ARCH} --paoc-mode=llvm -compiler-options="${COMPILER_OPTIONS_LLVM_PRE_OPT}" -paoc-output=etsstdlib_llvmaot_pre_opt_2_${TARGET_ARCH}.an
            DEPENDS ark_aot etsstdlib
        )
        add_dependencies(
                ets-compile-stdlib
                ets-compile-stdlib-llvm-${TARGET_ARCH}
                ets-compile-stdlib-llvm-pre-opt-2-${TARGET_ARCH}
        )
    endif()
    if (TARGET_ARCH STREQUAL "arm64")
        add_custom_target(ets-compile-stdlib-osr-${TARGET_ARCH}
            COMMENT "Running ark_aot compilation for etsstdlib for ${TARGET_ARCH} with osr mode"
            COMMAND ${ARK_AOT_RUNNER} --binary-dir=${CMAKE_BINARY_DIR} --target-arch=${TARGET_ARCH} --paoc-mode=osr -compiler-options="${COMPILER_OPTIONS_OSR}" -paoc-output=etsstdlib_osr_${TARGET_ARCH}.an
            DEPENDS ark_aot etsstdlib
        )
        add_dependencies(ets-compile-stdlib ets-compile-stdlib-osr-${TARGET_ARCH})
    endif()
endfunction()

run_deterministic_tests(GEN_STD_LIB
                        ets-stdlib-deterministic
                        ITER_NUM 3)
#add_dependencies(ets_tests ets-stdlib-deterministic)

if(NOT CMAKE_CROSSCOMPILING)
    if (NOT (PANDA_ENABLE_ADDRESS_SANITIZER OR PANDA_ENABLE_THREAD_SANITIZER) OR
        NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR
        PANDA_CI_TESTING_MODE STREQUAL "Nightly")
            panda_set_flag(PANDA_TEST_COMPILE_STDLIB)
    endif()
endif()

if(PANDA_TEST_COMPILE_STDLIB)
    set (ARK_AOT_RUNNER ${CMAKE_SOURCE_DIR}/plugins/ets/compiler/tools/paoc_compile_stdlib.sh)
    if (NOT PANDA_CI_TESTING_MODE STREQUAL "Nightly")
       set (COMPILER_OPTIONS "--compiler-check-final=true")
       if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
          set (COMPILER_OPTIONS ${COMPILER_OPTIONS} "--compiler-inst-graph-coloring-limit=2000")
       endif()
    endif()
    set (COMPILER_OPTIONS_NO_INLINE "--compiler-inlining=false" ${COMPILER_OPTIONS})
    set (COMPILER_OPTIONS_GC_STW "--gc-type=stw" ${COMPILER_OPTIONS})
    if (PANDA_LLVM_AOT)
        set (COMPILER_OPTIONS_LLVM ${COMPILER_OPTIONS})
        set (COMPILER_OPTIONS_LLVM_PRE_OPT "--llvm-pre-opt=2" ${COMPILER_OPTIONS})
        # NOTE(zdenis): False positives come from libLLVM.so
        if (NOT PANDA_ENABLE_THREAD_SANITIZER)
            set (COMPILER_OPTIONS_LLVM "--llvmaot-threads=4" "--llvmaot-methods-per-module=512" ${COMPILER_OPTIONS_LLVM})
            set (COMPILER_OPTIONS_LLVM_PRE_OPT "--llvmaot-threads=4" "--llvmaot-methods-per-module=512" ${COMPILER_OPTIONS_LLVM_PRE_OPT})
        endif()
    endif ()
    add_custom_target(ets-compile-stdlib)
    if (PANDA_COMPILER_TARGET_X86_64)
        compile_stdlib(x86_64)
    endif()
    if (PANDA_COMPILER_TARGET_AARCH64)
        compile_stdlib(arm64)
    endif()
    add_dependencies(ets_tests ets-compile-stdlib)
endif()

add_subdirectory(common)
add_subdirectory(equals)
add_subdirectory(lookup_by_name)
add_subdirectory(ets_test_suite)
add_subdirectory(runtime)
add_subdirectory(napi)
if(NOT (PANDA_ENABLE_ADDRESS_SANITIZER OR PANDA_ENABLE_THREAD_SANITIZER))
    add_subdirectory(micro-benchmarks)
endif()

function(panda_ets_napi_add_gtest)
    if (CMAKE_CROSSCOMPILING)
        return()
    endif()
    set(prefix ARG)
    set(noValues MULTI_CORES)
    set(singleValues NAME CPP_SOURCE INCLUDE_DIRS TSAN_EXTRA_OPTIONS)
    set(multiValues ETS_SOURCE)
    cmake_parse_arguments(${prefix}
                        "${noValues}"
                        "${singleValues}"
                        "${multiValues}"
                        ${ARGN})

    set(PANDA_STD_LIB "${PANDA_BINARY_ROOT}/plugins/ets/etsstdlib.abc")
    set(ARG_NAME ets_test_${ARG_NAME})

    add_custom_target(ets_gtests_sources_dependencies_${ARG_NAME})
    panda_ets_add_gtest(
        NO_CORES
        NAME ${ARG_NAME}
        SOURCES ${ARG_CPP_SOURCE}
        INCLUDE_DIRS ${ARG_INCLUDE_DIRS}
        LIBRARIES arkruntime arkassembler
        SANITIZERS ${PANDA_SANITIZERS_LIST}
        OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        TSAN_EXTRA_OPTIONS ${ARG_TSAN_EXTRA_OPTIONS}
        DEPS_TARGETS ets_gtests_sources_dependencies_${ARG_NAME}
        PANDA_STD_LIB ${PANDA_STD_LIB}
    )

    add_dependencies(ets_gtests ${ARG_NAME}_gtests)

    set(es2panda_target_test es2panda)
    set(es2panda_bin  $<TARGET_FILE:${es2panda_target_test}>)
    set(ETS_COMPILE_ARGUMENTS --gen-stdlib=false --extension=sts --opt-level=0)

    foreach(c ${ARG_ETS_SOURCE})
        STRING(REPLACE ".sts" "" CLEAR_NAME ${c})
        if(NOT TARGET es2panda_${CLEAR_NAME})
            add_custom_target(es2panda_${CLEAR_NAME}
                COMMAND ${es2panda_bin} ${ETS_COMPILE_ARGUMENTS} --output ${CMAKE_CURRENT_BINARY_DIR}/${CLEAR_NAME}.abc ${CMAKE_CURRENT_SOURCE_DIR}/${c}
                WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                DEPENDS es2panda
            )
        endif()
        add_dependencies(ets_gtests_sources_dependencies_${ARG_NAME} es2panda_${CLEAR_NAME})
    endforeach()

    add_dependencies(${ARG_NAME} etsstdlib)
    add_dependencies(ets_common_gtests ${ARG_NAME})
endfunction()

add_custom_target(ets_common_gtests)
add_dependencies(ets_gtests ets_common_gtests)
add_subdirectory(mock)
add_subdirectory(native)

if(PANDA_COMPILER_ENABLE)
    add_subdirectory(checked)
    add_dependencies(ets_tests ets_checked_tests)
endif()


if (PANDA_ETS_INTEROP_JS)
    add_subdirectory(interop_js)
endif()

add_subdirectory(debugger)
