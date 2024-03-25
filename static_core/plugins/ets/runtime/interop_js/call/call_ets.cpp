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

#include "plugins/ets/runtime/interop_js/call/call.h"
#include "plugins/ets/runtime/interop_js/call/arg_convertors.h"
#include "plugins/ets/runtime/interop_js/call/proto_reader.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"

namespace ark::ets::interop::js {

class CallETSHandler {
public:
    ALWAYS_INLINE CallETSHandler(EtsCoroutine *coro, InteropCtx *ctx, Method *method, Span<napi_value> jsargv,
                                 EtsObject *thisObj)
        : coro_(coro),
          ctx_(ctx),
          protoReader_(method, ctx_->GetClassLinker(), ctx_->LinkerCtx()),
          thisObj_(thisObj),
          jsargv_(jsargv)
    {
    }

    template <bool IS_STATIC>
    static ALWAYS_INLINE napi_value HandleImpl(EtsCoroutine *coro, InteropCtx *ctx, Method *method,
                                               Span<napi_value> jsargv, EtsObject *thisObj)
    {
        CallETSHandler st(coro, ctx, method, jsargv, thisObj);
        return st.HandleImpl<IS_STATIC>();
    }

    ~CallETSHandler() = default;

private:
    template <bool IS_STATIC>
    ALWAYS_INLINE bool ConvertArgs(Span<Value> etsArgs);
    ALWAYS_INLINE ObjectHeader **ConvertRestParams(Span<napi_value> restArgs);

    ALWAYS_INLINE bool CheckNumArgs(size_t numArgs) const;

    template <bool IS_STATIC>
    napi_value HandleImpl();

    static napi_value __attribute__((noinline)) ForwardException(InteropCtx *ctx, EtsCoroutine *coro)
    {
        if (coro->HasPendingException()) {
            ctx->ForwardEtsException(coro);
        }
        ASSERT(ctx->SanityJSExceptionPending());
        return nullptr;
    }

    NO_COPY_SEMANTIC(CallETSHandler);
    NO_MOVE_SEMANTIC(CallETSHandler);

    EtsCoroutine *const coro_;
    InteropCtx *const ctx_;

    ProtoReader protoReader_;

