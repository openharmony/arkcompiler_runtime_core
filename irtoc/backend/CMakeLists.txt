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

project(irtoc_backend)

###############################################################################
# irtoc: library
#
set(SOURCES
    compilation_unit.cpp
    compiler/codegen_boundary.cpp
    compiler/codegen_fastpath.cpp
)

add_library(irtoc SHARED ${SOURCES})

target_link_libraries(irtoc arkcompiler elfio arkbase)

add_dependencies(irtoc asm_defines_generator cross_values)

set_source_files_properties(compilation_unit.cpp PROPERTIES COMPILE_FLAGS -Wno-shadow)

target_include_directories(irtoc
    PUBLIC ${PANDA_BINARY_ROOT}/panda_gen_options
    PUBLIC ${PANDA_BINARY_ROOT}/cross_values
    PUBLIC ${PANDA_BINARY_ROOT}/runtime/include
    PUBLIC ${PANDA_THIRD_PARTY_SOURCES_DIR}/elfio
)

# Compile one or more irtoc script files into elf object file.
# Arguments:
#   TARGET_NAME - name of the target, that will generate object file. Thereby user can make dependency on it.
#   INPUT_FILES - input Irtoc scripts, usually it is files from `irtoc/scripts` folder.
#   TARGET_VARIABLE - name of a variable, which will hold name of the generated object file, so the user can link it to
#                     the target's sources.
function(irtoc_compile)
    set(noValues)
    set(singleValueArgs TARGET_NAME TARGET_VARIABLE)
    set(multiValueArgs INPUT_FILES)
    cmake_parse_arguments(ARG "${noValues}" "${singleValueArgs}" "${multiValueArgs}" ${ARGN})

    set(BUILD_DIR "${IRTOC_BUILD_DIR}/${ARG_TARGET_NAME}")
    file(MAKE_DIRECTORY ${BUILD_DIR})

    set(IRTOC_GENERATOR_TARGET irtoc_generate_${ARG_TARGET_NAME})
    set(IRTOC_COMPILE_OUTPUT "${BUILD_DIR}/irtoc_code.cpp")
    irtoc_generate(
        TARGET ${IRTOC_GENERATOR_TARGET}
        IR_API ir-constructor
        INPUT_FILES "${ARG_INPUT_FILES}"
        OUTPUT_FILE ${IRTOC_COMPILE_OUTPUT}
        WORKING_DIRECTORY ${BUILD_DIR}
    )

    ###############################################################################
    #  Executable
    #
    set(SOURCES ${IRTOC_COMPILE_OUTPUT} ${IRTOC_SOURCE_DIR}/backend/irtoc.cpp)

    set(IRTOC_EXEC ${ARG_TARGET_NAME}_exec)

    panda_add_executable(${IRTOC_EXEC} ${SOURCES})
    add_dependencies(${IRTOC_EXEC} ${IRTOC_GENERATOR_TARGET})

    target_link_libraries(${IRTOC_EXEC} irtoc)

    panda_add_sanitizers(TARGET ${IRTOC_EXEC} SANITIZERS ${PANDA_SANITIZERS_LIST})

    ###############################################################################
    #  final target: run irtoc tool, that will generate resulting object file
    #
    set(IRTOC_OUTPUT ${BUILD_DIR}/${ARG_TARGET_NAME}.o)

    # Irtoc validation is enabled only in pure Release build
    if (CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT IRTOC_WITH_SANITIZERS)
        set(VALIDATION_COMMAND ruby ${IRTOC_SOURCE_DIR}/lang/validation.rb ${BUILD_DIR}/validation.yaml ${BUILD_DIR}/disasm.txt)
    endif()

    add_custom_command(OUTPUT ${IRTOC_OUTPUT}
        COMMENT             "Building ${IRTOC_OUTPUT}"
        COMMAND             ${PANDA_RUN_PREFIX} ${IRTOC_EXEC}
                                --irtoc-output ${IRTOC_OUTPUT}
                                --compiler-cross-arch ${IRTOC_TARGET}
                                --compiler-disasm-dump:single-file
        COMMAND             ${VALIDATION_COMMAND}
        WORKING_DIRECTORY   ${BUILD_DIR}
        DEPENDS             ${IRTOC_EXEC}
    )

    add_custom_target(${ARG_TARGET_NAME} ALL
        DEPENDS ${IRTOC_OUTPUT} irtoc_plugins_txt
    )
    add_dependencies(${ARG_TARGET_NAME} ${IRTOC_EXEC})

    # Set output variable to the generated object file, so caller can link it to target's sources
    set(${ARG_TARGET_VARIABLE} ${IRTOC_OUTPUT} PARENT_SCOPE)
endfunction()

###############################################################################
#  Other stuff
#
panda_gen_options(TARGET irtoc YAML_FILE options.yaml GENERATED_HEADER irtoc_options_gen.h)

panda_add_sanitizers(TARGET irtoc SANITIZERS ${PANDA_SANITIZERS_LIST})

add_check_style(".")
panda_add_to_clang_tidy(TARGET irtoc)
