/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

namespace panda::abc2program {

AbcMethodProcessor::AbcMethodProcessor(panda_file::File::EntityId entity_id, const panda_file::File &abc_file,
                                       AbcStringTable &abc_string_table)
    : AbcFileEntityProcessor(entity_id, abc_file, abc_string_table),
      function_(pandasm::Function("", panda_file::SourceLang::PANDA_ASSEMBLY))
{
    method_data_accessor_ = std::make_unique<panda_file::MethodDataAccessor>(abc_file_, entity_id_);
    FillUpProgramData();
}

void AbcMethodProcessor::FillUpProgramData()
{
    FillUpFunction();
}

void AbcMethodProcessor::FillUpFunction()
{
    std::optional<panda_file::File::EntityId> code_id = method_data_accessor_->GetCodeId();
    if (code_id.has_value()) {
        AbcCodeProcessor codeProcessor(code_id.value(), abc_file_, abc_string_table_, entity_id_);
    }
}

} // namespace panda::abc2program
