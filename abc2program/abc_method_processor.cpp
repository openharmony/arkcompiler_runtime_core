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
    static const uint32_t MAX_ARG_NUM = 0xFFFF;
    panda_file::File::EntityId proto_id = method_data_accessor_->GetProtoId();
    LOG(DEBUG, ABC2PROGRAM) << "[getting params]\nproto id: " << proto_id << " (0x" << std::hex << proto_id << ")";
    panda_file::ProtoDataAccessor proto_accessor(*file_, proto_id);
    auto params_num = proto_accessor.GetNumArgs();
    if (params_num > MAX_ARG_NUM) {
        LOG(ERROR, ABC2PROGRAM) << "> error encountered at " << proto_id << " (0x" << std::hex << proto_id
                                << "). number of function's arguments (" << std::dec << params_num
                                << ") exceeds MAX_ARG_NUM (" << MAX_ARG_NUM << ") !";

        return;
    }
    size_t refIdx = 0;
    function_.return_type = type_converter_.PandaFileTypeToPandasmType(proto_accessor.GetReturnType(), proto_accessor,
                                                                       refIdx);
    for (uint8_t i = 0; i < params_num; i++) {
        pandasm::Type argType = type_converter_.PandaFileTypeToPandasmType(proto_accessor.GetArgType(i), proto_accessor,
                                                                           refIdx);
        function_.params.emplace_back(pandasm::Function::Parameter(argType, lang));
    }
}

void AbcMethodProcessor::FillFunctionMetaData()
{
    if (method_data_accessor_->IsStatic()) {
        function_.metadata->SetAttribute("static");
    }
    if (file_->IsExternal(method_data_accessor_->GetMethodId())) {
        function_.metadata->SetAttribute("external");
    }
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
