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

find_program(
    LCOV
    NAMES "lcov"
    DOC "Path to lcov executable")
if(NOT LCOV)
    message(SEND_ERROR "lcov not found, please install")
endif()

find_program(
    GENHTML
    NAMES "genhtml"
    DOC "Path to genhtml executable")
if(NOT GENHTML)
    message(SEND_ERROR "genhtml not found, please install")
endif()

if(ENABLE_UNIT_TESTS_FULL_COVERAGE
    OR ENABLE_BYTECODE_OPTIMIZER_COVERAGE
    OR ENABLE_COMPILER_COVERAGE
    OR ENABLE_LIBPANDABASE_COVERAGE
    OR ENABLE_ES2PANDA_COVERAGE
    AND NOT IS_USED_COMMON_COVERAGE_TOOLCHAIN)

    set(USE_GCOV_TOOLS true)
    set(COVERAGE_FLAGS "-fprofile-arcs -ftest-coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COVERAGE_FLAGS}")
endif()

set(LCOV_FLAGS --quiet --rc lcov_branch_coverage=1)
set(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

if(ENABLE_UNIT_TESTS_FULL_COVERAGE)
    set(ENABLE_BYTECODE_OPTIMIZER_COVERAGE true)
    set(ENABLE_COMPILER_COVERAGE true)
endif()

function(collect_coverage_for_target)
    set(prefix ARG)
    set(oneValueArgs TARGET_NAME)
    set(multiValueArgs INCLUDE_DIR_PATTERN EXCLUDE_DIR_PATTERN)
    cmake_parse_arguments(${prefix} "${novalue}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(HTML_REPORT_DIRECTORY ${PANDA_BINARY_ROOT}/coverage_reports/${ARG_TARGET_NAME})
    set(COVERAGE_INTERMEDIATE_REPORT ${HTML_REPORT_DIRECTORY}/${ARG_TARGET_NAME}.info)

    if(ARG_INCLUDE_DIR_PATTERN)
        message(STATUS "INCLUDE_DIR_PATTERN=${ARG_INCLUDE_DIR_PATTERN}")
        set(EXTRACT_DATA_FROM_TRACEFILE COMMAND lcov --extract ${COVERAGE_INTERMEDIATE_REPORT} -o ${COVERAGE_INTERMEDIATE_REPORT} ${ARG_INCLUDE_DIR_PATTERN})
    endif()
    
    if(ARG_EXCLUDE_DIR_PATTERN)
        message(STATUS "EXCLUDE_DIR_PATTERN=${ARG_EXCLUDE_DIR_PATTERN}")
        set(REMOVE_DATA_FROM_TRACEFILE COMMAND lcov --remove ${COVERAGE_INTERMEDIATE_REPORT} -o ${COVERAGE_INTERMEDIATE_REPORT} ${ARG_EXCLUDE_DIR_PATTERN})
    endif()

    add_custom_command(
        TARGET ${ARG_TARGET_NAME} POST_BUILD
        WORKING_DIRECTORY ${PANDA_BINARY_ROOT}
        COMMAND mkdir -p ${HTML_REPORT_DIRECTORY}
        COMMAND lcov -c -b ${PANDA_BINARY_ROOT} -d ${DIRECTORY} -o ${COVERAGE_INTERMEDIATE_REPORT} ${LCOV_FLAGS}
        ${EXTRACT_DATA_FROM_TRACEFILE}
        ${REMOVE_DATA_FROM_TRACEFILE}
        COMMAND genhtml -o ${HTML_REPORT_DIRECTORY} ${COVERAGE_INTERMEDIATE_REPORT} --ignore-errors source ${LCOV_FLAGS}
        COMMAND echo "Coverage report: ${HTML_REPORT_DIRECTORY}/index.html"
        COMMAND lcov --zerocounters --directory ${PANDA_BINARY_ROOT}
    )
endfunction()
