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

set(INTRINSICS_STUB_INL ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_stub.inl)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_stub.inl.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_STUB_INL}
)

set(INTRINSICS_STUB_INL_H ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_stub.inl.h)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_stub.inl.h.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_STUB_INL_H}
)

set(INTRINSICS_CODEGEN_EXT_INL_H ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_codegen_ext.inl.h)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_codegen_ext.inl.h.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_CODEGEN_EXT_INL_H}
)

set(INTRINSICS_IR_BUILD_STATIC_CALL_INL ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_ir_build_static_call.inl)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_ir_build_static_call.inl.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_IR_BUILD_STATIC_CALL_INL}
)

set(INTRINSICS_IR_BUILD_VIRTUAL_CALL_INL ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_ir_build_virtual_call.inl)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_ir_build_virtual_call.inl.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_IR_BUILD_VIRTUAL_CALL_INL}
)

set(INTRINSICS_IR_BUILD_INL_H ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_ir_build.inl.h)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_ir_build.inl.h.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_IR_BUILD_INL_H}
)

set(INTRINSICS_CAN_ENCODE_INL ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_can_encode.inl)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_can_encode.inl.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_CAN_ENCODE_INL}
)

set(IR_DYN_BASE_TYPES_H ${PANDA_BINARY_ROOT}/compiler/generated/ir-dyn-base-types.h)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/ir-dyn-base-types.h.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES}
    OUTPUTFILE ${IR_DYN_BASE_TYPES_H}
)

add_custom_target(ir_dyn_base_types_h DEPENDS ${IR_DYN_BASE_TYPES_H})

set(SOURCE_LANGUAGES_H ${PANDA_BINARY_ROOT}/compiler/generated/source_languages.h)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/source_languages.h.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES}
    OUTPUTFILE ${SOURCE_LANGUAGES_H}
)

add_custom_target(source_languages_h DEPENDS ${SOURCE_LANGUAGES_H})

set(CODEGEN_LANGUAGE_EXTENSIONS_H ${PANDA_BINARY_ROOT}/compiler/generated/codegen_language_extensions.h)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/codegen_language_extensions.h.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES}
    OUTPUTFILE ${CODEGEN_LANGUAGE_EXTENSIONS_H}
)

set(COMPILER_INTERFACE_EXTENSIONS_H ${PANDA_BINARY_ROOT}/compiler/generated/compiler_interface_extensions.inl.h)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/compiler_interface_extensions.inl.h.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES}
    OUTPUTFILE ${COMPILER_INTERFACE_EXTENSIONS_H}
)

set(INST_BUILDER_EXTENSIONS_H ${PANDA_BINARY_ROOT}/compiler/generated/inst_builder_extensions.inl.h)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/inst_builder_extensions.inl.h.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES}
    OUTPUTFILE ${INST_BUILDER_EXTENSIONS_H}
)

set(INTRINSICS_EXTENSIONS_H ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_extensions.inl.h)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics_extensions.inl.h.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES ${YAML_FILES}
    OUTPUTFILE ${INTRINSICS_EXTENSIONS_H}
)

set(INTRINSICS_INLINE_INL ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_inline.inl)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_inline.inl.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_INLINE_INL}
)

set(INTRINSICS_TYPES_RESOLVING_INL_H ${PANDA_BINARY_ROOT}/compiler/generated/intrinsics_types_resolving.inl.h)
panda_gen_file(
    DATAFILE ${GEN_PLUGIN_OPTIONS_YAML}
    TEMPLATE ${PANDA_ROOT}/compiler/optimizer/templates/intrinsics/intrinsics_types_resolving.inl.h.erb
    REQUIRES ${PANDA_ROOT}/templates/plugin_options.rb
    EXTRA_DEPENDENCIES plugin_options_merge
    OUTPUTFILE ${INTRINSICS_TYPES_RESOLVING_INL_H}
)

add_custom_target(compiler_intrinsics DEPENDS
    plugin_options_gen
    ${INTRINSICS_STUB_INL}
    ${INTRINSICS_STUB_INL_H}
    ${INTRINSICS_CODEGEN_EXT_INL_H}
    ${INTRINSICS_IR_BUILD_STATIC_CALL_INL}
    ${INTRINSICS_IR_BUILD_VIRTUAL_CALL_INL}
    ${INTRINSICS_IR_BUILD_INL_H}
    ${INTRINSICS_CAN_ENCODE_INL}
    ${IR_DYN_BASE_TYPES_H}
    ${SOURCE_LANGUAGES_H}
    ${CODEGEN_LANGUAGE_EXTENSIONS_H}
    ${COMPILER_INTERFACE_EXTENSIONS_H}
    ${INST_BUILDER_EXTENSIONS_H}
    ${INTRINSICS_EXTENSIONS_H}
    ${INTRINSICS_INLINE_INL}
    ${INTRINSICS_TYPES_RESOLVING_INL_H}
)

add_dependencies(arkcompiler compiler_intrinsics)
