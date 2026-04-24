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
#include "runtime/include/exceptions.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/bridge/bridge.h"

namespace ark::ets {

// two-pass mirrors HasClassMethodOverride targetProto shift
static Method *ResolveClassMethodInVTable(Span<Method *> vtable, ClassLinkerContext *ctx, Method *ifaceMethod,
                                          Class *klass)
{
    Method *inherited = nullptr;
    for (size_t vi = vtable.size(); vi != 0;) {
        --vi;
        auto *vm = vtable[vi];
        if (vm->IsDefaultInterfaceMethod() || vm->GetClass()->IsInterface()) {
            continue;
        }
        if (vm->GetName() != ifaceMethod->GetName()) {
            continue;
        }
        if (vm->GetClass() == klass) {
            continue;  // pass1 skips own-class, inherited scan only
        }
        if (ETSProtoIsOverriddenBy(ctx, ifaceMethod->GetProtoId(), vm->GetProtoId())) {
            inherited = vm;
            break;
        }
    }

    auto targetProto = inherited != nullptr ? inherited->GetProtoId() : ifaceMethod->GetProtoId();
    // own-class methods, checks against inherited proto not original ifaceMethod
    for (size_t vi = vtable.size(); vi != 0;) {
        --vi;
        auto *vm = vtable[vi];
        if (vm->IsDefaultInterfaceMethod() || vm->GetClass()->IsInterface()) {
            continue;
        }
        if (vm->GetName() != ifaceMethod->GetName()) {
            continue;
        }
        if (vm->GetClass() != klass) {
            continue;
        }
        if (ETSProtoIsOverriddenBy(ctx, targetProto, vm->GetProtoId())) {
            return vm;
        }
    }

    ASSERT(inherited != nullptr);
    return inherited;
}

static Method *FindCopiedMethodInVTable(Span<Method *> vtable, ClassLinkerContext *ctx, Method *target)
{
    for (size_t vi = vtable.size(); vi != 0;) {
        --vi;
        auto *vm = vtable[vi];
        if (vm->GetName() == target->GetName() && vm->GetClass() == target->GetClass() &&
            ETSProtoIsOverriddenBy(ctx, target->GetProtoId(), vm->GetProtoId())) {
            return vm;
        }
    }
    return nullptr;
}

// stub identity disambiguates CONFLICT from ORDINARY for same iface method
static Method *FindConflictCopiedMethodInVTable(Span<Method *> vtable, ClassLinkerContext *ctx, Method *target)
{
    auto *stub = GetDefaultConflictMethodStub();
    for (size_t vi = vtable.size(); vi != 0;) {
        --vi;
        auto *vm = vtable[vi];
        if (vm->IsDefaultInterfaceMethod() && vm->GetName() == target->GetName() &&
            vm->GetClass() == target->GetClass() && vm->GetCompiledEntryPoint() == stub &&
            ETSProtoIsOverriddenBy(ctx, target->GetProtoId(), vm->GetProtoId())) {
            return vm;
        }
    }
    return nullptr;
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
        return true;  // inherited entries already correct
    }

    auto vtable = klass->GetVTable();
    auto *ctx = klass->GetLoadContext();

    size_t dispIdx = 0;
    for (size_t i = 0; i < itable_.Size(); i++) {
        auto &entry = itable_[i];
        auto methods = entry.GetInterface()->GetVirtualMethods();
        for (size_t j = 0; j < methods.size(); j++) {
            ASSERT(dispIdx < dispatches_.size());
            entry.GetMethods()[j] = ResolveMethod(vtable, ctx, klass, dispatches_[dispIdx]);
            dispIdx++;
        }
    }
    ASSERT(dispIdx == dispatches_.size());

    return true;
}

// index-based resolve with proto-checked fallback when vtable shifted
Method *EtsITableBuilder::ResolveMethod(Span<Method *> vtable, ClassLinkerContext *ctx, Class *klass,
                                        const IfaceMethodDispatch &disp)
{
    switch (disp.kind) {
        case IfaceMethodDispatch::Kind::CLASS_METHOD:
            if (disp.resolveIndex < vtable.size()) {
                auto *resolved = vtable[disp.resolveIndex];
                if (resolved->GetName() == disp.method->GetName() && !resolved->IsDefaultInterfaceMethod() &&
                    !resolved->GetClass()->IsInterface()) {
                    return resolved;
                }
            }
            return ResolveClassMethodInVTable(vtable, ctx, disp.method, klass);
        case IfaceMethodDispatch::Kind::COPIED_ORDINARY: {
            if (disp.resolveIndex < klass->GetCopiedMethods().size()) {
                auto *resolved = &klass->GetCopiedMethods()[disp.resolveIndex];
                if (resolved->GetName() == disp.method->GetName() && resolved->GetClass() == disp.method->GetClass()) {
                    return resolved;
                }
            }
            auto *cm = FindCopiedMethodInVTable(vtable, ctx, disp.method);
            ASSERT(cm != nullptr);
            return cm;
        }
        case IfaceMethodDispatch::Kind::COPIED_CONFLICT: {
            if (disp.resolveIndex < klass->GetCopiedMethods().size()) {
                auto *resolved = &klass->GetCopiedMethods()[disp.resolveIndex];
                if (resolved->GetName() == disp.method->GetName() && resolved->GetClass() == disp.method->GetClass() &&
                    resolved->GetCompiledEntryPoint() == GetDefaultConflictMethodStub()) {
                    return resolved;
                }
            }
            auto *conflict = FindConflictCopiedMethodInVTable(vtable, ctx, disp.method);
            ASSERT(conflict != nullptr);
            return conflict;
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
