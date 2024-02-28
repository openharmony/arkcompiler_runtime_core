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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JSVALUE_CALL_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JSVALUE_CALL_H

#include <node_api.h>
#include "utils/span.h"
#include "libpandabase/mem/mem.h"
namespace panda {
class Method;
}  // namespace panda

namespace panda::mem {
class Reference;
}  // namespace panda::mem

namespace panda::ets {
class EtsObject;
class EtsString;
class EtsCoroutine;
}  // namespace panda::ets

namespace panda::ets::interop::js {
class InteropCtx;

napi_value CallEtsFunctionImpl(napi_env env, Span<napi_value> jsargv);
napi_value EtsLambdaProxyInvoke(napi_env env, napi_callback_info cbinfo);

uint8_t JSRuntimeInitJSCallClass(EtsString *clsStr);
uint8_t JSRuntimeInitJSNewClass(EtsString *clsStr);

template <bool IS_STATIC>
napi_value EtsCallImpl(EtsCoroutine *coro, InteropCtx *ctx, Method *method, Span<napi_value> jsargv,
                       EtsObject *thisObj);

inline napi_value EtsCallImplInstance(EtsCoroutine *coro, InteropCtx *ctx, Method *method, Span<napi_value> jsargv,
                                      EtsObject *thisObj)
{
    return EtsCallImpl<false>(coro, ctx, method, jsargv, thisObj);
}

inline napi_value EtsCallImplStatic(EtsCoroutine *coro, InteropCtx *ctx, Method *method, Span<napi_value> jsargv)
{
    return EtsCallImpl<true>(coro, ctx, method, jsargv, nullptr);
}

template <char DELIM = '.', typename F>
static bool WalkQualifiedName(std::string_view name, F const &f)
{
    for (const char *p = name.data(); *p == DELIM;) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto e = ++p;
        while (*e != '\0' && *e != DELIM) {
            e++;
        }
        std::string substr(p, ToUintPtr(e) - ToUintPtr(p));
        if (UNLIKELY(!f(substr))) {
            return false;
        }
        p = e;
    }
    return true;
}

}  // namespace panda::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_JSVALUE_CALL_H
