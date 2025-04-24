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

// A set of types defined and used in platform implementation, owned by the VM
// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
class PANDA_PUBLIC_API EtsPlatformTypes {
public:
    EtsClass *coreBoolean;
    EtsClass *coreByte;
    EtsClass *coreChar;
    EtsClass *coreShort;
    EtsClass *coreInt;
    EtsClass *coreLong;
    EtsClass *coreFloat;
    EtsClass *coreDouble;

    EtsClass *escompatBigint;
    EtsClass *coreFunction;

    EtsClass *escompatError;
    EtsClass *coreOutOfMemoryError;
    EtsClass *coreException;

    EtsClass *coreStringBuilder;

    EtsClass *corePromise;
    EtsClass *coreJob;
    EtsMethod *corePromiseSubscribeOnAnotherPromise;
    EtsClass *corePromiseRef;
    EtsClass *coreWaitersList;
    EtsClass *coreMutex;
    EtsClass *coreEvent;
    EtsClass *coreCondVar;

    EtsClass *escompatArray;
    EtsClass *escompatArrayBuffer;
    EtsClass *containersArrayAsListInt;

    EtsClass *interopJSValue;

    EtsClass *coreTupleN;

    EtsClass *coreStackTraceElement;

    EtsClass *coreFinalizableWeakRef;
    EtsClass *coreFinalizationRegistry;
    EtsMethod *coreFinalizationRegistryExecCleanup;

    EtsClass *coreRuntimeLinker;
    EtsClass *coreBootRuntimeLinker;
    EtsClass *coreAbcRuntimeLinker;
    EtsClass *coreAbcFile;

    EtsClass *coreField;
    EtsClass *coreMethod;
    EtsClass *coreParameter;

    EtsClass *escompatSharedMemory;

private:
    friend class EtsClassLinkerExtension;
    friend class mem::Allocator;

    explicit EtsPlatformTypes(EtsCoroutine *coro);
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

// Obtain EtsPlatformTypes pointer cached in the coroutine
ALWAYS_INLINE inline EtsPlatformTypes const *PlatformTypes(EtsCoroutine *coro)
{
    return coro->GetLocalStorage().Get<EtsCoroutine::DataIdx::ETS_PLATFORM_TYPES_PTR, EtsPlatformTypes *>();
}

// Obtain EtsPlatfromTypes pointer from the VM
EtsPlatformTypes const *PlatformTypes(PandaEtsVM *vm);

// Obtain EtsPlatfromTypes pointer from the current VM
EtsPlatformTypes const *PlatformTypes();

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_H_
