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

cmake_minimum_required(VERSION 3.3.2 FATAL_ERROR)

panda_add_gtest(
    NAME static_linker_tests
    SOURCES
        linker_test.cpp
    LIBRARIES
        arklinker arkruntime arkassembler
    INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/assembler"
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
    TEST_RUN_DIR
        "${CMAKE_CURRENT_BINARY_DIR}"
    DEPS_TARGETS
        arkstdlib
)

file(COPY data DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

set(test_asm_files
    data/single/exceptions.pa
    data/single/lnp_dedup.pa
    data/single/lit_array.pa
    data/single/unresolved_global.pa
    data/single/hello_world.pa
    data/multi/bad_ffield_type/1.pa
    data/multi/bad_ffield_type/2.pa
    data/multi/fmethod_overloaded/1.pa
    data/multi/fmethod_overloaded/2.pa
    data/multi/ffield/1.pa
    data/multi/ffield/2.pa
    data/multi/ffield_overloaded/1.pa
    data/multi/ffield_overloaded/2.pa
    data/multi/fmethod/1.pa
    data/multi/fmethod/2.pa
    data/multi/bad_fmethod_name/1.pa
    data/multi/bad_fmethod_name/2.pa
    data/multi/bad_class_redefinition/1.pa
    data/multi/bad_class_redefinition/2.pa
    data/multi/dedup_field/1.pa
    data/multi/dedup_field/2.pa
    data/multi/dedup_method/1.pa
    data/multi/dedup_method/2.pa
    data/multi/bad_fmethod_overloaded/1.pa
    data/multi/bad_fmethod_overloaded/2.pa
    data/multi/bad_ffield/1.pa
    data/multi/bad_ffield/2.pa
    data/multi/fmethod_overloaded_2/4.pa
    data/multi/fmethod_overloaded_2/1.pa
    data/multi/fmethod_overloaded_2/2.pa
    data/multi/fmethod_overloaded_2/3.pa

    data/single/lit_array.gold
    data/single/exceptions.gold
    data/single/unresolved_global.gold
    data/single/hello_world.gold
    data/single/lnp_dedup.gold
    data/multi/fmethod_overloaded/out.gold
    data/multi/ffield/out.gold
    data/multi/ffield_overloaded/out.gold
    data/multi/fmethod/out.gold
    data/multi/dedup_field/out.gold
    data/multi/dedup_method/out.gold
    data/multi/fmethod_overloaded_2/out.gold
)

add_custom_target(static_linker_tests_gen_abc DEPENDS ${test_asm_files})
add_dependencies(static_linker_tests static_linker_tests_gen_abc arkruntime)

if (TARGET es2panda AND TARGET etsstdlib AND PANDA_WITH_ETS AND NOT CMAKE_CROSSCOMPILING)
    # internal issue #15674
    add_custom_target(static_linker_tests_field_value
        COMMAND $<TARGET_FILE:es2panda> --output "${CMAKE_CURRENT_BINARY_DIR}/a.abc" --ets-module --extension=sts "${CMAKE_CURRENT_SOURCE_DIR}/data/multi/field_value/a.sts"
        COMMAND $<TARGET_FILE:es2panda> --output "${CMAKE_CURRENT_BINARY_DIR}/b.abc" --ets-module --extension=sts "${CMAKE_CURRENT_SOURCE_DIR}/data/multi/field_value/b.sts"
        COMMAND $<TARGET_FILE:ark_link> --output "${CMAKE_CURRENT_BINARY_DIR}/l.abc" -- "${CMAKE_CURRENT_BINARY_DIR}/a.abc" "${CMAKE_CURRENT_BINARY_DIR}/b.abc"
        COMMAND $<TARGET_FILE:ark> --load-runtimes ets --boot-panda-files $<TARGET_PROPERTY:etsstdlib,FILE> "${CMAKE_CURRENT_BINARY_DIR}/l.abc" b/ETSGLOBAL::main
        DEPENDS  es2panda ark ark_link etsstdlib "${CMAKE_CURRENT_SOURCE_DIR}/data/multi/field_value/a.sts" "${CMAKE_CURRENT_SOURCE_DIR}/data/multi/field_value/b.sts"
    )

    # internal issue #17770
    add_custom_target(static_linker_tests_debug_info
        COMMAND $<TARGET_FILE:es2panda> --output "${CMAKE_CURRENT_BINARY_DIR}/CustomInt.abc"
            --ets-module --extension=sts --debug-info "${CMAKE_CURRENT_SOURCE_DIR}/data/multi/dedup_debug_info/CustomInt.sts"
        COMMAND $<TARGET_FILE:es2panda> --output "${CMAKE_CURRENT_BINARY_DIR}/CustomLong.abc"
            --ets-module --extension=sts --debug-info "${CMAKE_CURRENT_SOURCE_DIR}/data/multi/dedup_debug_info/CustomLong.sts"
        COMMAND $<TARGET_FILE:ark_link> --output "${CMAKE_CURRENT_BINARY_DIR}/CustomIntLongLinked.abc"
            -- "${CMAKE_CURRENT_BINARY_DIR}/CustomInt.abc" "${CMAKE_CURRENT_BINARY_DIR}/CustomLong.abc"
        COMMAND $<TARGET_FILE:ark> --load-runtimes ets --boot-panda-files $<TARGET_PROPERTY:etsstdlib,FILE>
            "${CMAKE_CURRENT_BINARY_DIR}/CustomIntLongLinked.abc" CustomLong/CustomLong::main   # Test execution
        COMMAND $<TARGET_FILE:ark_disasm> --verbose "${CMAKE_CURRENT_BINARY_DIR}/CustomIntLongLinked.abc"
            "${CMAKE_CURRENT_BINARY_DIR}/CustomIntLongLinked.disasm.pa"                         # Test disasm with debug info reading
        DEPENDS es2panda ark ark_disasm ark_link etsstdlib
            "${CMAKE_CURRENT_SOURCE_DIR}/data/multi/dedup_debug_info/CustomInt.sts"
            "${CMAKE_CURRENT_SOURCE_DIR}/data/multi/dedup_debug_info/CustomLong.sts"
    )
    add_dependencies(static_linker_tests static_linker_tests_field_value static_linker_tests_debug_info)
endif()
