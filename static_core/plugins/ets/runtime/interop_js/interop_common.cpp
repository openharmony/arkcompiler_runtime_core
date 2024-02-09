/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"

namespace ark::ets::interop::js {

[[noreturn]] void InteropFatal(const char *message)
{
    InteropCtx::Fatal(message);
    UNREACHABLE();
}

[[noreturn]] void InteropFatal(const std::string &message)
{
    InteropCtx::Fatal(message.c_str());
    UNREACHABLE();
}

[[noreturn]] void InteropFatal(const char *message, napi_status status)
{
    InteropCtx::Fatal(std::string(message) + " status=" + std::to_string(status));
    UNREACHABLE();
}

void InteropTrace(const char *func, const char *file, int line)
{
    INTEROP_LOG(DEBUG) << "trace: " << func << ":" << file << ":" << line;
}

}  // namespace ark::ets::interop::js

namespace ark::ets::ts2ets::GlobalCtx {  // NOLINT(readability-identifier-naming)

void Init()
{
    interop::js::InteropCtx::Init(EtsCoroutine::GetCurrent(), nullptr);
}

}  // namespace ark::ets::ts2ets::GlobalCtx
