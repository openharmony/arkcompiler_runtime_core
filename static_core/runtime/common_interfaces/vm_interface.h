/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#ifndef COMMON_INTERFACES_VM_INTERFACE_H
#define COMMON_INTERFACES_VM_INTERFACE_H

#include "common_interfaces/heap/heap_visitor.h"
#include "common_interfaces/thread/mutator.h"

namespace ark::mem {
class PANDA_PUBLIC_API VMInterface {
public:
    virtual void Init() = 0;
    virtual void VisitGlobalRoots(const RefFieldVisitor &visitor) = 0;
    virtual void VisitConcurrentRoots(const RefFieldVisitor &visitor) = 0;
    virtual void VisitAllRoots(const RefFieldVisitor &visitor) = 0;
    virtual void UpdateAndSweep(const WeakRefFieldVisitor &visitor) = 0;
    virtual void VisitPreforwardRoots(const RefFieldVisitor &visitor) = 0;
    virtual void ProcessFinalizationRegistryCleanup() = 0;
    virtual void ProcessReferencesAfterCopy() = 0;
    NO_COPY_SEMANTIC(VMInterface);
    NO_MOVE_SEMANTIC(VMInterface);
    // Default construction
    VMInterface() = default;
    virtual ~VMInterface() = default;
};
}  // namespace ark::mem

#endif
