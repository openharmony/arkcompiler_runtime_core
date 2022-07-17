/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RUNTIME_MEM_FREE_OBJECT
#define PANDA_RUNTIME_MEM_FREE_OBJECT

#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/object_header.h"
#include "runtime/include/object_accessor.h"
#include "libpandabase/macros.h"

namespace panda::mem {
class FreeObject : public ObjectHeader {
public:
    uint32_t GetSize() const
    {
        return size_.GetInt();
    }

    FreeObject *GetNext() const
    {
        return next_;
    }

    static constexpr size_t GetNextOffset()
    {
        return MEMBER_OFFSET(FreeObject, next_);
    }

    static constexpr size_t GetSizeOffset()
    {
        return MEMBER_OFFSET(FreeObject, size_);
    }

private:
    FreeObject *next_;
    coretypes::TaggedValue size_;
};
}  // namespace panda::mem

#endif  // PANDA_RUNTIME_MEM_FREE_OBJECT
