/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

static thread_local std::stringstream g_consoleBuf;

static void ConsoleCommit()
{
    auto const &str = g_consoleBuf.str();
    if (g_mlogBufPrint != nullptr) {
        g_mlogBufPrint(LOG_ID_MAIN, ark::Logger::PandaLog2MobileLog::INFO, "arkts.console", "%s", str.c_str());
    } else {
        std::cout << str.c_str() << std::endl;
    }
    g_consoleBuf = std::stringstream();
}

template <typename T>
static void ConsoleAddToBuffer(T const &t)
{
    g_consoleBuf << t;
}

extern "C" void StdConsolePrintln(ObjectHeader *header [[maybe_unused]])
{
    ConsoleCommit();
}

extern "C" void StdConsolePrintBool([[maybe_unused]] ObjectHeader *header, uint8_t b)
{
    ConsoleAddToBuffer(b == 0 ? "false" : "true");
}

extern "C" void StdConsolePrintChar([[maybe_unused]] ObjectHeader *header, uint16_t c)
{
    const utf::Utf8Char utf8Ch = utf::ConvertUtf16ToUtf8(c, 0, false);
    ConsoleAddToBuffer(std::string_view(reinterpret_cast<const char *>(utf8Ch.ch.data()), utf8Ch.n));
}

extern "C" void StdConsolePrintString([[maybe_unused]] ObjectHeader *header, EtsString *str)
{
    auto s = str->GetCoreType();
    if (s->IsUtf16()) {
        uint16_t *vdataPtr = s->GetDataUtf16();
        uint32_t vlength = s->GetLength();
        size_t mutf8Len = utf::Utf16ToMUtf8Size(vdataPtr, vlength);

        PandaVector<uint8_t> out(mutf8Len);
        utf::ConvertRegionUtf16ToMUtf8(vdataPtr, out.data(), vlength, mutf8Len, 0);

        ConsoleAddToBuffer(reinterpret_cast<const char *>(out.data()));
    } else {
        ConsoleAddToBuffer(std::string_view(reinterpret_cast<const char *>(s->GetDataMUtf8()), s->GetLength()));
    }
}

extern "C" void StdConsolePrintI32([[maybe_unused]] ObjectHeader *header, int32_t v)
{
    ConsoleAddToBuffer<int32_t>(v);
}

extern "C" void StdConsolePrintI16([[maybe_unused]] ObjectHeader *header, int16_t v)
{
    ConsoleAddToBuffer<int32_t>(v);
}

extern "C" void StdConsolePrintI8([[maybe_unused]] ObjectHeader *header, int8_t v)
{
    ConsoleAddToBuffer<int32_t>(v);
}

extern "C" void StdConsolePrintI64([[maybe_unused]] ObjectHeader *header, int64_t v)
{
    ConsoleAddToBuffer<int64_t>(v);
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
        ConsoleAddToBuffer<float>(v);
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
        ConsoleAddToBuffer<double>(v);
    }
}

}  // namespace ark::ets::intrinsics
