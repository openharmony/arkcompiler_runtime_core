/**
* Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef COMMON_INTERFACE_OBJECTS_BASE_STRING_TABLE_H
#define COMMON_INTERFACE_OBJECTS_BASE_STRING_TABLE_H

#include "common_interfaces/objects/readonly_handle.h"
#include "common_interfaces/thread/mutator.h"

namespace common {
class BaseString;

template <typename Impl>
class BaseStringTableInterface {
public:
    BaseStringTableInterface() {}

    using HandleCreator = std::function<ReadOnlyHandle<BaseString>(Mutator *, BaseString *)>;

    BaseString *GetOrInternFlattenString(Mutator *mutator, const HandleCreator &handleCreator, BaseString *string)
    {
        return static_cast<Impl *>(this)->GetOrInternFlattenString(mutator, handleCreator, string);
    }

    BaseString *GetOrInternStringFromCompressedSubString(Mutator *mutator, const HandleCreator &handleCreator,
                                                         const ReadOnlyHandle<BaseString> &string,
                                                         uint32_t offset, uint32_t utf8Len)
    {
        return static_cast<Impl *>(this)->GetOrInternStringFromCompressedSubString(
            mutator, handleCreator, string, offset, utf8Len);
    }

    BaseString *GetOrInternString(Mutator *mutator, const HandleCreator &handleCreator, const uint8_t *utf8Data,
                                  uint32_t utf8Len, bool canBeCompress)
    {
        return static_cast<Impl *>(this)->GetOrInternString(mutator, handleCreator, utf8Data, utf8Len, canBeCompress);
    }

    BaseString *GetOrInternString(Mutator *mutator, const HandleCreator &handleCreator, const uint16_t *utf16Data,
                                  uint32_t utf16Len, bool canBeCompress)
    {
        return static_cast<Impl *>(this)->GetOrInternString(mutator, handleCreator, utf16Data, utf16Len, canBeCompress);
    }

    BaseString *TryGetInternString(const ReadOnlyHandle<BaseString> &string)
    {
        return static_cast<Impl *>(this)->TryGetInternString(string);
    }

    // Runtime module interfaces
    void Init() {};

    void Fini() {};

private:
    NO_COPY_SEMANTIC_CC(BaseStringTableInterface);
    NO_MOVE_SEMANTIC_CC(BaseStringTableInterface);
};
}
#endif //COMMON_INTERFACE_OBJECTS_BASE_STRING_TABLE_H