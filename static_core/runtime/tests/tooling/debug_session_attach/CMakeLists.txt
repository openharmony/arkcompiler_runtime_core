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

panda_add_gtest(
    NO_CORES
    NAME debug_session_attach_test
    SOURCES
        debug_session_attach_test.cpp
    LIBRARIES
        arkruntime
        arkassembler
    SANITIZERS
        ${PANDA_SANITIZERS_LIST}
    DEPS_TARGETS
        arkstdlib
)

set(abc_file_name debug_session_attach_test)
add_panda_assembly(
    TARGET debug_session_attach_test_ark_asm
    SOURCE debug_session_attach_test.pa
    TARGETNAME ${abc_file_name}
)
add_dependencies(debug_session_attach_test_ark_asm ark_asm)

# Compile assembly file
set(an_file_name debug_session_attach_test.an)

set(paoc_mode aot)
if (PANDA_LLVM_AOT)
    set(paoc_mode llvm)
endif()

add_custom_command(
    OUTPUT ${an_file_name}
    COMMAND ${PANDA_RUN_PREFIX} $<TARGET_FILE:ark_aot>
        --paoc-panda-files=${abc_file_name}.abc
        --paoc-output=${an_file_name}
        --paoc-mode=${paoc_mode}
        --compiler-regex='.*::.*CompiledMethod'
    DEPENDS ark_aot debug_session_attach_test_ark_asm
)
add_custom_target(debug_session_attach_test_ark_aot DEPENDS ${an_file_name})
set_target_properties(debug_session_attach_test_ark_aot PROPERTIES FILE ${an_file_name})

add_dependencies(debug_session_attach_test_gtests debug_session_attach_test_ark_aot)
