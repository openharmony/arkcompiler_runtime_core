/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_record.h"
#include "plugins/ets/runtime/types/ets_type.h"

namespace ark::ets::interop::js {

napi_value JSRefConvertRecord::WrapImpl(InteropCtx *ctx, EtsObject *obj)
{
    auto env = ctx->GetJSEnv();

    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    if (LIKELY(storage->HasReference(obj, env))) {
        return storage->GetJsObject(obj, env);
    }

    napi_value handlerObj = GetReferenceValue(env, handleObjCache_);

    napi_value targetObj;
    NAPI_CHECK_FATAL(napi_create_object(env, &targetObj));

    std::array<napi_value, 2U> args = {targetObj, handlerObj};
    napi_value proxy = ctx->GetCommonJSObjectCache()->GetProxy();
    napi_value proxyObj;
    NAPI_CHECK_FATAL(napi_new_instance(env, proxy, args.size(), args.data(), &proxyObj));

    storage->CreateETSObjectRef(ctx, obj, handlerObj);

    return proxyObj;
}

napi_value JSRefConvertRecord::RecordGetHandler(napi_env env, napi_callback_info cbinfo)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS(coro);
    ScopedManagedCodeThread managedScope(coro);

    size_t argc;
    napi_value jsThis;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, nullptr, &jsThis, &data));
    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, jsArgs->data(), &jsThis, &data));

    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    ASSERT(storage != nullptr);
    ets_proxy::SharedReference *sharedRef = storage->GetReference(env, jsThis);
    ASSERT(sharedRef != nullptr);

    auto *etsThis = sharedRef->GetEtsObject();
    ASSERT(etsThis != nullptr);

    Method *getMethod = PlatformTypes()->escompatRecordGetter->GetPandaMethod();

    if (argc < 2U) {
        INTEROP_LOG(ERROR) << "Invalid number of arguments for $_get";
        return nullptr;
    }

    Span sp(&jsArgs[1], 1);
    return CallETSInstance(coro, ctx, getMethod, sp, etsThis);
}

napi_value JSRefConvertRecord::RecordSetHandler(napi_env env, napi_callback_info cbinfo)
{
    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);
    INTEROP_CODE_SCOPE_JS(coro);
    ScopedManagedCodeThread managedScope(coro);

    size_t argc;
    napi_value jsThis;
    void *data;
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, nullptr, &jsThis, &data));
    auto jsArgs = ctx->GetTempArgs<napi_value>(argc);
    NAPI_CHECK_FATAL(napi_get_cb_info(env, cbinfo, &argc, jsArgs->data(), &jsThis, &data));

    ets_proxy::SharedReferenceStorage *storage = ctx->GetSharedRefStorage();
    ASSERT(storage != nullptr);
    ets_proxy::SharedReference *sharedRef = storage->GetReference(env, jsThis);
    ASSERT(sharedRef != nullptr);

    auto *etsThis = sharedRef->GetEtsObject();
    ASSERT(etsThis != nullptr);

    Method *setMethod = PlatformTypes()->escompatRecordSetter->GetPandaMethod();

    if (argc < 3U) {
        INTEROP_LOG(ERROR) << "Invalid number of arguments for $_set";
        return nullptr;
    }

    Span sp(&jsArgs[1], 2U);

    CallETSInstance(coro, ctx, setMethod, sp, etsThis);

    napi_value trueValue = GetBooleanValue(env, true);
    return trueValue;
}

EtsObject *JSRefConvertRecord::UnwrapImpl([[maybe_unused]] InteropCtx *ctx, [[maybe_unused]] napi_value jsFun)
{
    auto coro = EtsCoroutine::GetCurrent();
    InteropCtx::ThrowETSError(coro, "Object is not compatible with escompat.Record");
    return nullptr;
}

}  // namespace ark::ets::interop::js