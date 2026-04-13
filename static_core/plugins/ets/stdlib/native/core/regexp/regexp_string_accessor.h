/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_REGEXP_STRING_ACCESSOR_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_REGEXP_STRING_ACCESSOR_H

#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/include/coretypes/string_flatten.h"
#include "runtime/include/runtime.h"

#include <cstdint>
#include <optional>

namespace ark::ets::stdlib {

class RegExpStringAccessor {
public:
    explicit RegExpStringAccessor(EtsString *etsStr)
    {
        ASSERT(etsStr != nullptr);
        auto *coreStr = etsStr->GetCoreType();
        length_ = coreStr->GetLength();
        isUtf16_ = coreStr->IsUtf16();

        if (coreStr->IsLineString()) {
            if (isUtf16_) {
                dataUtf16_ = coreStr->GetDataUtf16();
            } else {
                dataUtf8_ = coreStr->GetDataUtf8();
            }
            return;
        }

        auto *executionCtx = ets::EtsExecutionContext::GetCurrent();
        [[maybe_unused]] ets::EtsHandleScope scope(executionCtx);
        auto ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::ETS);
        VMHandle<coretypes::String> handle(executionCtx->GetMT(), reinterpret_cast<ObjectHeader *>(coreStr));
        flatInfo_.emplace(coretypes::FlatStringInfo::FlattenAllString(handle, ctx));
        if (isUtf16_) {
            dataUtf16_ = flatInfo_->GetDataUtf16();
        } else {
            dataUtf8_ = flatInfo_->GetDataUtf8();
        }
    }

    NO_COPY_SEMANTIC(RegExpStringAccessor);
    NO_MOVE_SEMANTIC(RegExpStringAccessor);

    ~RegExpStringAccessor() = default;

    bool IsUtf16() const
    {
        return isUtf16_;
    }

    uint32_t GetLength() const
    {
        return length_;
    }

    const uint8_t *GetDataUtf8() const
    {
        ASSERT(!isUtf16_);
        return dataUtf8_;
    }

    const uint16_t *GetDataUtf16() const
    {
        ASSERT(isUtf16_);
        return dataUtf16_;
    }

private:
    std::optional<coretypes::FlatStringInfo> flatInfo_;
    const uint8_t *dataUtf8_ = nullptr;
    const uint16_t *dataUtf16_ = nullptr;
    uint32_t length_ = 0;
    bool isUtf16_ = false;
};

}  // namespace ark::ets::stdlib

#endif  // PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_REGEXP_STRING_ACCESSOR_H
