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

#include "runtime/trace.h"
#include "types/ets_string.h"
#include "libarkbase/utils/logger.h"
#include "runtime/include/exceptions.h"

#ifdef PANDA_TARGET_OHOS
#include <hilog/log.h>
#endif

namespace ark::ets::intrinsics {

#ifdef PANDA_TARGET_OHOS
namespace {
constexpr uint32_t ARK_LOG_DOMAIN = 0xD003D00;
constexpr const char *ARK_LOG_TAG = "ArkRuntime";

#ifdef PANDA_OHOS_USE_INNER_HILOG
constexpr OHOS::HiviewDFX::HiLogLabel LOG_LABEL = {LOG_APP, ARK_LOG_DOMAIN, ARK_LOG_TAG};
#endif

void LogPrintEtsString(EtsString *str, LogLevel level)
{
    if (UNLIKELY(str == nullptr)) {
        ThrowNullPointerException();
        return;
    }
    auto mutf8 = str->GetMutf8();
#ifdef PANDA_OHOS_USE_INNER_HILOG
    if (level == LOG_INFO) {
        OHOS::HiviewDFX::HiLog::Info(LOG_LABEL, "%{public}s", mutf8.c_str());
    } else {
        OHOS::HiviewDFX::HiLog::Debug(LOG_LABEL, "%{public}s", mutf8.c_str());
    }
#else
    OH_LOG_Print(LOG_APP, level, ARK_LOG_DOMAIN, ARK_LOG_TAG, "%{public}s", mutf8.c_str());
#endif
}
}  // namespace
#endif

extern "C" void TracerStartTraceImpl(EtsString *name)
{
    if (UNLIKELY(name == nullptr)) {
        ThrowNullPointerException();
        return;
    }
    ark::Tracer::Start(name->GetMutf8().c_str());  // NOLINT(readability-redundant-string-cstr)
}

extern "C" void TracerFinishTraceImpl()
{
    ark::Tracer::Finish();
}

extern "C" void TracerStartAsyncTraceImpl(EtsString *name, int32_t taskId)
{
    if (UNLIKELY(name == nullptr)) {
        ThrowNullPointerException();
        return;
    }
    ark::Tracer::StartAsync(name->GetMutf8().c_str(), taskId);  // NOLINT(readability-redundant-string-cstr)
}

extern "C" void TracerFinishAsyncTraceImpl(EtsString *name, int32_t taskId)
{
    if (UNLIKELY(name == nullptr)) {
        ThrowNullPointerException();
        return;
    }
    ark::Tracer::FinishAsync(name->GetMutf8().c_str(), taskId);  // NOLINT(readability-redundant-string-cstr)
}

extern "C" void LogDebugImpl(EtsString *msg)
{
    if (UNLIKELY(msg == nullptr)) {
        ThrowNullPointerException();
        return;
    }
#ifdef PANDA_TARGET_OHOS
    LogPrintEtsString(msg, LOG_DEBUG);
#else
    LOG(DEBUG, STDLIB) << msg->GetMutf8();
#endif
}

extern "C" void LogInfoImpl(EtsString *msg)
{
    if (UNLIKELY(msg == nullptr)) {
        ThrowNullPointerException();
        return;
    }
#ifdef PANDA_TARGET_OHOS
    LogPrintEtsString(msg, LOG_INFO);
#else
    LOG(INFO, STDLIB) << msg->GetMutf8();
#endif
}

}  // namespace ark::ets::intrinsics
