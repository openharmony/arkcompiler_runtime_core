/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "helpers.h"
#include "method_handle_data_accessor.h"

namespace ark::panda_file {

MethodHandleDataAccessor::MethodHandleDataAccessor(const File &pandaFile, File::EntityId methodHandleId)
    : pandaFile_(pandaFile), methodHandleId_(methodHandleId)
{
    auto sp = pandaFile_.GetSpanFromId(methodHandleId_);

    type_ = static_cast<MethodHandleType>(helpers::Read<sizeof(uint8_t)>(&sp));
    offset_ = helpers::ReadULeb128(&sp);

    size_ = pandaFile_.GetIdFromPointer(sp.data()).GetOffset() - methodHandleId_.GetOffset();
}

}  // namespace ark::panda_file
