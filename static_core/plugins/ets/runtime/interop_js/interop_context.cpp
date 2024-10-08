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

#include "plugins/ets/runtime/interop_js/interop_context.h"

#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/interop_js/js_convert.h"
#include "plugins/ets/runtime/interop_js/interop_common.h"
#include "plugins/ets/runtime/interop_js/code_scopes.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/local_object_handle.h"

namespace ark::ets::interop::js {

namespace descriptors = panda_file_items::class_descriptors;

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static constexpr std::string_view const FUNCTION_INTERFACE_DESCRIPTORS[] = {
    descriptors::FUNCTION0,  descriptors::FUNCTION1,  descriptors::FUNCTION2,  descriptors::FUNCTION3,
    descriptors::FUNCTION4,  descriptors::FUNCTION5,  descriptors::FUNCTION6,  descriptors::FUNCTION7,
    descriptors::FUNCTION8,  descriptors::FUNCTION9,  descriptors::FUNCTION10, descriptors::FUNCTION11,
    descriptors::FUNCTION12, descriptors::FUNCTION13, descriptors::FUNCTION14, descriptors::FUNCTION15,
    descriptors::FUNCTION16, descriptors::FUNCTIONN,
};

static Class *CacheClass(EtsClassLinker *etsClassLinker, std::string_view descriptor)
{
    auto klass = etsClassLinker->GetClass(descriptor.data())->GetRuntimeClass();
    ASSERT(klass != nullptr);
    return klass;
}

void ConstStringStorage::LoadDynamicCallClass(Class *klass)
{
    if (klass == lastLoadedClass_) {
        return;
    }
    lastLoadedClass_ = klass;
    auto fields = klass->GetStaticFields();
    ASSERT(fields.Size() == 1);
    auto startFrom = klass->GetFieldPrimitive<uint32_t>(fields[0]);
    napi_value jsArr;
    auto *env = Ctx()->GetJSEnv();
    if (jsStringBufferRef_ == nullptr) {
        jsArr = InitBuffer(Ctx()->GetStringBufferSize());
    } else {
        jsArr = GetReferenceValue(env, jsStringBufferRef_);
        if (startFrom >= stringBuffer_.size()) {
            stringBuffer_.resize(Ctx()->GetStringBufferSize());
        } else if (stringBuffer_[startFrom] != nullptr) {
            return;
        }
    }

    auto *pf = klass->GetPandaFile();
    panda_file::ClassDataAccessor cda(*pf, klass->GetFileId());
    [[maybe_unused]] auto annotationFound = cda.EnumerateAnnotation(
        "Lets/annotation/DynamicCall;", [this, pf, startFrom, jsArr, env](panda_file::AnnotationDataAccessor &ada) {
            for (uint32_t i = 0; i < ada.GetCount(); i++) {
                auto adae = ada.GetElement(i);
                auto *elemName = pf->GetStringData(adae.GetNameId()).data;
                if (!utf::IsEqual(utf::CStringAsMutf8("value"), elemName)) {
                    continue;
                }
                auto arr = adae.GetArrayValue();
                auto count = arr.GetCount();
                for (uint32_t j = 0, bufferIndex = startFrom; j < count; j++, bufferIndex++) {
                    panda_file::File::EntityId stringId(arr.Get<uint32_t>(j));
                    auto data = pf->GetStringData(stringId);
                    napi_value jsStr;
                    NAPI_CHECK_FATAL(
                        napi_create_string_utf8(env, utf::Mutf8AsCString(data.data), data.utf16Length, &jsStr));
                    ASSERT(stringBuffer_[bufferIndex] == nullptr);
                    napi_set_element(env, jsArr, bufferIndex, jsStr);
                    stringBuffer_[bufferIndex] = jsStr;
                }
                return true;
            }
            UNREACHABLE();
        });
    ASSERT(annotationFound.has_value());
}

napi_value ConstStringStorage::GetConstantPool()
{
    return GetReferenceValue(Ctx()->GetJSEnv(), jsStringBufferRef_);
}

napi_value ConstStringStorage::InitBuffer(size_t length)
{
    napi_value jsArr;
    NAPI_CHECK_FATAL(napi_create_array_with_length(Ctx()->GetJSEnv(), length, &jsArr));
    NAPI_CHECK_FATAL(napi_create_reference(Ctx()->GetJSEnv(), jsArr, 1, &jsStringBufferRef_));
    stringBuffer_.resize(Ctx()->GetStringBufferSize());
    return jsArr;
}

InteropCtx *ConstStringStorage::Ctx()
{
    return InteropCtx::FromConstStringStorage(this);
}

InteropCtx::InteropCtx(EtsCoroutine *coro, napi_env env)
{
    JSNapiEnvScope envscope(this, env);

    jsEnvForEventLoopCallbacks_ = env;
    PandaEtsVM *vm = coro->GetPandaVM();
    EtsClassLinker *etsClassLinker = vm->GetClassLinker();
    refstor_ = vm->GetGlobalObjectStorage();
    classLinker_ = Runtime::GetCurrent()->GetClassLinker();
    linkerCtx_ = etsClassLinker->GetEtsClassLinkerExtension()->GetBootContext();

    auto *jobQueue = Runtime::GetCurrent()->GetInternalAllocator()->New<JsJobQueue>();
    vm->InitJobQueue(jobQueue);

    CacheClasses(etsClassLinker);

    RegisterBuiltinJSRefConvertors(this);

    {
        auto method = EtsClass::FromRuntimeClass(jsRuntimeClass_)->GetMethod("createFinalizationRegistry");
        ASSERT(method != nullptr);
        auto res = method->GetPandaMethod()->Invoke(coro, nullptr);
        ASSERT(!coro->HasPendingException());
        auto queue = EtsObject::FromCoreType(res.GetAs<ObjectHeader *>());
        jsvalueFregistryRef_ = Refstor()->Add(queue->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
        ASSERT(jsvalueFregistryRef_ != nullptr);

        jsvalueFregistryRegister_ = queue->GetClass()
                                        ->GetMethod("register", "Lstd/core/Object;Lstd/core/Object;Lstd/core/Object;:V")
                                        ->GetPandaMethod();
        ASSERT(jsvalueFregistryRegister_ != nullptr);
    }

    {
        EtsClass *promiseInteropClass =
            EtsClass::FromRuntimeClass(CacheClass(etsClassLinker, "Lstd/interop/js/PromiseInterop;"));
        ASSERT(promiseInteropClass != nullptr);
        promiseInteropConnectMethod_ =
            promiseInteropClass->GetMethod("connectPromise", "Lstd/core/Promise;J:V")->GetPandaMethod();
        ASSERT(promiseInteropConnectMethod_ != nullptr);
    }

    etsProxyRefStorage_ = ets_proxy::SharedReferenceStorage::Create();
    ASSERT(etsProxyRefStorage_.get() != nullptr);

    // Set InteropCtx::DestroyLocalScopeForTopFrame to VM for call it it deoptimization and exception handlers
    vm->SetClearInteropHandleScopesFunction([this](Frame *frame) { this->DestroyLocalScopeForTopFrame(frame); });
    vm->SetDestroyExternalDataFunction(Destroy);
}

void InteropCtx::CacheClasses(EtsClassLinker *etsClassLinker)
{
    jsRuntimeClass_ = CacheClass(etsClassLinker, descriptors::JS_RUNTIME);
    jsValueClass_ = CacheClass(etsClassLinker, descriptors::JS_VALUE);
    jsErrorClass_ = CacheClass(etsClassLinker, descriptors::JS_ERROR);
    objectClass_ = CacheClass(etsClassLinker, descriptors::OBJECT);
    stringClass_ = CacheClass(etsClassLinker, descriptors::STRING);
    bigintClass_ = CacheClass(etsClassLinker, descriptors::BIG_INT);
    undefinedClass_ = CacheClass(etsClassLinker, descriptors::INTERNAL_UNDEFINED);
    promiseClass_ = CacheClass(etsClassLinker, descriptors::PROMISE);
    errorClass_ = CacheClass(etsClassLinker, descriptors::ERROR);
    exceptionClass_ = CacheClass(etsClassLinker, descriptors::EXCEPTION);
    typeClass_ = CacheClass(etsClassLinker, descriptors::TYPE);

    boxIntClass_ = CacheClass(etsClassLinker, descriptors::BOX_INT);
    boxLongClass_ = CacheClass(etsClassLinker, descriptors::BOX_LONG);

    arrayClass_ = CacheClass(etsClassLinker, descriptors::ARRAY);
    arraybufClass_ = CacheClass(etsClassLinker, descriptors::ARRAY_BUFFER);

    arrayAsListIntClass_ = CacheClass(etsClassLinker, descriptors::ARRAY_AS_LIST_INT);

    for (auto descr : FUNCTION_INTERFACE_DESCRIPTORS) {
        functionalInterfaces_.insert(CacheClass(etsClassLinker, descr));
    }
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
        ctx->ThrowETSError(coro, "interop/js throws undefined/null");
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
    auto istk = ctx->interopStk_.GetRecords();
    auto istkIt = istk.rbegin();

    auto printIstkFrames = [&istkIt, &istk](void *fp) {
        while (istkIt != istk.rend() && fp == istkIt->etsFrame) {
            INTEROP_LOG(ERROR) << "<interop> " << (istkIt->descr != nullptr ? istkIt->descr : "unknown");
            istkIt++;
        }
    };

    for (auto stack = StackWalker::Create(coro); stack.HasFrame(); stack.NextFrame()) {
        printIstkFrames(istkIt->etsFrame);
        Method *method = stack.GetMethod();
        INTEROP_LOG(ERROR) << method->GetClass()->GetName() << "." << method->GetName().data << " at "
                           << method->GetLineNumberAndSourceFile(stack.GetBytecodePc());
    }
    ASSERT(istkIt == istk.rend() || istkIt->etsFrame == nullptr);
    printIstkFrames(nullptr);

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

void InteropCtx::Init(EtsCoroutine *coro, napi_env env)
{
    // Initialize InteropCtx in VM ExternalData
    new (InteropCtx::Current(coro)) InteropCtx(coro, env);
}

}  // namespace ark::ets::interop::js
