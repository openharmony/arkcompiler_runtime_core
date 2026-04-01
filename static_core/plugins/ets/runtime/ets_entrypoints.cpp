/**
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_entrypoints.h"
#include "plugins/ets/runtime/types/ets_async_context.h"

#include "runtime/execution/job_execution_context.h"
#include "runtime/execution/job_launch.h"
#include "include/coretypes/string.h"
#include "runtime/include/managed_thread.h"
#include "include/object_header.h"
#include "libarkfile/shorty_iterator.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_runtime_interface.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_string_case_conversion.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_to_string_cache.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_intrinsics_helpers.h"
#include "plugins/ets/runtime/intrinsics/helpers/intrinsic_await_promise_impl.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/types/ets_escompat_array.h"
#include "plugins/ets/runtime/ets_stubs-inl.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_string_builder.h"
#include "runtime/arch/helpers.h"
#include "runtime/execution/job_worker_group.h"
#include "runtime/interpreter/vregister_iterator.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "runtime/include/class_linker-inl.h"
#include "types/ets_object.h"
#include "intrinsics.h"
#include "types/ets_promise.h"

#include "execution/stackless/stackless_job_manager.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_async_context-inl.h"
#include "plugins/ets/runtime/job_queue.h"
#include "runtime/execution/job_events.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/execution/stackless/suspendable_job.h"

namespace ark::ets {

using TypeId = panda_file::Type::TypeId;

#if defined(__clang__)
#pragma clang diagnostic push
// CC-OFFNXT(warning_suppression) gcc false positive
#pragma clang diagnostic ignored "-Wgnu-label-as-value"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
// CC-OFFNXT(warning_suppression) gcc false positive
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

static inline bool Launch(EtsExecutionContext *executionCtx, Method *method, const EtsHandle<EtsPromise> &promiseHandle,
                          PandaVector<Value> &&args)
{
    ASSERT(executionCtx != nullptr);
    PandaEtsVM *etsVm = executionCtx->GetPandaVM();
    auto *jobMan = JobExecutionContext::CastFromMutator(executionCtx->GetMT())->GetManager();
    auto *promiseRef = etsVm->GetGlobalObjectStorage()->Add(promiseHandle.GetPtr(), mem::Reference::ObjectType::GLOBAL);
    auto *evt = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(promiseRef, jobMan);
    // create the job and put it to the ready queue
    auto epInfo = Job::ManagedEntrypointInfo {evt, method, std::move(args)};
    auto *job = jobMan->CreateJob(method->GetFullName(), std::move(epInfo), EtsCoroutine::LAUNCH);
    auto launchResult = jobMan->Launch(job, LaunchParams {job->GetPriority()});
    if (UNLIKELY(launchResult != LaunchResult::OK)) {
        // OOM
        if (launchResult == LaunchResult::RESOURCE_LIMIT_EXCEED) {
            jobMan->DestroyJob(job);
        }
        etsVm->GetGlobalObjectStorage()->Remove(promiseRef);
    }
    return launchResult == LaunchResult::OK;
}

void LaunchCoroutine(Method *method, ObjectHeader *obj, uint64_t *args, ObjectHeader *thisObj)
{
    auto *promise = reinterpret_cast<EtsPromise *>(obj);
    ASSERT(promise != nullptr);

    PandaVector<Value> values;
    arch::ArgReaderStack<RUNTIME_ARCH> argReader(reinterpret_cast<uint8_t *>(args));
    arch::ValueWriter writer(&values);
    if (thisObj != nullptr) {
        ASSERT(!method->IsStatic());
        // Add this for virtual call
        values.push_back(Value(thisObj));
    } else {
        if (!method->IsStatic()) {
            auto pThisObj = const_cast<ObjectHeader **>((argReader).template ReadPtr<ObjectHeader *>());
            values.push_back(Value(*pThisObj));
        }
    }
    ARCH_COPY_METHOD_ARGS(method, argReader, writer);

    auto *executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsPromise> promiseHandle(executionCtx, promise);
    // NOTE(panferovi): should be fixed in #19443
    ASSERT(promiseHandle->GetMutex(executionCtx) == nullptr);
    ASSERT(promiseHandle->GetEvent(executionCtx) == nullptr);
    // NOTE(panferovi): issue with raw args and thisObj??
    auto *mutex = EtsMutex::Create(executionCtx);
    promiseHandle->SetMutex(executionCtx, mutex);
    auto *event = EtsEventWithDependencies::Create(executionCtx);
    promiseHandle->SetEvent(executionCtx, event);
    bool successfulLaunch = Launch(executionCtx, method, promiseHandle, std::move(values));
    if (UNLIKELY(!successfulLaunch)) {
        HandlePendingException();
        UNREACHABLE();
    }
}

extern "C" void CreateLaunchStaticCoroutineEntrypoint(Method *method, ObjectHeader *obj, uint64_t *args)
{
    LaunchCoroutine(method, obj, args, nullptr);
}

extern "C" void CreateLaunchVirtualCoroutineEntrypoint(Method *method, ObjectHeader *obj, uint64_t *args,
                                                       ObjectHeader *thisObj)
{
    LaunchCoroutine(method, obj, args, thisObj);
}

template <BytecodeInstruction::Format FORMAT>
ObjectHeader *LaunchFromInterpreterImpl(Method *method, Frame *frame, const uint8_t *pc)
{
    EtsPromise *promise = EtsPromise::Create();
    if (UNLIKELY(promise == nullptr)) {
        return nullptr;
    }

    auto numArgs = method->GetNumArgs();
    auto args = PandaVector<Value> {numArgs};
    auto frameHandler = StaticFrameHandler(frame);
    auto vregIter = interpreter::VRegisterIterator<FORMAT> {BytecodeInstruction(pc), frame};
    for (decltype(numArgs) i = 0; i < numArgs; ++i) {
        args[i] = Value::FromVReg(frameHandler.GetVReg(vregIter.GetVRegIdx(i)));
    }

    auto *executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsPromise> promiseHandle(executionCtx, promise);
    bool successfulLaunch = Launch(executionCtx, method, promiseHandle, std::move(args));
    if (UNLIKELY(!successfulLaunch)) {
        return nullptr;
    }
    frame->GetAccAsVReg().SetReference(promiseHandle.GetPtr());
    return promiseHandle.GetPtr();
}

extern "C" ObjectHeader *StringBuilderAppendLongEntrypoint(ObjectHeader *sb, int64_t v)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendLong(sb, v);
}

extern "C" ObjectHeader *StringBuilderAppendCharEntrypoint(ObjectHeader *sb, uint16_t ch)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendChar(sb, ch);
}

extern "C" ObjectHeader *StringBuilderAppendBoolEntrypoint(ObjectHeader *sb, uint8_t v)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendBool(sb, ToEtsBoolean(static_cast<bool>(v)));
}

extern "C" ObjectHeader *StringBuilderAppendStringEntrypoint(ObjectHeader *sb, ObjectHeader *str)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendString(sb, reinterpret_cast<EtsString *>(str));
}

extern "C" ObjectHeader *StringBuilderAppendString2Entrypoint(ObjectHeader *sb, ObjectHeader *str0, ObjectHeader *str1)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendStrings(sb, reinterpret_cast<EtsString *>(str0), reinterpret_cast<EtsString *>(str1));
}

extern "C" ObjectHeader *StringBuilderAppendString3Entrypoint(ObjectHeader *sb, ObjectHeader *str0, ObjectHeader *str1,
                                                              ObjectHeader *str2)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendStrings(sb, reinterpret_cast<EtsString *>(str0), reinterpret_cast<EtsString *>(str1),
                                      reinterpret_cast<EtsString *>(str2));
}

extern "C" ObjectHeader *StringBuilderAppendString4Entrypoint(ObjectHeader *sb, ObjectHeader *str0, ObjectHeader *str1,
                                                              ObjectHeader *str2, ObjectHeader *str3)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendStrings(sb, reinterpret_cast<EtsString *>(str0), reinterpret_cast<EtsString *>(str1),
                                      reinterpret_cast<EtsString *>(str2), reinterpret_cast<EtsString *>(str3));
}

extern "C" ObjectHeader *StringBuilderAppendNullStringEntrypoint(ObjectHeader *sb)
{
    ASSERT(sb != nullptr);
    return StringBuilderAppendNullString(sb);
}

extern "C" bool IsClassValueTypedEntrypoint(Class *cls)
{
    return EtsClass::FromRuntimeClass(cls)->IsValueTyped();
}

extern "C" bool CompareETSValueTypedEntrypoint(ManagedThread *thread, ObjectHeader *obj1, ObjectHeader *obj2)
{
    auto eobj1 = EtsObject::FromCoreType(obj1);
    auto eobj2 = EtsObject::FromCoreType(obj2);
    return EtsValueTypedEquals(EtsExecutionContext::FromMT(thread), eobj1, eobj2);
}

extern "C" EtsString *EtsGetTypeofEntrypoint(ManagedThread *thread, ObjectHeader *obj)
{
    EtsObject *eobj = EtsObject::FromCoreType(obj);
    return EtsGetTypeof(EtsExecutionContext::FromMT(thread), eobj);
}

extern "C" bool EtsIstrueEntrypoint(ManagedThread *thread, ObjectHeader *obj)
{
    EtsObject *eobj = EtsObject::FromCoreType(obj);
    return EtsIstrue(EtsExecutionContext::FromMT(thread), eobj);
}

extern "C" ObjectHeader *StringBuilderToStringEntrypoint(ObjectHeader *sb)
{
    ASSERT(sb != nullptr);
    return StringBuilderToString(sb)->GetCoreType();
}

extern "C" ObjectHeader *LongToStringDecimalEntrypoint(ObjectHeader *cache, int64_t number)
{
    if (UNLIKELY(cache == nullptr)) {
        return LongToStringDecimalNoCacheEntrypoint(number);
    }
    return LongToStringCache::FromCoreType(cache)->GetOrCache(EtsExecutionContext::GetCurrent(), number)->GetCoreType();
}

extern "C" ObjectHeader *LongToStringDecimalStoreEntrypoint(ObjectHeader *elem, int64_t number)
{
    auto *cache = ManagedThread::GetCurrent()->GetLongToStringCache();
    if (UNLIKELY(cache == nullptr)) {
        return LongToStringDecimalNoCacheEntrypoint(number);
    }
    return LongToStringCache::FromCoreType(cache)
        ->CacheAndGetNoCheck(EtsExecutionContext::GetCurrent(), number, elem)
        ->GetCoreType();
}

extern "C" ObjectHeader *LongToStringDecimalNoCacheEntrypoint(int64_t number)
{
    return LongToStringCache::GetNoCache(number)->GetCoreType();
}

extern "C" ObjectHeader *FloatToStringDecimalEntrypoint(ObjectHeader *cache, uint32_t number)
{
    if (UNLIKELY(cache == nullptr)) {
        return FloatToStringDecimalNoCacheEntrypoint(number);
    }
    return FloatToStringCache::FromCoreType(cache)
        ->GetOrCache(EtsExecutionContext::GetCurrent(), bit_cast<float>(number))
        ->GetCoreType();
}

extern "C" ObjectHeader *FloatToStringDecimalStoreEntrypoint(ObjectHeader *elem, uint32_t number)
{
    auto *cache = ManagedThread::GetCurrent()->GetFloatToStringCache();
    if (UNLIKELY(cache == nullptr)) {
        return FloatToStringDecimalNoCacheEntrypoint(number);
    }
    return FloatToStringCache::FromCoreType(cache)
        ->CacheAndGetNoCheck(EtsExecutionContext::GetCurrent(), bit_cast<float>(number), elem)
        ->GetCoreType();
}

extern "C" ObjectHeader *FloatToStringDecimalNoCacheEntrypoint(uint32_t number)
{
    return FloatToStringCache::GetNoCache(bit_cast<float>(number))->GetCoreType();
}

extern "C" ObjectHeader *DoubleToStringDecimalEntrypoint(ObjectHeader *cache, uint64_t number)
{
    if (UNLIKELY(cache == nullptr)) {
        return DoubleToStringDecimalNoCacheEntrypoint(number);
    }
    return DoubleToStringCache::FromCoreType(cache)
        ->GetOrCache(EtsExecutionContext::GetCurrent(), bit_cast<double>(number))
        ->GetCoreType();
}

extern "C" ObjectHeader *DoubleToStringDecimalStoreEntrypoint(ObjectHeader *elem, uint64_t number)
{
    auto *cache = ManagedThread::GetCurrent()->GetDoubleToStringCache();
    if (UNLIKELY(cache == nullptr)) {
        return DoubleToStringDecimalNoCacheEntrypoint(number);
    }
    return DoubleToStringCache::FromCoreType(cache)
        ->CacheAndGetNoCheck(EtsExecutionContext::GetCurrent(), bit_cast<double>(number), elem)
        ->GetCoreType();
}

extern "C" ObjectHeader *DoubleToStringDecimalNoCacheEntrypoint(uint64_t number)
{
    return DoubleToStringCache::GetNoCache(bit_cast<double>(number))->GetCoreType();
}

extern "C" void BeginGeneralNativeMethod()
{
    auto *mThread = ManagedThread::GetCurrent();
    ASSERT(mThread != nullptr);
    auto *executionCtx = EtsExecutionContext::FromMT(mThread);
    ASSERT(executionCtx != nullptr);
    auto *storage = executionCtx->GetPandaAniEnv()->GetEtsReferenceStorage();

    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr uint32_t MAX_LOCAL_REF = 4096;
    if (UNLIKELY(!storage->PushLocalEtsFrame(MAX_LOCAL_REF))) {
        LOG(FATAL, RUNTIME) << "eTS NAPI push local frame failed";
    }

    mThread->NativeCodeBegin();
}

extern "C" void EndGeneralNativeMethodPrim()
{
    auto *mThread = ManagedThread::GetCurrent();
    ASSERT(mThread != nullptr);
    auto *executionCtx = EtsExecutionContext::FromMT(mThread);
    ASSERT(executionCtx != nullptr);
    auto *storage = executionCtx->GetPandaAniEnv()->GetEtsReferenceStorage();

    mThread->NativeCodeEnd();
    storage->PopLocalEtsFrame(EtsReference::GetUndefined());
}

extern "C" ObjectHeader *EndGeneralNativeMethodObj(ark::ets::EtsReference *etsRef)
{
    auto *mThread = ManagedThread::GetCurrent();
    ASSERT(mThread != nullptr);
    auto *executionCtx = EtsExecutionContext::FromMT(mThread);
    ASSERT(executionCtx != nullptr);
    auto *storage = executionCtx->GetPandaAniEnv()->GetEtsReferenceStorage();
    mThread->NativeCodeEnd();
    ObjectHeader *ret = nullptr;
    if (LIKELY(!mThread->HasPendingException())) {
        ret = storage->GetEtsObject(etsRef)->GetCoreType();
    }

    storage->PopLocalEtsFrame(EtsReference::GetUndefined());
    return ret;
}

extern "C" void BeginQuickNativeMethod()
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto *storage = executionCtx->GetPandaAniEnv()->GetEtsReferenceStorage();

    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr uint32_t MAX_LOCAL_REF = 4096;
    if (UNLIKELY(!storage->PushLocalEtsFrame(MAX_LOCAL_REF))) {
        LOG(FATAL, RUNTIME) << "eTS NAPI push local frame failed";
    }
}

extern "C" void EndQuickNativeMethodPrim()
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    ASSERT(executionCtx != nullptr);
    auto *storage = executionCtx->GetPandaAniEnv()->GetEtsReferenceStorage();

    storage->PopLocalEtsFrame(EtsReference::GetUndefined());
}

extern "C" ObjectHeader *EndQuickNativeMethodObj(ark::ets::EtsReference *etsRef)
{
    auto *mThread = ManagedThread::GetCurrent();
    ASSERT(mThread != nullptr);
    auto *executionCtx = EtsExecutionContext::FromMT(mThread);
    ASSERT(executionCtx != nullptr);
    auto *storage = executionCtx->GetPandaAniEnv()->GetEtsReferenceStorage();
    ObjectHeader *ret = nullptr;
    if (LIKELY(!mThread->HasPendingException())) {
        ret = storage->GetEtsObject(etsRef)->GetCoreType();
    }

    storage->PopLocalEtsFrame(EtsReference::GetUndefined());
    return ret;
}

extern "C" coretypes::String *CreateStringFromCharCodeSingleNoCacheEntrypoint(int32_t charCode)
{
    // We could use `ark::ets::intrinsics::StdCoreStringFromCharCodeSingle` as the entrypoint, it works, but
    // that function lookups the char code in the cache while we are sure the char code is not cached.
    return EtsString::CreateNewStringFromCharCode(charCode)->GetCoreType();
}

extern "C" int32_t WriteStringToMem(int64_t buf, ObjectHeader *s)
{
    auto str = reinterpret_cast<EtsString *>(s);
    auto addr = reinterpret_cast<void *>(buf);
    auto size = static_cast<uint32_t>(str->GetUtf8Length());

    if (str->IsUtf16()) {
        if (size != str->CopyDataRegionUtf8(addr, 0, size, size)) {
            return -1;
        }
    } else {
        PandaVector<uint8_t> tree8Buf;
        if (memcpy_s(addr, size, str->IsTreeString() ? str->GetTreeStringDataMUtf8(tree8Buf) : str->GetDataMUtf8(),
                     size) != 0) {
            return -1;
        }
    }
    return size;
}

extern "C" ObjectHeader *CreateStringFromMem(int64_t buf, int32_t len)
{
    auto str = EtsString::CreateFromUtf8(reinterpret_cast<const char *>(buf), static_cast<uint32_t>(len));
    return reinterpret_cast<ObjectHeader *>(str);
}

extern "C" int32_t GetStringSizeInBytes(ObjectHeader *str)
{
    return reinterpret_cast<EtsString *>(str)->GetUtf8Length();
}

extern "C" bool SameValueZeroEntrypoint(ObjectHeader *o1, ObjectHeader *o2)
{
    return ark::ets::intrinsics::helpers::SameValueZero(EtsExecutionContext::GetCurrent(), EtsObject::FromCoreType(o1),
                                                        EtsObject::FromCoreType(o2));
}

extern "C" EtsBoolean EtsStringEqualsEntrypoint(coretypes::String *str1, coretypes::String *str2)
{
    if (str1 == nullptr) {
        return UINT8_C(str2 == nullptr);
    }
    if (str1 == str2) {
        return UINT8_C(1);
    }
    if (str2 == nullptr) {
        return UINT8_C(0);
    }
    return ToEtsBoolean(EtsString::StringsAreEqualWithCache(str1, str2));
}

extern "C" EtsBoolean EtsDefaultLocaleAllowsFastLatinCaseConversion()
{
    return ToEtsBoolean(ark::ets::intrinsics::caseconversion::DefaultLocaleAllowsFastLatinCaseConversion());
}

extern "C" EtsObject *EtsStacklessInitAsyncContextEntrypoint(EtsPromise *promise, uint32_t refCount, uint32_t primCount,
                                                             uint32_t pc)
{
    return ark::ets::intrinsics::helpers::EtsAwaitPromiseImpl(promise, refCount, primCount, pc);
}

}  // namespace ark::ets
