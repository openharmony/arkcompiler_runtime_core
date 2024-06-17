/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_NAPI_IMPL_H
#define PANDA_NAPI_IMPL_H

#include <node_api.h>
#include <node_api_types.h>
#include "detail/enumerate_napi.h"

namespace ark::ets::interop::js {

struct NapiImpl {
    // NOLINTBEGIN(cppcoreguidelines-macro-usage)
    static void InitNapi()
    {
        NapiImpl impl;
#define NAPI_METHOD(name, ...) impl.name = ::name;
        ENUMERATE_NAPI(NAPI_METHOD)
#undef NAPI_METHOD

        Init(impl);
    }

#define NAPI_METHOD(name, ...) napi_status (*name)(PARAMS_PAIR(__VA_ARGS__)) = nullptr;

    ENUMERATE_NAPI(NAPI_METHOD)

#undef NAPI_METHOD

private:
    PANDA_PUBLIC_API static void Init(NapiImpl impl);
    // NOLINTEND(cppcoreguidelines-macro-usage)
};

}  // namespace ark::ets::interop::js
#endif  // PANDA_NAPI_IMPL_H
