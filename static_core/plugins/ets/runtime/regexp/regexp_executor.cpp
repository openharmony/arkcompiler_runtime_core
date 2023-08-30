/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/regexp/regexp_executor.h"
#include "include/coretypes/string.h"
#include "include/mem/panda_string.h"
#include "runtime/handle_scope-inl.h"
#include "types/ets_array.h"

namespace ark::ets {

std::pair<uint32_t, uint32_t> RegExpExecutor::GetIndices(CaptureState *captureState) const
{
    uint8_t *begin = GetInputPtr();
    uint32_t start = captureState->captureStart - begin;
    uint32_t end = captureState->captureEnd - begin;
    if (IsWideChar()) {
        return {start / WIDE_CHAR_SIZE, end / WIDE_CHAR_SIZE};
    }
    return {start, end};
}

void RegExpExecutor::HandleCaptures(PandaVector<std::pair<bool, PandaString>> &captures,
                                    PandaVector<std::pair<uint32_t, uint32_t>> &indices,
                                    CaptureState *captureState) const
{
    int32_t len = captureState->captureEnd - captureState->captureStart;
    PandaString res;
    if ((captureState->captureStart != nullptr && captureState->captureEnd != nullptr) && (len >= 0)) {
        if (IsWideChar()) {
            // create utf-16
            res = PandaString(reinterpret_cast<const char *>(captureState->captureStart), len);
        } else {
            // create utf-8 string
            PandaVector<uint8_t> buffer(len + 1);
            uint8_t *dest = buffer.data();
            if (memcpy_s(dest, len + 1, reinterpret_cast<const uint8_t *>(captureState->captureStart), len) != EOK) {
                LOG(FATAL, COMMON) << "memcpy_s failed";
                UNREACHABLE();
            }
            dest[len] = '\0';  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            res = PandaString(reinterpret_cast<const char *>(buffer.data()), len);
        }
        captures.push_back({true, res});
        indices.emplace_back(GetIndices(captureState));
    }
}

RegExpMatchResult<PandaString> RegExpExecutor::GetResult(bool isSuccess, bool hasIndices) const
{
    RegExpMatchResult<PandaString> result;
    PandaVector<std::pair<bool, PandaString>> captures;
    PandaVector<std::pair<uint32_t, uint32_t>> indices;
    result.isSuccess = isSuccess;
    result.isWide = IsWideChar();
    if (!isSuccess) {
        return result;
    }
    for (uint32_t i = 0; i < GetCaptureCount(); i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        CaptureState *captureState = &GetCaptureResultList()[i];
        if (i == 0) {
            result.index = captureState->captureStart - GetInputPtr();
            if (IsWideChar()) {
                result.index /= WIDE_CHAR_SIZE;
            }
        }
        HandleCaptures(captures, indices, captureState);
    }
    result.captures = std::move(captures);
    if (hasIndices) {
        result.indices = std::move(indices);
    } else {
        result.indices = {};
    }
    result.endIndex = GetCurrentPtr() - GetInputPtr();
    if (IsWideChar()) {
        result.endIndex /= WIDE_CHAR_SIZE;
    }
    return result;
}
}  // namespace ark::ets
