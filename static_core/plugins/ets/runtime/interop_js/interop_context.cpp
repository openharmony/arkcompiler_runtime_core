/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/napi_env_scope.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/local_object_handle.h"

namespace panda::ets::interop::js {

InteropCtx::InteropCtx(EtsCoroutine *coro, napi_env env)
{
    EtsJSNapiEnvScope envscope(this, env);

    PandaEtsVM *vm = coro->GetPandaVM();
    EtsClassLinker *etsClassLinker = vm->GetClassLinker();
    refstor_ = vm->GetGlobalObjectStorage();
    linkerCtx_ = etsClassLinker->GetEtsClassLinkerExtension()->GetBootContext();

    JSRuntimeIntrinsicsSetIntrinsicsAPI(GetIntrinsicsAPI());
    auto *jobQueue = Runtime::GetCurrent()->GetInternalAllocator()->New<JsJobQueue>();
    vm->InitJobQueue(jobQueue);

    auto cacheClass = [&etsClassLinker](std::string_view descriptor) {
        auto klass = etsClassLinker->GetClass(descriptor.data())->GetRuntimeClass();
        ASSERT(klass != nullptr);
        return klass;
    };

    namespace descriptors = panda_file_items::class_descriptors;

    jsruntimeClass_ = cacheClass(descriptors::JS_RUNTIME);
    jsvalueClass_ = cacheClass(descriptors::JS_VALUE);
    jserrorClass_ = cacheClass(descriptors::JS_ERROR);
    objectClass_ = cacheClass(descriptors::OBJECT);
    stringClass_ = cacheClass(descriptors::STRING);
    voidClass_ = cacheClass(descriptors::VOID);
    promiseClass_ = cacheClass(descriptors::PROMISE);
    errorClass_ = cacheClass(descriptors::ERROR);
    exceptionClass_ = cacheClass(descriptors::EXCEPTION);
    typeClass_ = cacheClass(descriptors::TYPE);

    boxIntClass_ = cacheClass(descriptors::BOX_INT);
    boxLongClass_ = cacheClass(descriptors::BOX_LONG);

    arrayClass_ = cacheClass(descriptors::ARRAY);
    arraybufClass_ = cacheClass(descriptors::ARRAY_BUFFER);

    RegisterBuiltinJSRefConvertors(this);

    {
        auto method = EtsClass::FromRuntimeClass(jsruntimeClass_)->GetMethod("createFinalizationQueue");
        ASSERT(method != nullptr);
        auto res = method->GetPandaMethod()->Invoke(coro, nullptr);
        ASSERT(!coro->HasPendingException());
        auto queue = EtsObject::FromCoreType(res.GetAs<ObjectHeader *>());
        jsvalueFqueueRef_ = Refstor()->Add(queue->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
        ASSERT(jsvalueFqueueRef_ != nullptr);

        jsvalueFqueueRegister_ =
            queue->GetClass()
                ->GetMethod("register", "Lstd/core/Object;Lstd/core/Object;Lstd/core/Object;:Lstd/core/void;")
                ->GetPandaMethod();
        ASSERT(jsvalueFqueueRegister_ != nullptr);
    }

    etsProxyRefStorage_ = ets_proxy::SharedReferenceStorage::Create();
    ASSERT(etsProxyRefStorage_.get() != nullptr);

    // Set InteropCtx::DestroyLocalScopeForTopFrame to VM for call it it deoptimization and exception handlers
    coro->GetPandaVM()->SetClearInteropHandleScopesFunction(
        [this](Frame *frame) { this->DestroyLocalScopeForTopFrame(frame); });
}

EtsObject *InteropCtx::CreateETSCoreJSError(EtsCoroutine *coro, JSValue *jsvalue)
{
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coro);
    VMHandle<ObjectHeader> jsvalueHandle(coro, jsvalue->GetCoreType());

    Method::Proto proto(Method::Proto::ShortyVector {panda_file::Type(panda_file::Type::TypeId::VOID),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE)},
                        Method::Proto::RefTypeVector {utf::Mutf8AsCString(GetJSValueClass()->GetDescriptor())});
    auto ctorName = utf::CStringAsMutf8(panda_file_items::CTOR.data());
    auto ctor = GetJSErrorClass()->GetDirectMethod(ctorName, proto);
    ASSERT(ctor != nullptr);

    auto excObj = ObjectHeader::Create(coro, GetJSErrorClass());
    if (UNLIKELY(excObj == nullptr)) {
        return nullptr;
    }
    VMHandle<ObjectHeader> excHandle(coro, excObj);

