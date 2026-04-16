/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_EXCEPTIONS_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_EXCEPTIONS_H_

#include "plugins/ets/runtime/ets_platform_types.h"
#include <string_view>
#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/mem/panda_string.h"

namespace ark::ets {

PANDA_PUBLIC_API EtsObject *SetupEtsException(EtsExecutionContext *executionCtx, EtsClass *exceptionClass,
                                              const char *msg);

PANDA_PUBLIC_API void ThrowEtsException(EtsExecutionContext *executionCtx, const char *classDescriptor,
                                        const char *msg);

inline void ThrowEtsException(EtsExecutionContext *executionCtx, std::string_view classDescriptor, std::string_view msg)
{
    ThrowEtsException(executionCtx, classDescriptor.data(), msg.data());
}

void ThrowEtsException(EtsExecutionContext *executionCtx, EtsClass *cls, const char *msg);

inline void ThrowEtsException(EtsExecutionContext *executionCtx, EtsClass *exceptionClass, std::string_view msg)
{
    ThrowEtsException(executionCtx, exceptionClass, msg.data());
}

void ThrowEtsException(EtsExecutionContext *executionCtx, EtsClass *cls, int32_t errorCode, const char *msg);

inline void ThrowEtsException(EtsExecutionContext *executionCtx, EtsClass *exceptionClass, int32_t errorCode,
                              std::string_view msg)
{
    ThrowEtsException(executionCtx, exceptionClass, errorCode, msg.data());
}

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_EXCEPTIONS_H_
