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

cmake_minimum_required(VERSION 3.5.2 FATAL_ERROR)
project(plugin_ets)

message(STATUS "eTS plugin is found")

include(cmake/import.cmake)

add_custom_target(ets_tests COMMENT "Running ets test suites")

function(compile_ets_code ETS_SRC OUTPUT_ABC TARGET)
    set(oneValueArgs OPT_LEVEL)
    set(multiValueArgs COMPILER_EXTRA_OPTIONS)
    cmake_parse_arguments(ARG "" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (NOT DEFINED ARG_OPT_LEVEL)
        set(ARG_OPT_LEVEL 2)
    endif()

    set(ES2PANDA_ARGUMENTS
        --opt-level=${ARG_OPT_LEVEL}
        --thread=0
        --extension=sts
        --output=${OUTPUT_ABC}
        ${ARG_COMPILER_EXTRA_OPTIONS}
    )
    set(COMPILE_DEPENDENCIES
        ${es2panda_target}
    )

    get_filename_component(OUTPUT_DIR ${OUTPUT_ABC} DIRECTORY)
    file(MAKE_DIRECTORY ${OUTPUT_DIR})

    if ("${ETS_SRC}" STREQUAL "GEN_STD_LIB")
        list(APPEND ES2PANDA_ARGUMENTS "--gen-stdlib=true")
        set(STAMP "${OUTPUT_ABC}.stamp")
        set(STDLIB ${CMAKE_CURRENT_LIST_DIR}/stdlib/)
        file(WRITE ${STAMP} "")
        add_custom_target(${TARGET}.stamp
            BYPRODUCTS ${STAMP}
            COMMAND find ${STDLIB} -newer ${STAMP} -exec touch ${STAMP} ";" || touch ${STAMP}
            VERBATIM
        )
        list(APPEND COMPILE_DEPENDENCIES ${TARGET}.stamp ${STAMP})
    else()
        list(APPEND ES2PANDA_ARGUMENTS ${ETS_SRC})
        list(APPEND COMPILE_DEPENDENCIES etsstdlib ${ETS_SRC})
    endif()

    add_custom_command(
        OUTPUT ${OUTPUT_ABC}
        COMMENT "Running es2panda for sts file: ${ETS_SRC}"
        COMMAND ${es2panda_bin} ${ES2PANDA_ARGUMENTS}
        DEPENDS ${COMPILE_DEPENDENCIES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

    add_custom_target(${TARGET} DEPENDS ${OUTPUT_ABC})

    if(TARGET etsstdlib)
        if (NOT TARGET regenerate_and_check_stdlib AND ${CMAKE_GENERATOR} MATCHES "Ninja")
            # Note(ivan-tyulyanin): -P replaces CMAKE_CURRENT_SOURCE_DIR so using ETS_PLUGIN_SOURCE_DIR
            add_custom_target(regenerate_and_check_stdlib
                COMMAND ${CMAKE_COMMAND} -DETS_PLUGIN_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/GenerateStdLib.cmake)
            add_dependencies(etsstdlib regenerate_and_check_stdlib)
        endif()
    endif()

endfunction()


# NB! ADDING THIS PROPERTY IS ALLOWED ONLY IN SPECIAL CASES. DO NOT COPY-PASTE IT.
set_target_properties(ets_tests PROPERTIES first-level-tests-dependency TRUE)
add_dependencies(tests ets_tests)

# List for accumulation of all ets gtests binary paths.
# It's used by CI to archive these binaries into a single artifact
# and send it to second stage where unit tests will use them.
set_property(GLOBAL PROPERTY ets_stash_list "")

if(TARGET es2panda_tests)
    if(NOT PANDA_ENABLE_THREAD_SANITIZER OR PANDA_CI_TESTING_MODE STREQUAL "Nightly")
        add_dependencies(ets_tests es2panda_tests)
    endif()
endif()

if(PANDA_WITH_RUNTIME)
    add_subdirectory(runtime)
endif()

if(PANDA_WITH_TOOLCHAIN)
    add_subdirectory(assembler)
    add_subdirectory(abc2program)
    add_subdirectory(disassembler)
    add_subdirectory(isa)
endif()

add_subdirectory(bytecode_optimizer)
add_subdirectory(irtoc_scripts)
add_subdirectory(verification)
add_subdirectory(tools)

add_subdirectory(arkts_header)

# NOTE(nsizov): change this logic when ets compiler moved into ets plugin
if(TARGET es2panda)
    compile_ets_code(GEN_STD_LIB "${CMAKE_CURRENT_BINARY_DIR}/etsstdlib.abc" etsstdlib)

    set_target_properties(etsstdlib PROPERTIES FILE "${CMAKE_CURRENT_BINARY_DIR}/etsstdlib.abc")
    add_dependencies(ark etsstdlib)
else()
    message(WARNING "es2panda was not found for etsstdlib build")
endif()

if(PANDA_WITH_COMPILER)
    add_subdirectory(compiler)
    set (COMPILER_OPTIONS "--compiler-check-final=true")
    add_custom_target(ets-compile-stdlib-default
        COMMENT "Running ark_aot compilation for etsstdlib with default options"
        COMMAND ${CMAKE_SOURCE_DIR}/plugins/ets/compiler/tools/paoc_compile_stdlib.sh --prefix="${PANDA_RUN_PREFIX}" --binary-dir=${CMAKE_BINARY_DIR} -compiler-options="${COMPILER_OPTIONS}" -paoc-output=${CMAKE_CURRENT_BINARY_DIR}/etsstdlib.an
        DEPENDS ark_aot etsstdlib
    )
endif()

if(PANDA_WITH_TESTS)
    add_subdirectory(tests)
endif()

add_plugin_options(${CMAKE_CURRENT_SOURCE_DIR}/ets_plugin_options.yaml)

add_runtime_options(${CMAKE_CURRENT_SOURCE_DIR}/runtime_options.yaml)

add_entrypoints_yaml(${CMAKE_CURRENT_SOURCE_DIR}/runtime/ets_entrypoints.yaml)

# ----- Collecting ets gtests paths for CI stash ---------------------------------
# NB! This must be the last section!

# Write to a file the list of all binaries
get_property(stash_files GLOBAL PROPERTY ets_stash_list)
list(JOIN stash_files "\n" file_content)
file(WRITE ${PANDA_BINARY_ROOT}/ets_stash_files.txt "${file_content}\n")
