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
    EtsClassLinker *ets_class_linker = vm->GetClassLinker();
    refstor_ = vm->GetGlobalObjectStorage();
    linker_ctx_ = ets_class_linker->GetEtsClassLinkerExtension()->GetBootContext();

    JSRuntimeIntrinsicsSetIntrinsicsAPI(GetIntrinsicsAPI());
    auto *job_queue = Runtime::GetCurrent()->GetInternalAllocator()->New<JsJobQueue>();
    vm->InitJobQueue(job_queue);

    auto cache_class = [&](std::string_view descriptor) {
        auto klass = ets_class_linker->GetClass(descriptor.data())->GetRuntimeClass();
        ASSERT(klass != nullptr);
        return klass;
    };

    namespace descriptors = panda_file_items::class_descriptors;

    jsruntime_class_ = cache_class(descriptors::JS_RUNTIME);
    jsvalue_class_ = cache_class(descriptors::JS_VALUE);
    jserror_class_ = cache_class(descriptors::JS_ERROR);
    object_class_ = cache_class(descriptors::OBJECT);
    string_class_ = cache_class(descriptors::STRING);
    void_class_ = cache_class(descriptors::VOID);
    promise_class_ = cache_class(descriptors::PROMISE);
    error_class_ = cache_class(descriptors::ERROR);
    exception_class_ = cache_class(descriptors::EXCEPTION);
    type_class_ = cache_class(descriptors::TYPE);

    box_int_class_ = cache_class(descriptors::BOX_INT);
    box_long_class_ = cache_class(descriptors::BOX_LONG);

    array_class_ = cache_class(descriptors::ARRAY);
    arraybuf_class_ = cache_class(descriptors::ARRAY_BUFFER);

    RegisterBuiltinJSRefConvertors(this);

    {
        auto method = EtsClass::FromRuntimeClass(jsruntime_class_)->GetMethod("createFinalizationQueue");
        ASSERT(method != nullptr);
        auto res = method->GetPandaMethod()->Invoke(coro, nullptr);
        ASSERT(!coro->HasPendingException());
        auto queue = EtsObject::FromCoreType(res.GetAs<ObjectHeader *>());
        jsvalue_fqueue_ref_ = Refstor()->Add(queue->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
        ASSERT(jsvalue_fqueue_ref_ != nullptr);

        jsvalue_fqueue_register_ =
            queue->GetClass()
                ->GetMethod("register", "Lstd/core/Object;Lstd/core/Object;Lstd/core/Object;:Lstd/core/void;")
                ->GetPandaMethod();
        ASSERT(jsvalue_fqueue_register_ != nullptr);
    }

    ets_proxy_ref_storage_ = ets_proxy::SharedReferenceStorage::Create();
    ASSERT(ets_proxy_ref_storage_.get() != nullptr);

    // Set InteropCtx::DestroyLocalScopeForTopFrame to VM for call it it deoptimization and exception handlers
    coro->GetPandaVM()->SetClearInteropHandleScopesFunction(
        [this](Frame *frame) { this->DestroyLocalScopeForTopFrame(frame); });
}

EtsObject *InteropCtx::CreateETSCoreJSError(EtsCoroutine *coro, JSValue *jsvalue)
{
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coro);
    VMHandle<ObjectHeader> jsvalue_handle(coro, jsvalue->GetCoreType());

    Method::Proto proto(Method::Proto::ShortyVector {panda_file::Type(panda_file::Type::TypeId::VOID),
                                                     panda_file::Type(panda_file::Type::TypeId::REFERENCE)},
                        Method::Proto::RefTypeVector {utf::Mutf8AsCString(GetJSValueClass()->GetDescriptor())});
    auto ctor_name = utf::CStringAsMutf8(panda_file_items::CTOR.data());
    auto ctor = GetJSErrorClass()->GetDirectMethod(ctor_name, proto);
    ASSERT(ctor != nullptr);

    auto exc_obj = ObjectHeader::Create(coro, GetJSErrorClass());
    if (UNLIKELY(exc_obj == nullptr)) {
        return nullptr;
    }
    VMHandle<ObjectHeader> exc_handle(coro, exc_obj);

    std::array<Value, 2> args {Value(exc_handle.GetPtr()), Value(jsvalue_handle.GetPtr())};
    ctor->InvokeVoid(coro, args.data());
    auto res = EtsObject::FromCoreType(exc_handle.GetPtr());
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
    //    TODO(vpukhov): compat: add intrinsic to obtain JSValue from compat/ instances

    auto obj_refconv = JSRefConvertResolve(ctx, ctx->GetObjectClass());
    LocalObjectHandle<EtsObject> ets_obj(coro, obj_refconv->Unwrap(ctx, val));
    if (UNLIKELY(ets_obj.GetPtr() == nullptr)) {
        INTEROP_LOG(INFO) << "Something went wrong while unwrapping pending js exception";
        ASSERT(ctx->SanityETSExceptionPending());
        return;
    }

    auto klass = ets_obj->GetClass()->GetRuntimeClass();
    if (LIKELY(ctx->GetErrorClass()->IsAssignableFrom(klass) || ctx->GetExceptionClass()->IsAssignableFrom(klass))) {
        coro->SetException(ets_obj->GetCoreType());
        return;
    }

    // TODO(vpukhov): should throw a special error (JSError?) with cause set
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

