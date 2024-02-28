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


set(CODEGEN_VISITORS_INC ${PANDA_BINARY_ROOT}/bytecode_optimizer/generated/codegen_visitors.inc)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    API ${PANDA_ROOT}/templates/plugin_options.rb
    TEMPLATE ${PANDA_ROOT}/bytecode_optimizer/templates/codegen_visitors.inc.erb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${CODEGEN_VISITORS_INC}
)

add_custom_target(bytecode_optimizer_codegen_visitors_inc DEPENDS
    plugin_options_gen
    ${CODEGEN_VISITORS_INC}
)

set(REG_ENCODER_VISITORS_INC ${PANDA_BINARY_ROOT}/bytecode_optimizer/generated/reg_encoder_visitors.inc)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    API ${PANDA_ROOT}/templates/plugin_options.rb
    TEMPLATE ${PANDA_ROOT}/bytecode_optimizer/templates/reg_encoder_visitors.inc.erb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${REG_ENCODER_VISITORS_INC}
)

add_custom_target(bytecode_optimizer_reg_encoder_visitors_inc DEPENDS
    plugin_options_gen
    ${REG_ENCODER_VISITORS_INC}
)

set(CODEGEN_INTRINSICS_CPP ${PANDA_BINARY_ROOT}/bytecode_optimizer/generated/codegen_intrinsics.cpp)
panda_gen_file(
    DATA ${GEN_PLUGIN_OPTIONS_YAML}
    API ${PANDA_ROOT}/templates/plugin_options.rb
    TEMPLATE ${PANDA_ROOT}/bytecode_optimizer/templates/codegen_intrinsics.cpp.erb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${CODEGEN_INTRINSICS_CPP}
)

add_custom_target(bytecode_optimizer_codegen_intrinsics_cpp DEPENDS
    plugin_options_gen
    ${CODEGEN_INTRINSICS_CPP}
)

panda_gen(
    DATA
        ${PANDA_BINARY_ROOT}/runtime/intrinsics.yaml
        ${PANDA_BINARY_ROOT}/plugin_options.yaml
    API
        ${PANDA_ROOT}/runtime/templates/intrinsics.rb
        ${PANDA_ROOT}/templates/plugin_options.rb
    TEMPLATES codegen_call_intrinsics.inc.erb
    SOURCE ${PANDA_ROOT}/bytecode_optimizer/templates
    EXTRA_DEPENDENCIES arkruntime_gen_intrinsics_yaml plugin_options_merge
    TARGET_NAME bytecode_optimizer_codegen_call_intrinsics_inc
    DESTINATION ${PANDA_BINARY_ROOT}/bytecode_optimizer/generated
)

panda_gen(
    DATA ${PANDA_BINARY_ROOT}/runtime/intrinsics.yaml
    API ${PANDA_ROOT}/runtime/templates/intrinsics.rb
    TEMPLATES get_intrinsic_id.inc.erb
    SOURCE ${PANDA_ROOT}/bytecode_optimizer/templates
    EXTRA_DEPENDENCIES arkruntime_gen_intrinsics_yaml
    TARGET_NAME bytecode_optimizer_get_intrinsic_id_inc
    DESTINATION ${PANDA_BINARY_ROOT}/bytecode_optimizer/generated
)

add_dependencies(arkbytecodeopt_obj
    bytecode_optimizer_codegen_visitors_inc
    bytecode_optimizer_reg_encoder_visitors_inc
    bytecode_optimizer_codegen_intrinsics_cpp
    bytecode_optimizer_codegen_call_intrinsics_inc
    bytecode_optimizer_get_intrinsic_id_inc
)

add_dependencies(panda_gen_files
    bytecode_optimizer_codegen_visitors_inc
    bytecode_optimizer_reg_encoder_visitors_inc
    bytecode_optimizer_codegen_intrinsics_cpp
)