    EtsObject *thisObj_;
    Span<napi_value> jsargv_;
};

template <bool IS_STATIC>
ALWAYS_INLINE inline bool CallETSHandler::ConvertArgs(Span<Value> etsArgs)
{
    HandleScope<ObjectHeader *> etsHandleScope(coro_);
    auto const createRoot = [coro = coro_](ObjectHeader *val) {
        return reinterpret_cast<ObjectHeader **>(VMHandle<ObjectHeader>(coro, val).GetAddress());
    };

    ObjectHeader **thisObjRoot = IS_STATIC ? nullptr : createRoot(thisObj_->GetCoreType());

    using ArgValueBox = std::variant<uint64_t, ObjectHeader **>;
    auto const numArgs = protoReader_.GetMethod()->GetNumArgs() - !IS_STATIC;
    auto const numNonRest = numArgs - protoReader_.GetMethod()->HasVarArgs();
    auto etsBoxedArgs = ctx_->GetTempArgs<ArgValueBox>(numArgs);

    // Convert and store in root if necessary
    for (uint32_t argIdx = 0; argIdx < numNonRest; ++argIdx, protoReader_.Advance()) {
        auto jsVal = jsargv_[argIdx];
        auto store = [&etsBoxedArgs, &argIdx, createRoot](auto val) {
            if constexpr (std::is_pointer_v<decltype(val)> || std::is_null_pointer_v<decltype(val)>) {
                etsBoxedArgs[argIdx] = createRoot(val);
            } else {
                etsBoxedArgs[argIdx] = static_cast<uint64_t>(val);
            }
        };
        if (UNLIKELY(!ConvertArgToEts(ctx_, protoReader_, store, jsVal))) {
            return false;
        }
    }

    if (protoReader_.GetMethod()->HasVarArgs()) {
        const auto restIdx = numArgs - 1;
        etsBoxedArgs[restIdx] = ConvertRestParams(jsargv_.SubSpan(restIdx));
    }

    // Unbox values
    if constexpr (!IS_STATIC) {
        etsArgs[0] = Value(*thisObjRoot);
    }
    static constexpr size_t ETS_ARGS_DISP = IS_STATIC ? 0 : 1;

    for (size_t i = 0; i < numArgs; ++i) {
        ArgValueBox &box = etsBoxedArgs[i];
        if (std::holds_alternative<ObjectHeader **>(box)) {
            ObjectHeader **slot = std::get<1>(box);
            etsArgs[ETS_ARGS_DISP + i] = Value(slot != nullptr ? *slot : nullptr);
        } else {
            etsArgs[ETS_ARGS_DISP + i] = Value(std::get<0>(box));
        }
    }
    return true;
}

ObjectHeader **CallETSHandler::ConvertRestParams(Span<napi_value> restArgs)
{
    ASSERT(protoReader_.GetType().IsReference());
    ASSERT(protoReader_.GetClass()->IsArrayClass());

    ObjectHeader **restParamsSlot = PackRestParameters(coro_, ctx_, protoReader_, restArgs);
    ASSERT(restParamsSlot != nullptr);

    return restParamsSlot;
}

bool CallETSHandler::CheckNumArgs(size_t numArgs) const
{
    const auto method = protoReader_.GetMethod();
    bool const hasRestParams = method->HasVarArgs();
    ASSERT((hasRestParams && numArgs > 0) || !hasRestParams);

    if ((hasRestParams && (numArgs - 1) > jsargv_.size()) || (!hasRestParams && numArgs != jsargv_.size())) {
        std::string msg = "CallEtsFunction: wrong argc, ets_argc=" + std::to_string(numArgs) +
                          " js_argc=" + std::to_string(jsargv_.size()) + " ets_method='" +
                          std::string(method->GetFullName(true)) + "'";
        InteropCtx::ThrowJSTypeError(ctx_->GetJSEnv(), msg);
        return false;
    }
    return true;
}

template <bool IS_STATIC>
napi_value CallETSHandler::HandleImpl()
{
    ASSERT_MANAGED_CODE();
    auto method = protoReader_.GetMethod();

    protoReader_.Advance();  // skip return type
    ASSERT(method->IsStatic() == IS_STATIC);
    ASSERT(IS_STATIC == (thisObj_ == nullptr));

    auto const numArgs = method->GetNumArgs() - (IS_STATIC ? 0 : 1);
    if (UNLIKELY(!CheckNumArgs(numArgs))) {
        return ForwardException(ctx_, coro_);
    }

    auto etsArgs = ctx_->GetTempArgs<Value>(method->GetNumArgs());
    if (UNLIKELY(!ConvertArgs<IS_STATIC>(*etsArgs))) {
        return ForwardException(ctx_, coro_);
    }

    Value etsRes = method->Invoke(coro_, etsArgs->data());
    if (UNLIKELY(coro_->HasPendingException())) {
        return ForwardException(ctx_, coro_);
    }

    protoReader_.Reset();
    napi_value jsRes;
    auto readVal = [&etsRes](auto typeTag) { return etsRes.GetAs<typename decltype(typeTag)::type>(); };
    if (UNLIKELY(!ConvertArgToJS(ctx_, protoReader_, &jsRes, readVal))) {
        return ForwardException(ctx_, coro_);
    }
    return jsRes;
}

napi_value CallETSInstance(EtsCoroutine *coro, InteropCtx *ctx, Method *method, Span<napi_value> jsargv,
                           EtsObject *thisObj)
{
    return CallETSHandler::HandleImpl<false>(coro, ctx, method, jsargv, thisObj);
}
napi_value CallETSStatic(EtsCoroutine *coro, InteropCtx *ctx, Method *method, Span<napi_value> jsargv)
{
    return CallETSHandler::HandleImpl<true>(coro, ctx, method, jsargv, nullptr);
}

Expected<Method *, char const *> ResolveEntryPoint(InteropCtx *ctx, std::string_view entryPoint)
{
    uint8_t const *classDescriptor;
    uint8_t const *methodName;
    PandaString complexClassName;

    if (auto packageSep = entryPoint.rfind('.'); packageSep != PandaString::npos) {
        complexClassName = 'L' + PandaString(entryPoint.substr(0, packageSep + 1)) + "ETSGLOBAL;";
        std::replace(complexClassName.begin(), complexClassName.end(), '.', '/');
        classDescriptor = utf::CStringAsMutf8(complexClassName.data());
        methodName = utf::CStringAsMutf8(&entryPoint.at(packageSep + 1));
    } else {
        classDescriptor = utf::CStringAsMutf8("LETSGLOBAL;");
        methodName = utf::CStringAsMutf8(entryPoint.data());
    }

    Class *cls = ctx->GetClassLinker()->GetClass(classDescriptor, true, ctx->LinkerCtx());
    if (UNLIKELY(cls == nullptr)) {
        return Unexpected("Cannot find class");
    }

    Method *method = cls->GetDirectMethod(methodName);
    if (UNLIKELY(method == nullptr)) {
        return Unexpected("Cannot find method");
    }
    return method;
}

}  // namespace ark::ets::interop::js
