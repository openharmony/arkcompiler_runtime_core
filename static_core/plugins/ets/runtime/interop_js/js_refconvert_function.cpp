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

#include "plugins/ets/runtime/interop_js/js_refconvert_function.h"

namespace panda::ets::interop::js {

napi_value JSRefConvertFunction::WrapImpl(InteropCtx *ctx, EtsObject *obj)
{
    auto coro = EtsCoroutine::GetCurrent();
    ASSERT(ctx == InteropCtx::Current(coro));
    auto env = ctx->GetJSEnv();

    ASSERT(obj->GetClass() == klass_);
    ASSERT(klass_->GetMethod("invoke") != nullptr);

    JSValue *jsValue;
    {
        NapiScope jsHandleScope(env);

        ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
        if (LIKELY(storage->HasReference(obj))) {
            ets_proxy::SharedReference *sharedRef = storage->GetReference(obj);
            ASSERT(sharedRef != nullptr);
            jsValue = JSValue::CreateRefValue(coro, ctx, sharedRef->GetJsObject(env), napi_function);
        } else {
            napi_value jsFn;
            ets_proxy::SharedReference *payloadSharedRef = storage->GetNextAlloc();
            NAPI_CHECK_FATAL(
                napi_create_function(env, "invoke", NAPI_AUTO_LENGTH, EtsLambdaProxyInvoke, payloadSharedRef, &jsFn));

            ets_proxy::SharedReference *sharedRef = storage->CreateETSObjectRef(ctx, obj, jsFn);
            if (UNLIKELY(sharedRef == nullptr)) {
                ASSERT(InteropCtx::SanityJSExceptionPending());
                return nullptr;
            }
            ASSERT(payloadSharedRef == sharedRef);
            jsValue = JSValue::CreateRefValue(coro, ctx, jsFn, napi_function);
        }
    }
    return jsValue->GetRefValue(env);
}

EtsObject *JSRefConvertFunction::UnwrapImpl([[maybe_unused]] InteropCtx *ctx, [[maybe_unused]] napi_value jsFun)
{
    UNREACHABLE();
}

}  // namespace panda::ets::interop::js
