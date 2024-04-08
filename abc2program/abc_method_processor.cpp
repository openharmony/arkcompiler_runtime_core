/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "abc_method_processor.h"
#include "abc_code_processor.h"
#include "method_data_accessor-inl.h"

namespace panda::abc2program {

AbcMethodProcessor::AbcMethodProcessor(panda_file::File::EntityId entity_id,
                                       Abc2ProgramEntityContainer &entity_container)
    : AbcFileEntityProcessor(entity_id, entity_container),
      type_converter_(AbcTypeConverter(*string_table_)),
      function_(pandasm::Function(entity_container_.GetFullMethodNameById(entity_id_),
                                  panda_file::SourceLang::ECMASCRIPT))
{
    method_data_accessor_ = std::make_unique<panda_file::MethodDataAccessor>(*file_, entity_id_);
}

void AbcMethodProcessor::FillProgramData()
{
    FillFunctionData();
    AddFunctionIntoFunctionTable();
}

void AbcMethodProcessor::FillFunctionData()
{
    FillProto();
    FillFunctionKind();
    FillFunctionMetaData();
    FillCodeData();
}

void AbcMethodProcessor::AddFunctionIntoFunctionTable()
{
    std::string full_method_name = entity_container_.GetFullMethodNameById(entity_id_);
    program_->function_table.emplace(full_method_name, std::move(function_));
}

void AbcMethodProcessor::FillProto()
{
    uint32_t params_num = GetNumArgs();
    pandasm::Type any_type = pandasm::Type(ANY_TYPE_NAME, 0);
    function_.return_type = any_type;
    for (uint8_t i = 0; i < params_num; i++) {
        function_.params.emplace_back(pandasm::Function::Parameter(any_type, lang));
    }
}

uint32_t AbcMethodProcessor::GetNumArgs() const
{
    std::optional<panda_file::File::EntityId> code_id = method_data_accessor_->GetCodeId();
    if (!code_id.has_value()) {
        return 0;
    }
    panda_file::File::EntityId code_id_value = code_id.value();
    panda_file::CodeDataAccessor code_data_accessor(*file_, code_id_value);
    return code_data_accessor.GetNumArgs();
}

void AbcMethodProcessor::FillFunctionKind()
{
    uint32_t method_acc_flags = method_data_accessor_->GetAccessFlags();
    uint32_t function_kind_u32 = ((method_acc_flags & MASK_9_TO_16_BITS) >> BITS_8_SHIFT);
    panda_file::FunctionKind function_kind = static_cast<panda_file::FunctionKind>(function_kind_u32);
    function_.SetFunctionKind(function_kind);
}

void AbcMethodProcessor::FillFunctionMetaData()
{
    if (method_data_accessor_->IsStatic()) {
        function_.metadata->SetAttribute("static");
    }
    if (file_->IsExternal(method_data_accessor_->GetMethodId())) {
        function_.metadata->SetAttribute("external");
    }
    FillAccessFlags();
}

void AbcMethodProcessor::FillAccessFlags()
{
    uint32_t access_flags = method_data_accessor_->GetAccessFlags();
    access_flags = (access_flags & LOWEST_8_BIT_MASK);
    function_.metadata->SetAccessFlags(access_flags);
}

void AbcMethodProcessor::FillCodeData()
{
    std::optional<panda_file::File::EntityId> code_id = method_data_accessor_->GetCodeId();
    if (code_id.has_value()) {
        AbcCodeProcessor code_processor(code_id.value(), entity_container_, entity_id_, function_);
        code_processor.FillProgramData();
    }
}

} // namespace panda::abc2program
