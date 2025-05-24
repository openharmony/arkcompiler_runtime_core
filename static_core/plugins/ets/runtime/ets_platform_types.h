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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_H_
#define PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_H_

#include "plugins/ets/runtime/ets_coroutine.h"

namespace ark::ets {

class EtsClass;
class EtsMethod;
class EtsCoroutine;
class EtsClassLinker;
template <typename T>
class EtsTypedObjectArray;

// A set of types defined and used in platform implementation, owned by the VM
// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
class PANDA_PUBLIC_API EtsPlatformTypes {
public:
    // Classes should follow the common naming schema

    // Arity threshold for functional types
    static constexpr uint32_t CORE_FUNCTION_ARITY_THRESHOLD = 17U;
    static constexpr uint32_t ASCII_CHAR_TABLE_SIZE = 128;

    /* Core runtime type system */
    EtsClass *coreObject {};  // IsObjectClass
    EtsClass *coreClass {};   // IsClassClass
    EtsClass *coreString {};  // IsStringClass

    /* ets numeric classes */
    EtsClass *coreBoolean {};
    EtsClass *coreByte {};
    EtsClass *coreChar {};
    EtsClass *coreShort {};
    EtsClass *coreInt {};
    EtsClass *coreLong {};
    EtsClass *coreFloat {};
    EtsClass *coreDouble {};

    /* ets base language classes */
    EtsClass *escompatBigint {};
    EtsClass *escompatError {};
    EtsClass *coreFunction {};
    std::array<EtsClass *, CORE_FUNCTION_ARITY_THRESHOLD> coreFunctions {};
    std::array<EtsClass *, CORE_FUNCTION_ARITY_THRESHOLD> coreFunctionRs {};
    EtsClass *coreTupleN {};

    /* Runtime linkage classes */
    EtsClass *coreRuntimeLinker {};
    EtsClass *coreBootRuntimeLinker {};
    EtsClass *coreAbcRuntimeLinker {};
    EtsClass *coreMemoryRuntimeLinker {};
    EtsClass *coreAbcFile {};

    /* Error handling */
    EtsClass *coreOutOfMemoryError {};
    EtsClass *coreException {};
    EtsClass *coreStackTraceElement {};

    /* StringBuilder */
    EtsClass *coreStringBuilder {};

    /* Concurrency classes */
    EtsClass *corePromise {};
    EtsClass *coreJob {};
    EtsMethod *corePromiseSubscribeOnAnotherPromise {};
    EtsClass *corePromiseRef {};
    EtsClass *coreWaitersList {};
    EtsClass *coreMutex {};
    EtsClass *coreEvent {};
    EtsClass *coreCondVar {};
    EtsClass *coreQueueSpinlock {};

    /* Finalization */
    EtsClass *coreFinalizableWeakRef {};
    EtsClass *coreFinalizationRegistry {};
    EtsMethod *coreFinalizationRegistryExecCleanup {};

    /* Containers */
    EtsClass *escompatArray {};
    EtsMethod *escompatArrayPush {};
    EtsMethod *escompatArrayPop {};
    EtsClass *escompatArrayBuffer {};
    EtsClass *containersArrayAsListInt {};
    EtsClass *escompatRecord {};
    EtsMethod *escompatRecordGetter {};
    EtsMethod *escompatRecordSetter {};

    /* InteropJS */
    EtsClass *interopJSValue {};

    /* TypeAPI */
    EtsClass *coreField {};
    EtsClass *coreMethod {};
    EtsClass *coreParameter {};
    EtsClass *coreClassType {};

    /* escompat.Process */
    EtsClass *escompatProcess {};
    EtsMethod *escompatProcessListUnhandledJobs {};
    EtsMethod *escompatProcessListUnhandledPromises {};

    EtsClass *coreTuple {};
    EtsClass *escompatRegExpExecArray {};
    EtsClass *escompatJsonReplacer {};

    struct Entry {
        size_t slotIndex {};
    };

    /* Internal Caches */
    void InitializeCaches();
    void VisitRoots(const GCRootVisitor &visitor) const;
    void UpdateCachesVmRefs(const GCRootUpdater &updater) const;
    EtsTypedObjectArray<EtsString> *GetAsciiCacheTable() const
    {
        return asciiCharCache_;
    }
    Entry const *GetTypeEntry(const uint8_t *descriptor) const;

    EtsClass *escompatMap {};
    EtsClass *escompatMapEntry {};

public:
    static constexpr size_t GetCoreLongTypeOffset()
    {
        return MEMBER_OFFSET(EtsPlatformTypes, coreLong);
    }
    static constexpr size_t GetCoreFloatTypeOffset()
    {
        return MEMBER_OFFSET(EtsPlatformTypes, coreFloat);
    }
    static constexpr size_t GetCoreDoubleTypeOffset()
    {
        return MEMBER_OFFSET(EtsPlatformTypes, coreDouble);
    }

private:
    friend class EtsClassLinkerExtension;
    friend class mem::Allocator;
    mutable EtsTypedObjectArray<EtsString> *asciiCharCache_ {nullptr};
    void PreloadType(EtsClassLinker *linker, EtsClass **slot, std::string_view descriptor);
    PandaUnorderedMap<const uint8_t *, Entry, utf::Mutf8Hash, utf::Mutf8Equal> entryTable_;

    explicit EtsPlatformTypes(EtsCoroutine *coro);
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

// Obtain EtsPlatformTypes pointer cached in the coroutine
ALWAYS_INLINE inline EtsPlatformTypes const *PlatformTypes(EtsCoroutine *coro)
{
    ASSERT(coro != nullptr);
    return coro->GetLocalStorage().Get<EtsCoroutine::DataIdx::ETS_PLATFORM_TYPES_PTR, EtsPlatformTypes *>();
}

// Obtain EtsPlatfromTypes pointer from the VM
EtsPlatformTypes const *PlatformTypes(PandaEtsVM *vm);

// Obtain EtsPlatfromTypes pointer from the current VM
EtsPlatformTypes const *PlatformTypes();

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_H_