    std::array<Value, 2U> args {Value(excHandle.GetPtr()), Value(jsvalueHandle.GetPtr())};
    ctor->InvokeVoid(coro, args.data());
    auto res = EtsObject::FromCoreType(excHandle.GetPtr());
    if (UNLIKELY(coro->HasPendingException())) {
        return nullptr;
    }
    return res;
}

void InteropCtx::ThrowETSError(EtsCoroutine *coro, napi_value val)
{
    auto ctx = Current(coro);

    if (coro->IsUsePreAllocObj()) {
        coro->SetUsePreAllocObj(false);
        coro->SetException(coro->GetVM()->GetOOMErrorObject());
        return;
    }
    ASSERT(!coro->HasPendingException());

    if (IsNullOrUndefined(ctx->GetJSEnv(), val)) {
        ctx->ThrowETSError(coro, "interop/js throws null");
        return;
    }

    // To catch `TypeError` or `UserError extends TypeError`
    // 1. Frontend puts catch(compat/TypeError) { <instanceof-rethrow? if UserError expected> }
    //    Where js.UserError will be wrapped into compat/TypeError
    //    NOTE(vpukhov): compat: add intrinsic to obtain JSValue from compat/ instances

    auto objRefconv = JSRefConvertResolve(ctx, ctx->GetObjectClass());
    LocalObjectHandle<EtsObject> etsObj(coro, objRefconv->Unwrap(ctx, val));
    if (UNLIKELY(etsObj.GetPtr() == nullptr)) {
        INTEROP_LOG(INFO) << "Something went wrong while unwrapping pending js exception";
        ASSERT(ctx->SanityETSExceptionPending());
        return;
    }

    auto klass = etsObj->GetClass()->GetRuntimeClass();
    if (LIKELY(ctx->GetErrorClass()->IsAssignableFrom(klass) || ctx->GetExceptionClass()->IsAssignableFrom(klass))) {
        coro->SetException(etsObj->GetCoreType());
        return;
    }

    // NOTE(vpukhov): should throw a special error (JSError?) with cause set
    auto exc = JSConvertJSError::Unwrap(ctx, ctx->GetJSEnv(), val);
    if (LIKELY(exc.has_value())) {
        ASSERT(exc != nullptr);
        coro->SetException(exc.value()->GetCoreType());
    }  // otherwise exception is already set
}

void InteropCtx::ThrowETSError(EtsCoroutine *coro, const char *msg)
{
    ASSERT(!coro->HasPendingException());
    ets::ThrowEtsException(coro, panda_file_items::class_descriptors::ERROR, msg);
}

void InteropCtx::ThrowJSError(napi_env env, const std::string &msg)
{
    INTEROP_LOG(INFO) << "ThrowJSError: " << msg;
    ASSERT(!NapiIsExceptionPending(env));
    NAPI_CHECK_FATAL(napi_throw_error(env, nullptr, msg.c_str()));
}

void InteropCtx::ThrowJSTypeError(napi_env env, const std::string &msg)
{
    INTEROP_LOG(INFO) << "ThrowJSTypeError: " << msg;
    ASSERT(!NapiIsExceptionPending(env));
    NAPI_CHECK_FATAL(napi_throw_type_error(env, nullptr, msg.c_str()));
}

void InteropCtx::ThrowJSValue(napi_env env, napi_value val)
{
    INTEROP_LOG(INFO) << "ThrowJSValue";
    ASSERT(!NapiIsExceptionPending(env));
    NAPI_CHECK_FATAL(napi_throw(env, val));
}

void InteropCtx::ForwardEtsException(EtsCoroutine *coro)
{
    auto env = GetJSEnv();
    ASSERT(coro->HasPendingException());
    LocalObjectHandle<ObjectHeader> exc(coro, coro->GetException());
    coro->ClearException();

    auto klass = exc->ClassAddr<Class>();
    ASSERT(GetErrorClass()->IsAssignableFrom(klass) || GetExceptionClass()->IsAssignableFrom(klass));
    JSRefConvert *refconv = JSRefConvertResolve<true>(this, klass);
    if (UNLIKELY(refconv == nullptr)) {
        INTEROP_LOG(INFO) << "Exception thrown while forwarding ets exception: " << klass->GetDescriptor();
        return;
    }
    napi_value res = refconv->Wrap(this, EtsObject::FromCoreType(exc.GetPtr()));
    if (UNLIKELY(res == nullptr)) {
        return;
    }
    ThrowJSValue(env, res);
}

void InteropCtx::ForwardJSException(EtsCoroutine *coro)
{
    auto env = GetJSEnv();
    napi_value excval;
    ASSERT(NapiIsExceptionPending(env));
    NAPI_CHECK_FATAL(napi_get_and_clear_last_exception(env, &excval));
    ThrowETSError(coro, excval);
}

