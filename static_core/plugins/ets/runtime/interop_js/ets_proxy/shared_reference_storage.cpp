/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <cstring>

#include "plugins/ets/runtime/interop_js/ets_proxy/shared_reference_storage.h"
#include "plugins/ets/runtime/interop_js/interop_context.h"
#include "plugins/ets/runtime/interop_js/xgc/xgc.h"

namespace ark::ets::interop::js::ets_proxy {

class SharedReferenceSanity {
public:
    static ALWAYS_INLINE void Kill(SharedReference *ref)
    {
        ASSERT(ref->jsRef_ != nullptr);
        ref->jsRef_ = nullptr;
        ref->ctx_ = nullptr;
        ref->flags_.ClearFlags();
    }

    static ALWAYS_INLINE bool CheckAlive(SharedReference *ref)
    {
        return ref->jsRef_ != nullptr && !ref->flags_.IsEmpty();
    }
};

// NOTE(ipetrov, 20146): It's tempoary solution for SharedReferencesStorage. All napi calls should in native scope
class ScopedNativeCodeThreadIfNeeded {
public:
    explicit ScopedNativeCodeThreadIfNeeded(ManagedThread *thread) : thread_(thread)
    {
        if (thread_->IsInNativeCode()) {
            needToEndNativeCode_ = false;
        } else {
            thread_->NativeCodeBegin();
        }
    }
    NO_COPY_SEMANTIC(ScopedNativeCodeThreadIfNeeded);
    NO_MOVE_SEMANTIC(ScopedNativeCodeThreadIfNeeded);

