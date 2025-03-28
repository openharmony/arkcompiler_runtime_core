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

template <bool IS_STATIC>
static EtsMethod *FindMethod(EtsClass *klass, char const *name, char const *signature)
{
    if (klass == nullptr) {
        return nullptr;
    }

    EtsMethod *method = [&]() {
        if constexpr (IS_STATIC) {
            return klass->GetStaticMethod(name, signature);
        } else {
            return klass->GetInstanceMethod(name, signature);
        }
    }();

    if (method == nullptr) {
        LOG(ERROR, RUNTIME) << "Method " << name << " is not found in class " << klass->GetDescriptor();
        return nullptr;
    }
    return method;
}

EtsPlatformTypes::EtsPlatformTypes([[maybe_unused]] EtsCoroutine *coro)
{
    // NOLINTNEXTLINE(google-build-using-namespace)
    using namespace panda_file_items::class_descriptors;

    auto classLinker = PandaEtsVM::GetCurrent()->GetClassLinker();
    auto const findType = [classLinker](std::string_view descriptor) { return FindType(classLinker, descriptor); };

    coreBoolean = findType(BOX_BOOLEAN);
    coreByte = findType(BOX_BYTE);
    coreChar = findType(BOX_CHAR);
    coreShort = findType(BOX_SHORT);
    coreInt = findType(BOX_INT);
    coreLong = findType(BOX_LONG);
    coreFloat = findType(BOX_FLOAT);
    coreDouble = findType(BOX_DOUBLE);
    escompatBigint = findType(BIG_INT);
    coreFunction = findType(FUNCTION);

    coreRuntimeLinker = findType(RUNTIME_LINKER);
    coreBootRuntimeLinker = findType(BOOT_RUNTIME_LINKER);
    coreAbcFile = findType(ABC_FILE);
    coreAbcRuntimeLinker = findType(ABC_RUNTIME_LINKER);

    corePromise = findType(PROMISE);
    corePromiseSubscribeOnAnotherPromise =
        FindMethod<false>(corePromise, "subscribeOnAnotherPromise", "Lstd/core/PromiseLike;:V");
    corePromiseRef = findType(PROMISE_REF);
    coreJob = findType(JOB);
    coreWaitersList = findType(WAITERS_LIST);
    coreMutex = findType(MUTEX);
    coreEvent = findType(EVENT);
    coreCondVar = findType(COND_VAR);
    coreQueueSpinlock = findType(QUEUE_SPINLOCK);

    coreException = findType(EXCEPTION);
    escompatError = findType(ERROR);
    coreOutOfMemoryError = findType(OUT_OF_MEMORY_ERROR);
    escompatArrayBuffer = findType(ARRAY_BUFFER);
    coreStringBuilder = findType(STRING_BUILDER);
    containersArrayAsListInt = findType(ARRAY_AS_LIST_INT);
    escompatArray = findType(ARRAY);

    coreField = findType(FIELD);
    coreMethod = findType(METHOD);
    coreParameter = findType(PARAMETER);

    escompatSharedMemory = findType(SHARED_MEMORY);
    interopJSValue = findType(JS_VALUE);

    coreTupleN = findType(TUPLEN);

    coreStackTraceElement = findType(STACK_TRACE_ELEMENT);

    coreFinalizableWeakRef = findType(FINALIZABLE_WEAK_REF);
    coreFinalizationRegistry = findType(FINALIZATION_REGISTRY);
    coreFinalizationRegistryExecCleanup =
        FindMethod<true>(coreFinalizationRegistry, "execCleanup", "[Lstd/core/WeakRef;I:V");
}

}  // namespace ark::ets
