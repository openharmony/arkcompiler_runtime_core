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
#include "method_data_accessor.h"

namespace ark::panda_file {

MethodDataAccessor::MethodDataAccessor(const File &pandaFile, File::EntityId methodId)
    : pandaFile_(pandaFile), methodId_(methodId)
{
    auto sp = pandaFile_.GetSpanFromId(methodId);

    classIdx_ = helpers::Read<IDX_SIZE>(&sp);
    protoIdx_ = helpers::Read<IDX_SIZE>(&sp);

    classOff_ = pandaFile.ResolveClassIndex(methodId, classIdx_).GetOffset();
    protoOff_ = pandaFile.ResolveProtoIndex(methodId, protoIdx_).GetOffset();

    nameOff_ = helpers::Read<ID_SIZE>(&sp);
    accessFlags_ = helpers::ReadULeb128(&sp);

    isExternal_ = pandaFile_.IsExternal(methodId);
    if (!isExternal_) {
        taggedValuesSp_ = sp;
        size_ = 0;
    } else {
        size_ = pandaFile_.GetIdFromPointer(sp.data()).GetOffset() - methodId_.GetOffset();
    }
}

}  // namespace ark::panda_file