void JSConvertTypeCheckFailed(const char *typeName)
{
    auto ctx = InteropCtx::Current();
    auto env = ctx->GetJSEnv();
    InteropCtx::ThrowJSTypeError(env, typeName + std::string(" expected"));
}

static std::optional<std::string> GetErrorStack(napi_env env, napi_value jsErr)
{
    bool isError;
    if (napi_ok != napi_is_error(env, jsErr, &isError)) {
        return {};
    }
    if (!isError) {
        return "not an Error instance";
    }
    napi_value jsStk;
    if (napi_ok != napi_get_named_property(env, jsErr, "stack", &jsStk)) {
        return {};
    }
    size_t length;
    if (napi_ok != napi_get_value_string_utf8(env, jsStk, nullptr, 0, &length)) {
        return {};
    }
    std::string value;
    value.resize(length);
    // +1 for NULL terminated string!!!
    if (napi_ok != napi_get_value_string_utf8(env, jsStk, value.data(), value.size() + 1, &length)) {
        return {};
    }
    return value;
}

static std::optional<std::string> NapiTryDumpStack(napi_env env)
{
    bool isPending;
    if (napi_ok != napi_is_exception_pending(env, &isPending)) {
        return {};
    }

    std::string pendingErrorMsg;
    if (isPending) {
        napi_value valuePending;
        if (napi_ok != napi_get_and_clear_last_exception(env, &valuePending)) {
            return {};
        }
        auto resStk = GetErrorStack(env, valuePending);
        if (resStk.has_value()) {
            pendingErrorMsg = "\nWith pending exception:\n" + resStk.value();
        } else {
            pendingErrorMsg = "\nFailed to stringify pending exception";
        }
    }

    std::string stacktraceMsg;
    {
        napi_value jsDummyStr;
        if (napi_ok !=
            napi_create_string_utf8(env, "probe-stacktrace-not-actual-error", NAPI_AUTO_LENGTH, &jsDummyStr)) {
            return {};
        }
        napi_value jsErr;
        auto rc = napi_create_error(env, nullptr, jsDummyStr, &jsErr);
        if (napi_ok != rc) {
            return {};
        }
        auto resStk = GetErrorStack(env, jsErr);
        stacktraceMsg = resStk.has_value() ? resStk.value() : "failed to stringify probe exception";
    }

    return stacktraceMsg + pendingErrorMsg;
}

[[noreturn]] void InteropCtx::Fatal(const char *msg)
{
    INTEROP_LOG(ERROR) << "InteropCtx::Fatal: " << msg;

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);

    INTEROP_LOG(ERROR) << "======================== ETS stack ============================";
    auto &istk = ctx->GetInteropFrames();
    auto istkIt = istk.rbegin();

    for (auto stack = StackWalker::Create(coro); stack.HasFrame(); stack.NextFrame()) {
        while (istkIt != istk.rend() && stack.GetFp() == istkIt->etsFrame) {
            INTEROP_LOG(ERROR) << (istkIt->toJs ? "<ets-to-js>" : "<js-to-ets>");
            istkIt++;
        }

        Method *method = stack.GetMethod();
        INTEROP_LOG(ERROR) << method->GetClass()->GetName() << "." << method->GetName().data << " at "
                           << method->GetLineNumberAndSourceFile(stack.GetBytecodePc());
    }
    ASSERT(istkIt == istk.rend() || istkIt->etsFrame == nullptr);

    auto env = ctx->jsEnv_;
    INTEROP_LOG(ERROR) << (env != nullptr ? "<ets-entrypoint>" : "current js env is nullptr!");

    if (coro->HasPendingException()) {
        auto exc = EtsObject::FromCoreType(coro->GetException());
        INTEROP_LOG(ERROR) << "With pending exception: " << exc->GetClass()->GetDescriptor();
    }

    if (env != nullptr) {
        INTEROP_LOG(ERROR) << "======================== JS stack =============================";
        std::optional<std::string> jsStk = NapiTryDumpStack(env);
        if (jsStk.has_value()) {
            INTEROP_LOG(ERROR) << jsStk.value();
        } else {
            INTEROP_LOG(ERROR) << "JS stack print failed";
        }
    }

    INTEROP_LOG(ERROR) << "======================== Native stack =========================";
    PrintStack(Logger::Message(Logger::Level::ERROR, Logger::Component::ETS_INTEROP_JS, false).GetStream());
    std::abort();
}

}  // namespace panda::ets::interop::js
