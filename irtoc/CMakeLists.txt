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

project(irtoc)

if (TARGET arkcompiler)
    message(FATAL_ERROR "IRtoC core and front-end must be configured before defining target arkcompiler")
endif()

# These variables will be used in the IRtoC frontend and backend.
# Since it will be invoked from arbitrary folder,
# we need to use these variables to determine irtoc folders.

#
# IRtoC sources:
#

set(IRTOC_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(IRTOC_SOURCE_DIR ${IRTOC_SOURCE_DIR} PARENT_SCOPE)

#
# IRtoC build directory:
#

set(IRTOC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(IRTOC_BUILD_DIR ${IRTOC_BUILD_DIR} PARENT_SCOPE)

#
# IRtoC target:
#

if (NOT IRTOC_TARGET)
    if (PANDA_TARGET_AMD64)
        set(IRTOC_TARGET x86_64)
    elseif(PANDA_TARGET_ARM64)
        set(IRTOC_TARGET arm64)
    elseif(PANDA_TARGET_ARM32)
        set(IRTOC_TARGET arm32)
    else()
        message(FATAL_ERROR "Unsupported IRtoC target")
    endif()
endif()
set(IRTOC_TARGET ${IRTOC_TARGET} PARENT_SCOPE)

#
# IRtoC front-end:
#

set(IRTOC_LANG_SOURCES
    ${IRTOC_SOURCE_DIR}/lang/basic_block.rb
    ${IRTOC_SOURCE_DIR}/lang/cpp_function.rb
    ${IRTOC_SOURCE_DIR}/lang/function.rb
    ${IRTOC_SOURCE_DIR}/lang/ir_generator.rb
    ${IRTOC_SOURCE_DIR}/lang/instruction.rb
    ${IRTOC_SOURCE_DIR}/lang/instructions_data.rb
    ${IRTOC_SOURCE_DIR}/lang/irtoc.rb
    ${IRTOC_SOURCE_DIR}/lang/isa.rb
    ${IRTOC_SOURCE_DIR}/lang/options.rb
    ${IRTOC_SOURCE_DIR}/lang/output.rb
    ${IRTOC_SOURCE_DIR}/lang/regmap.rb
    ${IRTOC_SOURCE_DIR}/lang/regmask.rb
    ${IRTOC_SOURCE_DIR}/lang/validation.rb
)
set(IRTOC_LANG_SOURCES ${IRTOC_LANG_SOURCES} PARENT_SCOPE) # To be available in irtoc_generate

# Unit tests for IRtoC frontend. Available only in x86 mode, since there is no need to test in different targets.
if (PANDA_WITH_TESTS AND PANDA_TARGET_AMD64)
    add_custom_target(irtoc_unit_tests COMMENT "Irtoc frontend unit tests"
        COMMAND ruby ${IRTOC_SOURCE_DIR}/lang/tests/regmask_test.rb
        DEPENDS ${IRTOC_LANG_SOURCES}
                ${IRTOC_SOURCE_DIR}/lang/tests/regmask_test.rb)
    add_dependencies(tests irtoc_unit_tests)
endif()

# Run IRtoC front-end to generate C++ code from IRtoC sources.
# Arguments:
#   [in] TARGET            - name of the target that will generate C++ code. Thereby user can make a dependency on it.
#   [in] INPUT_FILES       - input IRtoC scripts, usually files from `irtoc/scripts` folder.
#   [in] OUTPUT_FILE       - name of the output C++ file.
#   [in] IR_API            - IR API to be emitted during C++ code generation. See IRtoC sources for possible supported values.
#   [in] WORKING_DIRECTORY - working directory for the front-end.
function(irtoc_generate)
    set(noValues)
    set(singleValueArgs TARGET OUTPUT_FILE IR_API WORKING_DIRECTORY)
    set(multiValueArgs INPUT_FILES)
    cmake_parse_arguments(ARG "${noValues}" "${singleValueArgs}" "${multiValueArgs}" ${ARGN})

    get_directory_property(irtoc_definitions_ DIRECTORY ${IRTOC_SOURCE_DIR} COMPILE_DEFINITIONS)

    if (CMAKE_BUILD_TYPE STREQUAL "Release")
        list(APPEND irtoc_definitions_ NDEBUG)
    endif()
    string(REGEX REPLACE ";" ":" IRTOC_DEFINITIONS "${irtoc_definitions_}")

    string(REPLACE ";" ":" IRTOC_INPUT "${ARG_INPUT_FILES}")

    set(ISAPI "${PANDA_ROOT}/isa/isapi.rb")
    set(ISA "${PANDA_BINARY_ROOT}/isa/isa.yaml")

    if (NOT DEFINED ARG_WORKING_DIRECTORY)
        set(ARG_WORKING_DIRECTORY ${IRTOC_BUILD_DIR})
    else()
        file(MAKE_DIRECTORY ${ARG_WORKING_DIRECTORY})
    endif()

    set(IRTOC_PLUGINS_FILE ${IRTOC_BUILD_DIR}/generated/plugins.txt)
    set(IRTOC_PLUGIN_FILES)
    if(EXISTS ${IRTOC_PLUGINS_FILE})
        file(STRINGS ${IRTOC_BUILD_DIR}/generated/plugins.txt PLUGINS_LIST)
        foreach(PLUGIN_FILE ${PLUGINS_LIST})
            list(APPEND IRTOC_PLUGIN_FILES "${PANDA_ROOT}/${PLUGIN_FILE}")
        endforeach()
    endif()

    add_custom_command(OUTPUT ${ARG_OUTPUT_FILE}
        COMMENT             "Run IRtoC generator for ${ARG_TARGET}"
        COMMAND             ruby ${IRTOC_SOURCE_DIR}/lang/irtoc.rb
                                --input ${IRTOC_INPUT}
                                --output ${ARG_OUTPUT_FILE}
                                --ir-api ${ARG_IR_API}
                                --ark_source_dir ${PANDA_ROOT}
                                --isa ${ISA}
                                --definitions ${IRTOC_DEFINITIONS}
                                --arch ${IRTOC_TARGET}
                                --plugins ${IRTOC_PLUGINS_FILE}
        WORKING_DIRECTORY   ${ARG_WORKING_DIRECTORY}
        DEPENDS             ${IRTOC_LANG_SOURCES} ${ARG_INPUT_FILES} ${ISA} ${ISAPI} ${IRTOC_SOURCE_DIR}/scripts/common.irt ${IRTOC_PLUGIN_FILES}
    )

    add_custom_target(${ARG_TARGET} DEPENDS ${ARG_OUTPUT_FILE} irtoc_plugins_txt)
    add_dependencies(${ARG_TARGET} isa_combine)
endfunction()