void JSConvertTypeCheckFailed(const char *type_name)
{
    auto ctx = InteropCtx::Current();
    auto env = ctx->GetJSEnv();
    InteropCtx::ThrowJSTypeError(env, type_name + std::string(" expected"));
}

static std::optional<std::string> GetErrorStack(napi_env env, napi_value js_err)
{
    bool is_error;
    if (napi_ok != napi_is_error(env, js_err, &is_error)) {
        return {};
    }
    if (!is_error) {
        return "not an Error instance";
    }
    napi_value js_stk;
    if (napi_ok != napi_get_named_property(env, js_err, "stack", &js_stk)) {
        return {};
    }
    size_t length;
    if (napi_ok != napi_get_value_string_utf8(env, js_stk, nullptr, 0, &length)) {
        return {};
    }
    std::string value;
    value.resize(length);
    // +1 for NULL terminated string!!!
    if (napi_ok != napi_get_value_string_utf8(env, js_stk, value.data(), value.size() + 1, &length)) {
        return {};
    }
    return value;
}

static std::optional<std::string> NapiTryDumpStack(napi_env env)
{
    bool is_pending;
    if (napi_ok != napi_is_exception_pending(env, &is_pending)) {
        return {};
    }

    std::string pending_error_msg;
    if (is_pending) {
        napi_value value_pending;
        if (napi_ok != napi_get_and_clear_last_exception(env, &value_pending)) {
            return {};
        }
        auto res_stk = GetErrorStack(env, value_pending);
        if (res_stk.has_value()) {
            pending_error_msg = "\nWith pending exception:\n" + res_stk.value();
        } else {
            pending_error_msg = "\nFailed to stringify pending exception";
        }
    }

    std::string stacktrace_msg;
    {
        napi_value js_dummy_str;
        if (napi_ok !=
            napi_create_string_utf8(env, "probe-stacktrace-not-actual-error", NAPI_AUTO_LENGTH, &js_dummy_str)) {
            return {};
        }
        napi_value js_err;
        auto rc = napi_create_error(env, nullptr, js_dummy_str, &js_err);
        if (napi_ok != rc) {
            return {};
        }
        auto res_stk = GetErrorStack(env, js_err);
        stacktrace_msg = res_stk.has_value() ? res_stk.value() : "failed to stringify probe exception";
    }

    return stacktrace_msg + pending_error_msg;
}

[[noreturn]] void InteropCtx::Fatal(const char *msg)
{
    INTEROP_LOG(ERROR) << "InteropCtx::Fatal: " << msg;

    auto coro = EtsCoroutine::GetCurrent();
    auto ctx = InteropCtx::Current(coro);

    INTEROP_LOG(ERROR) << "======================== ETS stack ============================";
    auto &istk = ctx->GetInteropFrames();
    auto istk_it = istk.rbegin();

    for (auto stack = StackWalker::Create(coro); stack.HasFrame(); stack.NextFrame()) {
        while (istk_it != istk.rend() && stack.GetFp() == istk_it->ets_frame) {
            INTEROP_LOG(ERROR) << (istk_it->to_js ? "<ets-to-js>" : "<js-to-ets>");
            istk_it++;
        }

        Method *method = stack.GetMethod();
        INTEROP_LOG(ERROR) << method->GetClass()->GetName() << "." << method->GetName().data << " at "
                           << method->GetLineNumberAndSourceFile(stack.GetBytecodePc());
    }
    ASSERT(istk_it == istk.rend() || istk_it->ets_frame == nullptr);

    auto env = ctx->js_env_;
    INTEROP_LOG(ERROR) << (env != nullptr ? "<ets-entrypoint>" : "current js env is nullptr!");

    if (coro->HasPendingException()) {
        auto exc = EtsObject::FromCoreType(coro->GetException());
        INTEROP_LOG(ERROR) << "With pending exception: " << exc->GetClass()->GetDescriptor();
    }

    if (env != nullptr) {
        INTEROP_LOG(ERROR) << "======================== JS stack =============================";
        std::optional<std::string> js_stk = NapiTryDumpStack(env);
        if (js_stk.has_value()) {
            INTEROP_LOG(ERROR) << js_stk.value();
        } else {
            INTEROP_LOG(ERROR) << "JS stack print failed";
        }
    }

    INTEROP_LOG(ERROR) << "======================== Native stack =========================";
    PrintStack(Logger::Message(Logger::Level::ERROR, Logger::Component::ETS_INTEROP_JS, false).GetStream());
    std::abort();
}

}  // namespace panda::ets::interop::js
