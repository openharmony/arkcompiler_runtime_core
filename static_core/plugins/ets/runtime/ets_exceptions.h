/**
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include <string_view>
#include "libpandabase/macros.h"

namespace ark::ets {

class EtsCoroutine;
class EtsObject;

PANDA_PUBLIC_API EtsObject *SetupEtsException(EtsCoroutine *coroutine, const char *classDescriptor, const char *msg);

// NOTE: Is used to throw all language exceptional objects (currently Errors and Exceptions)
PANDA_PUBLIC_API void ThrowEtsException(EtsCoroutine *coroutine, const char *classDescriptor, const char *msg);

// NOTE: Is used to throw all language exceptional objects (currently Errors and Exceptions)
inline void ThrowEtsException(EtsCoroutine *coroutine, std::string_view classDescriptor, const char *msg)
{
    ThrowEtsException(coroutine, classDescriptor.data(), msg);
}

// NOTE: Is used to throw all language exceptional objects (currently Errors and Exceptions)
inline void ThrowEtsException(EtsCoroutine *coroutine, std::string_view classDescriptor, std::string_view msg)
{
    ThrowEtsException(coroutine, classDescriptor.data(), msg.data());
}

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_EXCEPTIONS_H_
