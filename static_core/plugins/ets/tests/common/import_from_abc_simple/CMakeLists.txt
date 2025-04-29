# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

if (PANDA_TARGET_ARM32)
    return()
endif()

set(LIB_SOURCE_ETS ${CMAKE_CURRENT_SOURCE_DIR}/lib/lib.ets)
set(LIB_SOURCE_ABC ${CMAKE_CURRENT_BINARY_DIR}/lib.abc)
set(LIB_SOURCE_ABC_TARGET compile_lib_abc)

compile_ets_code(${LIB_SOURCE_ETS} ${LIB_SOURCE_ABC} ${LIB_SOURCE_ABC_TARGET}
                 COMPILER_EXTRA_OPTIONS "--emit-declaration")

set(ETS_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/arktsconfig.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/entry/arktsconfig.in.json ${ETS_CONFIG})

run_int_ets_code(import_from_abc ${CMAKE_CURRENT_BINARY_DIR}
                 SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/entry/entry.ets
                 ARKTS_CONFIGS ${ETS_CONFIG}
                 RUNTIME_EXTRA_OPTIONS --panda-files=${LIB_SOURCE_ABC})

add_dependencies(import_from_abc-cpp-abc-entry-ets-es2panda ${LIB_SOURCE_ABC_TARGET})
add_dependencies(import_from_abc-irtoc-abc-entry-ets-es2panda ${LIB_SOURCE_ABC_TARGET})

if (PANDA_LLVM_INTERPRETER)
    add_dependencies(import_from_abc-llvmirtoc-abc-entry-ets-es2panda ${LIB_SOURCE_ABC_TARGET})
endif()