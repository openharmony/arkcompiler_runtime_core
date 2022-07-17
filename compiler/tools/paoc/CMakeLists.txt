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

project(ark_aot)

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${GENERATED_DIR})

set(arkcompiler_OPTIONS_H ${GENERATED_DIR}/paoc_options.h)
panda_gen_file(
        DATAFILE ${CMAKE_CURRENT_LIST_DIR}/paoc.yaml
        TEMPLATE ${PANDA_ROOT}/templates/options/options.h.erb
        OUTPUTFILE ${arkcompiler_OPTIONS_H}
        REQUIRES ${PANDA_ROOT}/templates/common.rb
)
add_custom_target(arkcompiler_options DEPENDS ${arkcompiler_OPTIONS_H})

panda_add_executable(ark_aot
    paoc.cpp
)

add_dependencies(ark_aot arkcompiler_options)

target_link_libraries(ark_aot arkbase arkruntime aot_builder)

target_include_directories(ark_aot
    PUBLIC ${GENERATED_DIR}
    PUBLIC ${PANDA_ROOT}/compiler/aot
    PUBLIC ${PANDA_BINARY_ROOT}
)

panda_add_sanitizers(TARGET ark_aot SANITIZERS ${PANDA_SANITIZERS_LIST})

# TODO: remove after all components will use ark_aot instead of paoc
add_custom_target(paoc ALL
                  COMMAND cd $<TARGET_FILE_DIR:ark_aot> && ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE_NAME:ark_aot> paoc)

add_dependencies(paoc ark_aot)

add_check_style(".")
panda_add_to_clang_tidy(TARGET ark_aot)
