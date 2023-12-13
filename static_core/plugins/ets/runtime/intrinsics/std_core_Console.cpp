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

#include <iostream>
#include <string_view>
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "libpandabase/utils/utf.h"

#include "intrinsics.h"

namespace ark::ets::intrinsics {

namespace {
constexpr const char *NAN_LITERAL = "NaN";
constexpr const char *INF_LITERAL = "Infinity";
constexpr const char *NEGINF_LITERAL = "-Infinity";
}  // namespace

extern "C" void StdConsolePrintln(ObjectHeader *header [[maybe_unused]])
{
    std::cout << std::endl;
}

extern "C" void StdConsolePrintBool([[maybe_unused]] ObjectHeader *header, uint8_t b)
{
    if (b != 0U) {
        std::cout << "true";
    } else {
        std::cout << "false";
    }
}

extern "C" void StdConsolePrintChar([[maybe_unused]] ObjectHeader *header, uint16_t c)
{
    const utf::Utf8Char utf8Ch = utf::ConvertUtf16ToUtf8(c, 0, false);
    std::cout << std::string_view(reinterpret_cast<const char *>(utf8Ch.ch.data()), utf8Ch.n);
}

extern "C" void StdConsolePrintString([[maybe_unused]] ObjectHeader *header, EtsString *str)
{
    ark::intrinsics::PrintString(str->GetCoreType());
}

extern "C" void StdConsolePrintI32([[maybe_unused]] ObjectHeader *header, int32_t v)
{
    ark::intrinsics::PrintI32(v);
}

extern "C" void StdConsolePrintI16([[maybe_unused]] ObjectHeader *header, int16_t v)
{
    ark::intrinsics::PrintI32(v);
}

extern "C" void StdConsolePrintI8([[maybe_unused]] ObjectHeader *header, int8_t v)
{
    ark::intrinsics::PrintI32(v);
}

extern "C" void StdConsolePrintI64([[maybe_unused]] ObjectHeader *header, int64_t v)
{
    ark::intrinsics::PrintI64(v);
}

extern "C" void StdConsolePrintF32([[maybe_unused]] ObjectHeader *header, float v)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    if (std::isnan(v)) {
        StdConsolePrintString(header,
                              EtsHandle<EtsString>(coroutine, EtsString::CreateFromMUtf8(NAN_LITERAL)).GetPtr());
    } else if (!std::isfinite(v)) {
        if (v < 0) {
            StdConsolePrintString(header,
                                  EtsHandle<EtsString>(coroutine, EtsString::CreateFromMUtf8(NEGINF_LITERAL)).GetPtr());
        } else {
            StdConsolePrintString(header,
                                  EtsHandle<EtsString>(coroutine, EtsString::CreateFromMUtf8(INF_LITERAL)).GetPtr());
        }
    } else {
        ark::intrinsics::PrintF32(v);
    }
}

extern "C" void StdConsolePrintF64([[maybe_unused]] ObjectHeader *header, double v)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    if (std::isnan(v)) {
        StdConsolePrintString(header,
                              EtsHandle<EtsString>(coroutine, EtsString::CreateFromMUtf8(NAN_LITERAL)).GetPtr());
    } else if (!std::isfinite(v)) {
        if (v < 0) {
            StdConsolePrintString(header,
                                  EtsHandle<EtsString>(coroutine, EtsString::CreateFromMUtf8(NEGINF_LITERAL)).GetPtr());
        } else {
            StdConsolePrintString(header,
                                  EtsHandle<EtsString>(coroutine, EtsString::CreateFromMUtf8(INF_LITERAL)).GetPtr());
        }
    } else {
        ark::intrinsics::PrintF64(v);
    }
}

}  // namespace ark::ets::intrinsics
