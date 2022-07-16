/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "runtime/cha.h"

#include "libpandabase/events/events.h"
#include "runtime/include/locks.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/rendezvous.h"
#include "runtime/deoptimization.h"

namespace panda {

using os::memory::LockHolder;  // NOLINT(misc-unused-using-decls)

void ClassHierarchyAnalysis::Update(Class *klass)
{
    auto parent = klass->GetBase();

    if (klass->IsInterface()) {
        return;
    }

    if (parent == nullptr) {
        for (const auto &method : klass->GetVTable()) {
            SetHasSingleImplementation(method, true);
        }
        return;
    }

    ASSERT(klass->GetVTableSize() >= parent->GetVTableSize());

    PandaSet<Method *> invalidated_methods;

    for (size_t i = 0; i < parent->GetVTableSize(); ++i) {
        auto method = klass->GetVTable()[i];
        auto parent_method = parent->GetVTable()[i];
        if (method == parent_method || method->IsDefaultInterfaceMethod()) {
            continue;
        }

        if (HasSingleImplementation(parent_method)) {
            EVENT_CHA_INVALIDATE(std::string(parent_method->GetFullName()), klass->GetName());
            invalidated_methods.insert(parent_method);
        }
        UpdateMethod(method);
    }

    for (size_t i = parent->GetVTableSize(); i < klass->GetVTableSize(); ++i) {
        auto method = klass->GetVTable()[i];
        if (method->IsDefaultInterfaceMethod()) {
            continue;
        }
        UpdateMethod(method);
    }

    InvalidateMethods(invalidated_methods);
}

bool ClassHierarchyAnalysis::HasSingleImplementation(Method *method)
{
    LockHolder lock(GetLock());
    return method->HasSingleImplementation();
}

void ClassHierarchyAnalysis::SetHasSingleImplementation(Method *method, bool single_implementation)
{
    LockHolder lock(GetLock());
    method->SetHasSingleImplementation(single_implementation);
}

void ClassHierarchyAnalysis::UpdateMethod(Method *method)
{
    // TODO(msherstennikov): Currently panda is allowed to execute abstract method, thus we cannot propagate single
    // implementation property of the non-abstract method to the all overriden abstract methods.
    SetHasSingleImplementation(method, !method->IsAbstract());
}

void ClassHierarchyAnalysis::InvalidateMethods(const PandaSet<Method *> &methods)
{
    PandaSet<Method *> dependent_methods;

    {
        LockHolder lock(GetLock());
        for (auto method : methods) {
            InvalidateMethod(method, &dependent_methods);
        }
    }

    if (dependent_methods.empty()) {
        return;
    }

    InvalidateCompiledEntryPoint(dependent_methods, true);
}

void ClassHierarchyAnalysis::InvalidateMethod(Method *method, PandaSet<Method *> *dependent_methods) REQUIRES(GetLock())
{
    if (!method->HasSingleImplementation()) {
        return;
    }

    method->SetHasSingleImplementation(false);

    LOG(DEBUG, CLASS_LINKER) << "[CHA] Invalidate method " << method->GetFullName();

    auto it = dependency_map_.find(method);
    if (it == dependency_map_.end()) {
        return;
    }

    for (auto dep_method : it->second) {
        dependent_methods->insert(dep_method);
    }

    dependency_map_.erase(method);
}

void ClassHierarchyAnalysis::AddDependency(Method *callee, Method *caller)
{
    LockHolder lock(GetLock());
    LOG(DEBUG, CLASS_LINKER) << "[CHA] Add dependency: caller " << caller->GetFullName() << ", callee "
                             << callee->GetFullName();
    // Other thread can remove single implementation of the callee method while we compile caller method.
    if (!callee->HasSingleImplementation()) {
        return;
    }
    // There is no sense to store dependencies for abstract methods.
    ASSERT(!callee->IsAbstract());
    dependency_map_[callee].insert(caller);
}

}  // namespace panda
