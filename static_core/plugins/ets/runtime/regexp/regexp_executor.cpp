/**
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

#include "plugins/ets/runtime/regexp/regexp_executor.h"
#include "runtime/handle_scope-inl.h"

namespace panda::ets {
RegExpMatchResult<VMHandle<EtsString>> RegExpExecutor::GetResult(bool is_success) const
{
    auto *thread = ManagedThread::GetCurrent();
    RegExpMatchResult<VMHandle<EtsString>> result;
    PandaVector<std::pair<bool, VMHandle<EtsString>>> captures;
    result.is_success = is_success;
    if (is_success) {
        for (uint32_t i = 0; i < GetCaptureCount(); i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            CaptureState *capture_state = &GetCaptureResultList()[i];
            if (i == 0) {
                result.index = capture_state->capture_start - GetInputPtr();
                if (IsWideChar()) {
                    result.index /= WIDE_CHAR_SIZE;
                }
            }
            int32_t len = capture_state->capture_end - capture_state->capture_start;
            std::pair<bool, VMHandle<EtsString>> pair;
            if ((capture_state->capture_start != nullptr && capture_state->capture_end != nullptr) && (len >= 0)) {
                pair.first = false;
                if (IsWideChar()) {
                    // create utf-16 string
                    pair.second = VMHandle<EtsString>(
                        thread, EtsString::CreateFromUtf16(
                                    reinterpret_cast<const uint16_t *>(capture_state->capture_start), len / 2)
                                    ->GetCoreType());
                } else {
                    // create utf-8 string
                    PandaVector<uint8_t> buffer(len + 1);
                    uint8_t *dest = buffer.data();
                    if (memcpy_s(dest, len + 1, reinterpret_cast<const uint8_t *>(capture_state->capture_start), len) !=
                        EOK) {
                        LOG(FATAL, COMMON) << "memcpy_s failed";
                        UNREACHABLE();
                    }
                    dest[len] = '\0';  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    pair.second = VMHandle<EtsString>(
                        thread,
                        EtsString::CreateFromUtf8(reinterpret_cast<const char *>(buffer.data()), len)->GetCoreType());
                }
            } else {
                // undefined
                pair.first = true;
            }
            captures.emplace_back(pair);
        }
        result.captures = std::move(captures);
        result.end_index = GetCurrentPtr() - GetInputPtr();
        if (IsWideChar()) {
            result.end_index /= WIDE_CHAR_SIZE;
        }
    }
    return result;
}
}  // namespace panda::ets
