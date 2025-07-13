/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *tsClass *$1;
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ets/runtime/ets_platform_types.h"
#include "ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_class_linker.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/ets_vm.h"

#include "runtime/include/class_linker-inl.h"

namespace ark::ets {

void EtsPlatformTypes::InitializeCaches()
{
    auto *charClass = this->coreString;
    asciiCharCache_ = EtsTypedObjectArray<EtsString>::Create(charClass, EtsPlatformTypes::ASCII_CHAR_TABLE_SIZE,
                                                             ark::SpaceType::SPACE_TYPE_OBJECT);

    for (uint32_t i = 0; i < EtsPlatformTypes::ASCII_CHAR_TABLE_SIZE; ++i) {
        auto *str = EtsString::CreateNewStringFromCharCode(i);
        asciiCharCache_->Set(i, str);
    }
}

void EtsPlatformTypes::VisitRoots(const GCRootVisitor &visitor) const
{
    if (asciiCharCache_ != nullptr) {
        visitor(mem::GCRoot(mem::RootType::ROOT_VM, asciiCharCache_->GetCoreType()));
    }
}

void EtsPlatformTypes::UpdateCachesVmRefs(const GCRootUpdater &updater) const
{
    if (asciiCharCache_ != nullptr) {
        auto *obj = static_cast<ark::ObjectHeader *>(asciiCharCache_->GetCoreType());
        if (updater(&obj)) {
            asciiCharCache_ = reinterpret_cast<EtsTypedObjectArray<EtsString> *>(obj);
        }
    }
}

EtsPlatformTypes const *PlatformTypes(PandaEtsVM *vm)
{
    return vm->GetClassLinker()->GetEtsClassLinkerExtension()->GetPlatformTypes();
}

EtsPlatformTypes const *PlatformTypes()
{
    return PlatformTypes(PandaEtsVM::GetCurrent());
}

class SuppressErrorHandler : public ClassLinkerErrorHandler {
    void OnError([[maybe_unused]] ClassLinker::Error error, [[maybe_unused]] const PandaString &message) override {}
};

static EtsClass *FindType(EtsClassLinker *classLinker, std::string_view descriptor)
{
    SuppressErrorHandler handler;
    auto bootCtx = classLinker->GetEtsClassLinkerExtension()->GetBootContext();
    auto klass = classLinker->GetClass(descriptor.data(), false, bootCtx, &handler);
    if (klass == nullptr) {
        LOG(ERROR, RUNTIME) << "Cannot find a platform class " << descriptor;
        return nullptr;
    }
    return klass;
}

static EtsMethod *FindMethod(EtsClass *klass, char const *name, char const *signature, bool isStatic)
{
    if (klass == nullptr) {
        return nullptr;
    }

    EtsMethod *method = isStatic ? klass->GetStaticMethod(name, signature) : klass->GetInstanceMethod(name, signature);
    if (method == nullptr) {
        LOG(ERROR, RUNTIME) << "Method " << name << " is not found in class " << klass->GetDescriptor();
        return nullptr;
    }
    return method;
}

EtsPlatformTypes::Entry const *EtsPlatformTypes::GetTypeEntry(const uint8_t *descriptor) const
{
    if (auto it = entryTable_.find(descriptor); it != entryTable_.end()) {
        return &it->second;
    }
    return nullptr;
}

void EtsPlatformTypes::PreloadType(EtsClassLinker *linker, EtsClass **slot, std::string_view descriptor)
{
    auto cls = FindType(linker, descriptor);
    if (cls == nullptr) {
        return;
    }
    *slot = cls;

    size_t offset = ToUintPtr(slot) - ToUintPtr(this);
    ASSERT(IsAligned(offset, sizeof(uintptr_t)));
    Entry entry = {offset / sizeof(uintptr_t)};
    entryTable_.insert({utf::CStringAsMutf8(cls->GetDescriptor()), entry});
}

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid logic
EtsPlatformTypes::EtsPlatformTypes([[maybe_unused]] EtsCoroutine *coro)
{
    // NOLINTNEXTLINE(google-build-using-namespace)
    using namespace panda_file_items::class_descriptors;

    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    [[maybe_unused]] size_t orderOffset = 0;
    auto const updateOffset = [this, &orderOffset](auto *slot) {
        size_t newOffset = ToUintPtr(slot) - ToUintPtr(this);
        ASSERT_PRINT(newOffset == 0 || (newOffset == orderOffset + sizeof(uintptr_t)),
                     "type preloading should follow the definition order");
        orderOffset = newOffset;
    };
    auto const findType = [this, classLinker, &updateOffset](EtsClass *ark::ets::EtsPlatformTypes::*field,
                                                             std::string_view descriptor) {
        auto slot = &(this->*field);
        PreloadType(classLinker, slot, descriptor);
        updateOffset(slot);
    };
    auto const findMethod = [this, &updateOffset](EtsMethod *ark::ets::EtsPlatformTypes::*field, EtsClass *cls,
                                                  char const *name, char const *signature, bool isStatic) {
        auto slot = &(this->*field);
        *slot = FindMethod(cls, name, signature, isStatic);
        updateOffset(slot);
    };

    findType(&EtsPlatformTypes::coreObject, OBJECT);
    findType(&EtsPlatformTypes::coreClass, CLASS);
    findType(&EtsPlatformTypes::coreString, STRING);
    findType(&EtsPlatformTypes::coreLineString, LINE_STRING);
    findType(&EtsPlatformTypes::coreSlicedString, SLICED_STRING);
    findType(&EtsPlatformTypes::coreTreeString, TREE_STRING);

    findType(&EtsPlatformTypes::coreBoolean, BOX_BOOLEAN);
    findType(&EtsPlatformTypes::coreByte, BOX_BYTE);
    findType(&EtsPlatformTypes::coreChar, BOX_CHAR);
    findType(&EtsPlatformTypes::coreShort, BOX_SHORT);
    findType(&EtsPlatformTypes::coreInt, BOX_INT);
    findType(&EtsPlatformTypes::coreLong, BOX_LONG);
    findType(&EtsPlatformTypes::coreFloat, BOX_FLOAT);
    findType(&EtsPlatformTypes::coreDouble, BOX_DOUBLE);

    findType(&EtsPlatformTypes::escompatBigint, BIG_INT);
    findType(&EtsPlatformTypes::escompatError, ERROR);
    findType(&EtsPlatformTypes::coreFunction, FUNCTION);

    for (size_t i = 0; i < coreFunctions.size(); ++i) {
        PandaString descr = "Lstd/core/Function" + PandaString(std::to_string(i)) + ";";
        PreloadType(classLinker, &coreFunctions[i], descr);
        updateOffset(&coreFunctions[i]);
    }
    for (size_t i = 0; i < coreFunctionRs.size(); ++i) {
        PandaString descr = "Lstd/core/FunctionR" + PandaString(std::to_string(i)) + ";";
        PreloadType(classLinker, &coreFunctionRs[i], descr);
        updateOffset(&coreFunctionRs[i]);
    }

    findType(&EtsPlatformTypes::coreTupleN, TUPLEN);

    findType(&EtsPlatformTypes::coreRuntimeLinker, RUNTIME_LINKER);
    findType(&EtsPlatformTypes::coreBootRuntimeLinker, BOOT_RUNTIME_LINKER);
    findType(&EtsPlatformTypes::coreAbcRuntimeLinker, ABC_RUNTIME_LINKER);
    findType(&EtsPlatformTypes::coreMemoryRuntimeLinker, MEMORY_RUNTIME_LINKER);
    findType(&EtsPlatformTypes::coreAbcFile, ABC_FILE);

    findType(&EtsPlatformTypes::coreOutOfMemoryError, OUT_OF_MEMORY_ERROR);
    findType(&EtsPlatformTypes::coreException, EXCEPTION);
    findType(&EtsPlatformTypes::coreStackTraceElement, STACK_TRACE_ELEMENT);

    findType(&EtsPlatformTypes::coreStringBuilder, STRING_BUILDER);

    findType(&EtsPlatformTypes::corePromise, PROMISE);
    findType(&EtsPlatformTypes::coreJob, JOB);

    findMethod(&EtsPlatformTypes::corePromiseSubscribeOnAnotherPromise, corePromise, "subscribeOnAnotherPromise",
               "Lstd/core/PromiseLike;:V", false);
    findType(&EtsPlatformTypes::corePromiseRef, PROMISE_REF);
    findType(&EtsPlatformTypes::coreWaitersList, WAITERS_LIST);
    findType(&EtsPlatformTypes::coreMutex, MUTEX);
    findType(&EtsPlatformTypes::coreEvent, EVENT);
    findType(&EtsPlatformTypes::coreCondVar, COND_VAR);
    findType(&EtsPlatformTypes::coreQueueSpinlock, QUEUE_SPINLOCK);

    findType(&EtsPlatformTypes::coreFinalizableWeakRef, FINALIZABLE_WEAK_REF);
    findType(&EtsPlatformTypes::coreFinalizationRegistry, FINALIZATION_REGISTRY);
    findMethod(&EtsPlatformTypes::coreFinalizationRegistryExecCleanup, coreFinalizationRegistry, "execCleanup",
               "[Lstd/core/WeakRef;I:V", true);

    findType(&EtsPlatformTypes::escompatArray, ARRAY);
    findMethod(&EtsPlatformTypes::escompatArrayPush, escompatArray, "pushSingle", "Lstd/core/Object;:V", false);
    findMethod(&EtsPlatformTypes::escompatArrayPop, escompatArray, "pop", ":Lstd/core/Object;", false);
    findType(&EtsPlatformTypes::escompatArrayBuffer, ARRAY_BUFFER);
    findType(&EtsPlatformTypes::containersArrayAsListInt, ARRAY_AS_LIST_INT);
    findType(&EtsPlatformTypes::escompatRecord, RECORD);
    findMethod(&EtsPlatformTypes::escompatRecordGetter, escompatRecord, GET_INDEX_METHOD, nullptr, false);
    findMethod(&EtsPlatformTypes::escompatRecordSetter, escompatRecord, SET_INDEX_METHOD, nullptr, false);

    findType(&EtsPlatformTypes::interopJSValue, JS_VALUE);

    findType(&EtsPlatformTypes::coreField, FIELD);
    findType(&EtsPlatformTypes::coreMethod, METHOD);
    findType(&EtsPlatformTypes::coreParameter, PARAMETER);
    findType(&EtsPlatformTypes::coreClassType, CLASS_TYPE);

    findType(&EtsPlatformTypes::escompatProcess, PROCESS);
    findMethod(&EtsPlatformTypes::escompatProcessListUnhandledJobs, escompatProcess, "listUnhandledJobs",
               "Lescompat/Array;:V", true);
    findMethod(&EtsPlatformTypes::escompatProcessListUnhandledPromises, escompatProcess, "listUnhandledPromises",
               "Lescompat/Array;:V", true);

    findType(&EtsPlatformTypes::coreTuple, TUPLE);
    findType(&EtsPlatformTypes::escompatRegExpExecArray, REG_EXP_EXEC_ARRAY);
    findType(&EtsPlatformTypes::escompatJsonReplacer, JSON_REPLACER);
    if (LIKELY(Runtime::GetOptions().IsUseStringCaches())) {
        InitializeCaches();
    }
}

}  // namespace ark::ets