    ~ScopedNativeCodeThreadIfNeeded()
    {
        if (needToEndNativeCode_) {
            thread_->NativeCodeEnd();
        }
    }

private:
    ManagedThread *thread_ {nullptr};
    bool needToEndNativeCode_ {true};
};

SharedReferenceStorage *SharedReferenceStorage::sharedStorage_ = nullptr;

/* static */
PandaUniquePtr<SharedReferenceStorage> SharedReferenceStorage::Create(PandaEtsVM *vm)
{
    if (sharedStorage_ != nullptr) {
        return nullptr;
    }
    size_t realSize = SharedReferenceStorage::MAX_POOL_SIZE;

    void *data = os::mem::MapRWAnonymousRaw(realSize);
    if (data == nullptr) {
        INTEROP_LOG(FATAL) << "Cannot allocate MemPool";
        return nullptr;
    }
    auto sharedStorage = MakePandaUnique<SharedReferenceStorage>(vm, data, realSize);
    sharedStorage_ = sharedStorage.get();
    vm->AddRootProvider(sharedStorage_);
    return sharedStorage;
}

SharedReferenceStorage::~SharedReferenceStorage()
{
    vm_->RemoveRootProvider(this);
    sharedStorage_ = nullptr;
}

SharedReference *SharedReferenceStorage::GetReference(EtsObject *etsObject) const
{
    os::memory::ReadLockHolder lock(storageLock_);
    ASSERT(SharedReference::HasReference(etsObject));
    return GetItemByIndex(SharedReference::ExtractMaybeIndex(etsObject));
}

napi_status NapiUnwrap(napi_env env, napi_value jsObject, void **result)
{
#if defined(PANDA_JS_ETS_HYBRID_MODE) || defined(PANDA_TARGET_OHOS)
    return napi_xref_unwrap(env, jsObject, result);
#else
    return napi_unwrap(env, jsObject, result);
#endif
}

/* static */
SharedReference *ExtractMaybeReference(napi_env env, napi_value jsObject)
{
    void *data;
    if (UNLIKELY(NapiUnwrap(env, jsObject, &data) != napi_ok)) {
        return nullptr;
    }
    if (UNLIKELY(data == nullptr)) {
        return nullptr;
    }
    // Atomic with acquire order reason: load visibility after shared reference initialization
    return AtomicLoad(static_cast<SharedReference **>(data), std::memory_order_acquire);
}

SharedReference *SharedReferenceStorage::GetReference(napi_env env, napi_value jsObject) const
{
    void *data = ExtractMaybeReference(env, jsObject);
    if (UNLIKELY(data == nullptr)) {
        return nullptr;
    }
    return GetReference(data);
}

SharedReference *SharedReferenceStorage::GetReference(void *data) const
{
    os::memory::ReadLockHolder<os::memory::RWLock, DEBUG_BUILD> lock(storageLock_);
    auto *sharedRef = static_cast<SharedReference *>(data);
    ASSERT(IsValidItem(sharedRef));
    ASSERT(SharedReferenceSanity::CheckAlive(sharedRef));
    return sharedRef;
}

bool SharedReferenceStorage::HasReference(EtsObject *etsObject, napi_env env)
{
    os::memory::ReadLockHolder lock(storageLock_);
    if (!SharedReference::HasReference(etsObject)) {
        return false;
    }
    uint32_t index = SharedReference::ExtractMaybeIndex(etsObject);
    do {
        const SharedReference *currentRef = GetItemByIndex(index);
        if (currentRef->ctx_->GetJSEnv() == env) {
            return true;
        }
        index = currentRef->flags_.GetNextIndex();
    } while (index != 0U);
    return false;
}

napi_value SharedReferenceStorage::GetJsObject(EtsObject *etsObject, napi_env env) const
{
    storageLock_.ReadLock();
    const SharedReference *currentRef = GetItemByIndex(SharedReference::ExtractMaybeIndex(etsObject));
    // CC-OFFNXT(G.CTL.03) false positive
    do {
        if (currentRef->ctx_->GetJSEnv() == env) {
            auto ref = currentRef->jsRef_;
            storageLock_.Unlock();
            ScopedNativeCodeThreadIfNeeded s(EtsCoroutine::GetCurrent());
            napi_value jsValue;
            NAPI_CHECK_FATAL(napi_get_reference_value(env, ref, &jsValue));
            return jsValue;
        }
        uint32_t index = currentRef->flags_.GetNextIndex();
        ASSERT_PRINT(index != 0U, "No JS Object for SharedReference (" << this << ") and napi_env: " << env);
        currentRef = GetItemByIndex(index);
    } while (true);
}

bool SharedReferenceStorage::HasReferenceWithCtx(SharedReference *ref, InteropCtx *ctx) const
{
    uint32_t idx;
    do {
        if (ref->ctx_ == ctx) {
            return true;
        }
        idx = ref->flags_.GetNextIndex();
    } while (idx != 0U);
    return false;
}

static void DeleteSharedReferenceRefCallback([[maybe_unused]] napi_env env, void *data, [[maybe_unused]] void *hint)
{
    delete static_cast<SharedReference **>(data);
}

static SharedReference **CreateXRef(napi_env env, napi_value jsObject, [[maybe_unused]] NapiXRefDirection direction,
                                    napi_ref *result,
                                    const SharedReferenceStorage::PreInitJSObjectCallback &preInitCallback)
{
    ScopedNativeCodeThreadIfNeeded scope(EtsCoroutine::GetCurrent());
    // Deleter can be called after Runtime::Destroy, so InternalAllocator can not be used
    auto **refRef = new SharedReference *(nullptr);
    if (preInitCallback != nullptr) {
        jsObject = preInitCallback(refRef);
    }
#if defined(PANDA_JS_ETS_HYBRID_MODE) || defined(PANDA_TARGET_OHOS)
    if (UNLIKELY(napi_ok !=
                 napi_xref_wrap(env, jsObject, refRef, DeleteSharedReferenceRefCallback, direction, result))) {
        DeleteSharedReferenceRefCallback(env, refRef, nullptr);
        return nullptr;
    }
#else
    if (UNLIKELY((napi_ok != napi_wrap(env, jsObject, refRef, DeleteSharedReferenceRefCallback, nullptr, nullptr)) ||
                 (napi_ok != napi_create_reference(env, jsObject, 1, result)))) {
        DeleteSharedReferenceRefCallback(env, refRef, nullptr);
        return nullptr;
    }
#endif
    return refRef;
}

template <SharedReference::InitFn REF_INIT>
inline SharedReference *SharedReferenceStorage::CreateReference(InteropCtx *ctx, EtsObject *etsObject,
                                                                napi_value jsObject, NapiXRefDirection direction,
                                                                const PreInitJSObjectCallback &preInitCallback)
{
    // NOTE(ipetrov, XGC): XGC should be triggered only in hybrid mode. Need to remove when interop will work only in
    // hybrid mode
#ifdef PANDA_JS_ETS_HYBRID_MODE
    XGC::GetInstance()->TriggerGcIfNeeded(ctx->GetPandaEtsVM()->GetGC());
#endif  // PANDA_JS_ETS_HYBRID_MODE
    napi_ref jsRef;
    // Create XRef before SharedReferenceStorage lock to avoid deadlock situation with JS mutator lock in napi calls
    SharedReference **refRef = CreateXRef(ctx->GetJSEnv(), jsObject, direction, &jsRef, preInitCallback);
    if (refRef == nullptr) {
        auto *coro = EtsCoroutine::GetCurrent();
        if (coro->HasPendingException()) {
            ctx->ForwardEtsException(coro);
        }
        ASSERT(ctx->SanityJSExceptionPending());
        return nullptr;
    }
    os::memory::WriteLockHolder lock(storageLock_);
    SharedReference *sharedRef = AllocItem();
    if (UNLIKELY(sharedRef == nullptr)) {
        ctx->ThrowJSError(ctx->GetJSEnv(), "no free space for SharedReference");
        return nullptr;
    }
    SharedReference *startRef = sharedRef;
    SharedReference *lastRefInChain = nullptr;
    // If EtsObject has been already marked as interop object then add new created SharedReference for a new interop
    // context to chain of references with this EtsObject
    if (etsObject->HasInteropIndex()) {
        lastRefInChain = GetItemByIndex(etsObject->GetInteropIndex());
        startRef = lastRefInChain;
        ASSERT(!HasReferenceWithCtx(startRef, ctx));
        uint32_t index = lastRefInChain->flags_.GetNextIndex();
        while (index != 0U) {
            lastRefInChain = GetItemByIndex(index);
            index = lastRefInChain->flags_.GetNextIndex();
        }
    }
    (sharedRef->*REF_INIT)(ctx, etsObject, jsRef, GetIndexByItem(startRef));
    if (lastRefInChain != nullptr) {
        lastRefInChain->flags_.SetNextIndex(GetIndexByItem(sharedRef));
    }
    // Ref allocated during XGC, so need to mark it on Remark to avoid removing
    if (isXGCinProgress_) {
        refsAllocatedDuringXGC_.insert(sharedRef);
    }
    // Atomic with release order reason: XGC thread should see all writes (initialization of SharedReference) before
    // check initialization status
    AtomicStore(refRef, sharedRef, std::memory_order_release);
    LOG(DEBUG, ETS_INTEROP_JS) << "Alloc shared ref: " << sharedRef;
    return startRef;
}

SharedReference *SharedReferenceStorage::CreateETSObjectRef(InteropCtx *ctx, EtsObject *etsObject, napi_value jsObject,
                                                            const PreInitJSObjectCallback &callback)
{
    return CreateReference<&SharedReference::InitETSObject>(
        ctx, etsObject, jsObject, NapiXRefDirection::NAPI_DIRECTION_DYNAMIC_TO_STATIC, callback);
}

SharedReference *SharedReferenceStorage::CreateJSObjectRef(InteropCtx *ctx, EtsObject *etsObject, napi_value jsObject)
{
    return CreateReference<&SharedReference::InitJSObject>(ctx, etsObject, jsObject,
                                                           NapiXRefDirection::NAPI_DIRECTION_STATIC_TO_DYNAMIC);
}

SharedReference *SharedReferenceStorage::CreateHybridObjectRef(InteropCtx *ctx, EtsObject *etsObject,
                                                               napi_value jsObject)
{
    return CreateReference<&SharedReference::InitHybridObject>(ctx, etsObject, jsObject,
                                                               NapiXRefDirection::NAPI_DIRECTION_HYBRID);
}

void SharedReferenceStorage::RemoveReference(SharedReference *sharedRef)
{
    ASSERT(Size() > 0U);
    LOG(DEBUG, ETS_INTEROP_JS) << "Remove shared ref: " << sharedRef;
    FreeItem(sharedRef);
    SharedReferenceSanity::Kill(sharedRef);
}

void SharedReferenceStorage::DeleteJSRefAndRemoveReference(SharedReference *sharedRef)
{
    NAPI_CHECK_FATAL(napi_delete_reference(sharedRef->ctx_->GetJSEnv(), sharedRef->jsRef_));
    RemoveReference(sharedRef);
}

void SharedReferenceStorage::DeleteUnmarkedReferences(SharedReference *sharedRef)
{
    ASSERT(sharedRef != nullptr);
    ASSERT(!sharedRef->IsEmpty());
    EtsObject *etsObject = sharedRef->GetEtsObject();
    // Get head of refs chain
    auto currentIndex = etsObject->GetInteropIndex();
    SharedReference *currentRef = GetItemByIndex(currentIndex);
    auto nextIndex = currentRef->flags_.GetNextIndex();
    // if nextIndex is 0, then refs chain contains only 1 element, so need to also drop interop index from EtsObject
    if (LIKELY(nextIndex == 0U)) {
        ASSERT(sharedRef == currentRef);
        DeleteJSRefAndRemoveReference(currentRef);
        etsObject->DropInteropIndex();
        return;
    }
    SharedReference *headRef = currentRef;
    // -- Remove all unmarked refs except head: START --
    SharedReference *prevRef = currentRef;
    currentRef = GetItemByIndex(nextIndex);
    nextIndex = currentRef->flags_.GetNextIndex();
    while (nextIndex != 0U) {
        if (!currentRef->IsMarked()) {
            DeleteJSRefAndRemoveReference(currentRef);
            prevRef->flags_.SetNextIndex(nextIndex);
        } else {
            prevRef = currentRef;
        }
        currentRef = GetItemByIndex(nextIndex);
        nextIndex = currentRef->flags_.GetNextIndex();
    }
    if (!currentRef->IsMarked()) {
        DeleteJSRefAndRemoveReference(currentRef);
        prevRef->flags_.SetNextIndex(0U);
    }
    // -- Remove all unmarked refs except head: FINISH --
    // Check mark state for headRef, we need to keep interop index in EtsObject header
    if (!headRef->IsMarked()) {
        NAPI_CHECK_FATAL(napi_delete_reference(headRef->ctx_->GetJSEnv(), headRef->jsRef_));
        nextIndex = headRef->flags_.GetNextIndex();
        if (nextIndex == 0U) {
            // Head reference is alone reference in the chain, so need to drop interop index from EtsObject header
            RemoveReference(headRef);
            etsObject->DropInteropIndex();
            return;
        }
        // All unmarked references were removed from the chain, so reference after head should marked
        SharedReference *firstMarkedRef = GetItemByIndex(nextIndex);
        ASSERT(firstMarkedRef->IsMarked());
        // Copy content from first marked refs to head, so we keep index in EtsObject header
        [[maybe_unused]] auto res = memcpy_s(headRef, sizeof(SharedReference), firstMarkedRef, sizeof(SharedReference));
        ASSERT(res == EOK);
        // Content of marked reference copied to headRef, so now we can remove old firstMarkedRef
        RemoveReference(firstMarkedRef);
    }
}

void SharedReferenceStorage::NotifyXGCStarted()
{
    os::memory::WriteLockHolder lock(storageLock_);
    isXGCinProgress_ = true;
}

void SharedReferenceStorage::NotifyXGCFinished()
{
    os::memory::WriteLockHolder lock(storageLock_);
    isXGCinProgress_ = false;
}

void SharedReferenceStorage::VisitRoots(const GCRootVisitor &visitor)
{
    // No need lock, because we visit roots on pause and we wait XGC ConcurrentSweep for local GCs
    size_t capacity = Capacity();
    for (size_t i = 1U; i < capacity; ++i) {
        SharedReference *ref = GetItemByIndex(i);
        if (!ref->IsEmpty() && ref->flags_.GetNextIndex() == 0U) {
            visitor(mem::GCRoot {mem::RootType::ROOT_VM, ref->GetEtsObject()->GetCoreType()});
        }
    }
}

void SharedReferenceStorage::UpdateRefs()
{
    // No need lock, because we visit roots on pause and we wait XGC ConcurrentSweep for local GCs
    size_t capacity = Capacity();
    for (size_t i = 1U; i < capacity; ++i) {
        SharedReference *ref = GetItemByIndex(i);
        if (ref->IsEmpty()) {
            continue;
        }
        ObjectHeader *obj = ref->GetEtsObject()->GetCoreType();
        if (obj->IsForwarded()) {
            ref->SetETSObject(EtsObject::FromCoreType(ark::mem::GetForwardAddress(obj)));
        }
    }
}

void SharedReferenceStorage::SweepUnmarkedRefs()
{
    os::memory::WriteLockHolder lock(storageLock_);
    ASSERT_PRINT(refsAllocatedDuringXGC_.empty(),
                 "All references allocted during XGC should be proceesed on ReMark phase, unprocessed refs: "
                     << refsAllocatedDuringXGC_.size());
    size_t capacity = Capacity();
    for (size_t i = 1U; i < capacity; ++i) {
        SharedReference *ref = GetItemByIndex(i);
        if (ref->IsEmpty()) {
            continue;
        }
        // If the reference is unmarked, then we immediately remove all unmarked references from related chain
        if (!ref->IsMarked()) {
            DeleteUnmarkedReferences(ref);
        }
    }
}

void SharedReferenceStorage::UnmarkAll()
{
    os::memory::WriteLockHolder lock(storageLock_);
    size_t capacity = Capacity();
    for (size_t i = 1U; i < capacity; ++i) {
        auto *ref = GetItemByIndex(i);
        if (!ref->IsEmpty()) {
            ref->Unmark();
        }
    }
}

bool SharedReferenceStorage::CheckAlive(void *data)
{
    auto *sharedRef = reinterpret_cast<SharedReference *>(data);
    os::memory::ReadLockHolder lock(storageLock_);
    return IsValidItem(sharedRef) && SharedReferenceSanity::CheckAlive(sharedRef);
}

}  // namespace ark::ets::interop::js::ets_proxy
