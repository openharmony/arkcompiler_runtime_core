/*
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

#include "abc_annotation_processor.h"
#include "abc2program_log.h"

namespace panda::abc2program {

AbcAnnotationProcessor::AbcAnnotationProcessor(panda_file::File::EntityId entity_id, const panda_file::File &abc_file,
                                               AbcStringTable &abc_string_table)
    : AbcFileEntityProcessor(entity_id, abc_file, abc_string_table)
{
    annotation_data_accessor_ = std::make_unique<panda_file::AnnotationDataAccessor>(abc_file_, entity_id_);
    FillUpProgramData();
}

void AbcAnnotationProcessor::FillUpProgramData()
{
    log::Unimplemented(__PRETTY_FUNCTION__);
}

} // namespace panda::abc2program
