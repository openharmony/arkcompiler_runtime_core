/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "ets_itable_builder.h"

#include "include/method.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_vtable_builder.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/bridge/bridge.h"

namespace ark::ets {

struct VTableDiff {
    struct ChangedSlot {
        Method *oldMethod;
        Method *newMethod;
    };
    explicit VTableDiff(mem::InternalArenaAllocator *allocator) : changed(*allocator) {}
    InternalArenaVector<ChangedSlot> changed;  // NOLINT(misc-non-private-member-variables-in-classes)
};

static VTableDiff *ComputeVTableDiff(mem::InternalArenaAllocator *allocator, Span<Method *> superVtable,
                                     Span<Method *> vtable)
{
    VTableDiff *diff = allocator->New<VTableDiff>(allocator);
    for (size_t k = 0; k < superVtable.size(); k++) {
        if (superVtable[k] != vtable[k]) {
            diff->changed.push_back({superVtable[k], vtable[k]});
        }
    }
    return diff;
}

// CC-OFFNXT(G.FUD.06, huge_method) perf critical, solid logic
static void RemapInheritedClassEntries(ITable &itable, const VTableDiff &diff)
{
    for (size_t i = 0; i < itable.Size(); i++) {
        auto &entry = itable[i];
        auto methods = entry.GetMethods();
        // NOLINTNEXTLINE(modernize-loop-convert)
        for (size_t j = 0; j < methods.size(); j++) {
            auto *m = methods[j];
            if (m == nullptr || m->GetClass()->IsInterface() || m->IsDefaultInterfaceMethod()) {
                continue;
            }
            for (auto const &slot : diff.changed) {
                ASSERT(!slot.newMethod->GetClass()->IsInterface());
                ASSERT(!slot.newMethod->IsDefaultInterfaceMethod());
                if (slot.oldMethod == m) {
                    methods[j] = slot.newMethod;
                    break;
                }
            }
        }
    }
}

static Span<ITable::Entry> CloneBaseITable(ClassLinker *classLinker, Class *base, size_t size)
{
    auto allocator = classLinker->GetAllocator();
    Span<ITable::Entry> itable {size == 0 ? nullptr : allocator->AllocArray<ITable::Entry>(size), size};

    for (auto &entry : itable) {
        entry.SetMethods({nullptr, nullptr});
    }

    if (base != nullptr) {
        auto superItable = base->GetITable().Get();
        ASSERT(superItable.Size() <= itable.size());
        for (size_t i = 0; i < superItable.size(); i++) {
            itable[i] = superItable[i].Copy(allocator);
        }
    }
    return itable;
}

static PandaUnorderedMap<Class *, bool> CollectAllInterfaces(Class *base, Span<Class *> classInterfaces)
{
    PandaUnorderedMap<Class *, bool> interfaces;

    if (base != nullptr) {
        for (auto item : base->GetITable().Get()) {
            // true means already placed in itable, false means pending
            interfaces.insert({item.GetInterface(), true});
        }
    }

    for (auto interface : classInterfaces) {
        ASSERT(interface != nullptr);
        if (interfaces.insert({interface, false}).second) {
            for (auto item : interface->GetITable().Get()) {
                interfaces.insert({item.GetInterface(), false});
            }
        }
    }
    return interfaces;
}

static Span<ITable::Entry> LinearizeITable(ClassLinker *classLinker, Class *base, Span<Class *> classInterfaces)
{
    auto allocator = classLinker->GetAllocator();
    auto interfaces = CollectAllInterfaces(base, classInterfaces);

    auto itable = CloneBaseITable(classLinker, base, interfaces.size());
    auto shift = base != nullptr ? base->GetITable().Size() : 0;

    for (auto interface : classInterfaces) {
        auto iterator = interfaces.find(interface);
        if (!(iterator != interfaces.end() && !iterator->second)) {
            continue;
        }
        auto interfaceItable = interface->GetITable().Get();
        for (auto &item : interfaceItable) {
            auto subIterator = interfaces.find(item.GetInterface());
            if (subIterator != interfaces.end() && !subIterator->second) {
                itable[shift++] = item.Copy(allocator);
                subIterator->second = true;
            }
        }
        itable[shift++].SetInterface(interface);
        iterator->second = true;
    }

    return itable;
}

bool EtsITableBuilder::Build(ClassLinker *classLinker, Class *base, Span<Class *> classInterfaces, bool isInterface)
{
    Span<ITable::Entry> itable =
        classInterfaces.Size() == 0 ? CloneBaseITable(classLinker, base, base != nullptr ? base->GetITable().Size() : 0)
                                    : LinearizeITable(classLinker, base, classInterfaces);

    if (!isInterface) {
        size_t const superItableSize = base != nullptr ? base->GetITable().Size() : 0;
        for (size_t i = superItableSize; i < itable.Size(); i++) {
            auto &entry = itable[i];
            auto methods = entry.GetInterface()->GetVirtualMethods();
            Method **methodsAlloc = nullptr;
            if (!methods.Empty()) {
                methodsAlloc = classLinker->GetAllocator()->AllocArray<Method *>(methods.size());
            }
            Span<Method *> methodsArray = {methodsAlloc, methods.size()};
            entry.SetMethods(methodsArray);
        }
    }

    itable_ = ITable(itable);
    return true;
}

// CopiedMethods already materialized, this only wires itable entries to vtable
bool EtsITableBuilder::Resolve(Class *klass)
{
    if (klass->IsInterface()) {
        return true;
    }
    UpdateClass(klass);

    if (dispatches_.empty()) {
        return true;
    }

    auto vtable = klass->GetVTable();

    for (auto const &disp : dispatches_) {
        if (disp.kind == IfaceMethodDispatch::Kind::REMAP_VTABLE) {
            ASSERT(disp.method == nullptr);
            auto *base = klass->GetBase();
            ASSERT(base != nullptr);
            auto diff = ComputeVTableDiff(allocator_, base->GetVTable(), vtable);
            RemapInheritedClassEntries(itable_, *diff);
            continue;
        }
        ASSERT(disp.itableIndex < itable_.Size());
        auto &entry = itable_[disp.itableIndex];
        ASSERT(disp.methodIndex < entry.GetMethods().size());
        entry.GetMethods()[disp.methodIndex] = ResolveMethod(vtable, klass, disp);
    }

    return true;
}

Method *EtsITableBuilder::ResolveMethod(Span<Method *> vtable, Class *klass, const IfaceMethodDispatch &disp)
{
    switch (disp.kind) {
        case IfaceMethodDispatch::Kind::REMAP_VTABLE:
            UNREACHABLE();
        case IfaceMethodDispatch::Kind::CLASS_METHOD:
            ASSERT(disp.resolveIndex < vtable.size());
            ASSERT(vtable[disp.resolveIndex]->GetName() == disp.method->GetName());
            ASSERT(!vtable[disp.resolveIndex]->IsDefaultInterfaceMethod());
            ASSERT(!vtable[disp.resolveIndex]->GetClass()->IsInterface());
            return vtable[disp.resolveIndex];
        case IfaceMethodDispatch::Kind::COPIED_ORDINARY: {
            ASSERT(disp.resolveIndex < klass->GetCopiedMethods().size());
            auto *resolved = &klass->GetCopiedMethods()[disp.resolveIndex];
            ASSERT(resolved->GetName() == disp.method->GetName());
            ASSERT(resolved->GetClass() == disp.method->GetClass());
            return resolved;
        }
        case IfaceMethodDispatch::Kind::COPIED_CONFLICT: {
            ASSERT(disp.resolveIndex < klass->GetCopiedMethods().size());
            auto *resolved = &klass->GetCopiedMethods()[disp.resolveIndex];
            ASSERT(resolved->GetName() == disp.method->GetName());
            ASSERT(resolved->GetClass() == disp.method->GetClass());
            ASSERT(resolved->GetCompiledEntryPoint() == GetDefaultConflictMethodStub());
            return resolved;
        }
        case IfaceMethodDispatch::Kind::AME:
            return disp.method;
    }
    UNREACHABLE();
}

void EtsITableBuilder::UpdateClass(Class *klass)
{
    klass->SetITable(itable_);
    DumpITable(klass);
}

void EtsITableBuilder::DumpITable([[maybe_unused]] Class *klass)
{
#ifndef NDEBUG
    LOG(DEBUG, CLASS_LINKER) << "itable of class " << klass->GetName() << ":";
    auto itable = klass->GetITable();
    size_t idxI = 0;
    for (size_t i = 0; i < itable.Size(); i++) {
        auto entry = itable[i];
        auto interface = entry.GetInterface();
        LOG(DEBUG, CLASS_LINKER) << "[ interface - " << idxI++ << " ] " << interface->GetName() << ":";
        // #22950 itable has nullptr entries
    }
#endif  // NDEBUG
}

}  // namespace ark::ets
