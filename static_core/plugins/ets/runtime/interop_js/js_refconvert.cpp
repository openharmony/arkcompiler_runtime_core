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

#include "plugins/ets/runtime/interop_js/js_refconvert.h"

#include "plugins/ets/runtime/interop_js/ets_proxy/ets_class_wrapper.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_array.h"
#include "plugins/ets/runtime/interop_js/js_refconvert_function.h"

namespace ark::ets::interop::js {

static bool IsFunctionClass(InteropCtx *ctx, Class *klass)
{
    if (ctx->IsFunctionalInterface(klass)) {
        return true;
    }
    for (auto *itf : klass->GetInterfaces()) {
        if (ctx->IsFunctionalInterface(itf)) {
            return true;
        }
    }
    return false;
}

static std::unique_ptr<JSRefConvert> JSRefConvertCreateImpl(InteropCtx *ctx, Class *klass)
{
    INTEROP_FATAL_IF(klass->IsClassClass());

    if (klass->IsArrayClass()) {
        auto type = klass->GetComponentType()->GetType().GetId();
        if (type != panda_file::Type::TypeId::REFERENCE) {
            ctx->Fatal(std::string("Unhandled primitive array: ") + utf::Mutf8AsCString(klass->GetDescriptor()));
        }
        return std::make_unique<JSRefConvertReftypeArray>(klass);
    }

    if (js_proxy::JSProxy::IsProxyClass(klass)) {
        return ets_proxy::EtsClassWrapper::CreateJSRefConvertJSProxy(ctx, klass);
    }

    if (IsFunctionClass(ctx, klass)) {
        return std::make_unique<JSRefConvertFunction>(klass);
    }

    if (klass->IsInterface()) {
        return ets_proxy::EtsClassWrapper::CreateJSRefConvertEtsInterface(ctx, klass);
    }

    return ets_proxy::EtsClassWrapper::CreateJSRefConvertEtsProxy(ctx, klass);
}

template <bool ALLOW_INIT>
JSRefConvert *JSRefConvertCreate(InteropCtx *ctx, Class *klass)
{
    if (!CheckClassInitialized<ALLOW_INIT>(klass)) {
        ASSERT(EtsCoroutine::GetCurrent()->HasPendingException());
        return nullptr;
    }
    auto conv = JSRefConvertCreateImpl(ctx, klass);
    if (UNLIKELY(conv == nullptr)) {
        ctx->Fatal(std::string("Can't create convertor for ") + utf::Mutf8AsCString(klass->GetDescriptor()));
    }
    return ctx->GetRefConvertCache()->Insert(klass, std::move(conv));
}

template JSRefConvert *JSRefConvertCreate<false>(InteropCtx *ctx, Class *klass);
template JSRefConvert *JSRefConvertCreate<true>(InteropCtx *ctx, Class *klass);

}  // namespace ark::ets::interop::js
