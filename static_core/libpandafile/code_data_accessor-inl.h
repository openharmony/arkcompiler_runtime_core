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

#ifndef LIBPANDAFILE_CODE_DATA_ACCESSOR_INL_H_
#define LIBPANDAFILE_CODE_DATA_ACCESSOR_INL_H_

#include <algorithm>
#include "code_data_accessor.h"

namespace ark::panda_file {

template <class Callback>
inline void CodeDataAccessor::TryBlock::EnumerateCatchBlocks(const Callback &cb)
{
    auto sp = catchBlocksSp_;
    for (size_t i = 0; i < numCatches_; i++) {
        CatchBlock catchBlock(sp);
        if (!cb(catchBlock)) {
            return;
        }
        sp = sp.SubSpan(catchBlock.GetSize());
    }
    size_ = sp.data() - data_.data();
}

inline void CodeDataAccessor::TryBlock::SkipCatchBlocks()
{
    EnumerateCatchBlocks([](const CatchBlock & /* unused */) { return true; });
}

template <class Callback>
inline void CodeDataAccessor::EnumerateTryBlocks(const Callback &cb)
{
    auto sp = tryBlocksSp_;
    for (size_t i = 0; i < triesSize_; i++) {
        TryBlock tryBlock(sp);
        if (!cb(tryBlock)) {
            return;
        }
        sp = sp.SubSpan(tryBlock.GetSize());
    }
    size_ = pandaFile_.GetIdFromPointer(sp.data()).GetOffset() - codeId_.GetOffset();
}

inline void CodeDataAccessor::SkipTryBlocks()
{
    EnumerateTryBlocks([](const TryBlock & /* unused */) { return true; });
}

// static
inline uint32_t CodeDataAccessor::GetNumVregs(const File &pf, File::EntityId codeId)
{
    uint32_t numVregs;
    auto sp = pf.GetSpanFromId(codeId);
    numVregs = helpers::ReadULeb128(&sp);
    return numVregs;
}

// static
inline const uint8_t *CodeDataAccessor::GetInstructions(const File &pf, File::EntityId codeId, uint32_t *vregs)
{
    auto sp = pf.GetSpanFromId(codeId);
    *vregs = helpers::ReadULeb128(&sp);
    helpers::SkipULeb128(&sp);  // num_args
    helpers::SkipULeb128(&sp);  // code_size
    helpers::SkipULeb128(&sp);  // tries_size
    return sp.data();
}

}  // namespace ark::panda_file

#endif  // LIBPANDAFILE_CODE_DATA_ACCESSOR_INL_H_
