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

set(EXTENSION_DIR ${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/assembler/extension)

target_sources(arkassembler PRIVATE ${EXTENSION_DIR}/ecmascript_meta.cpp)
target_include_directories(arkassembler PUBLIC ${EXTENSION_DIR})

set(ECMASCRIPT_META_GEN_H ${PANDA_BINARY_ROOT}/assembler/ecmascript_meta_gen.h)
panda_gen_file(
        DATAFILE ${PANDA_ECMASCRIPT_PLUGIN_SOURCE}/assembler/extension/metadata.yaml
        TEMPLATE ${PANDA_ROOT}/assembler/templates/meta_gen.cpp.erb
        OUTPUTFILE ${ECMASCRIPT_META_GEN_H}
        REQUIRES ${PANDA_ROOT}/assembler/asm_metadata.rb
)
add_custom_target(ecmascript_meta_gen_h DEPENDS ${ECMASCRIPT_META_GEN_H})
add_dependencies(arkassembler ecmascript_meta_gen_h)

if (PANDA_ENABLE_AFL)
    target_sources(arkassembler_fuzz PRIVATE ${EXTENSION_DIR}/ecmascript_meta.cpp)
    target_include_directories(arkassembler_fuzz PUBLIC ${EXTENSION_DIR})
    add_dependencies(arkassembler_fuzz ecmascript_meta_gen_h)
endif()
