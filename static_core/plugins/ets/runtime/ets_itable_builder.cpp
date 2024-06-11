/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_vtable_builder.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark::ets {

static std::optional<Method *> FindMethodInVTable(Class *klass, Method *imethod, ClassLinkerErrorHandler *errHandler)
{
    auto vtable = klass->GetVTable();
    Method *candidate = nullptr;

    for (size_t i = vtable.size(); i != 0;) {
        i--;
        auto kmethod = vtable[i];
        if (kmethod->GetName() != imethod->GetName()) {
            continue;
        }
        if (!ETSProtoIsOverriddenBy(imethod->GetProtoId(), kmethod->GetProtoId())) {
            continue;
        }
        if (candidate != nullptr) {
            OnVTableConflict(errHandler, ClassLinker::Error::MULTIPLE_IMPLEMENT, "Multiple implementations", kmethod,
                             candidate);
            return std::nullopt;
        }
        candidate = kmethod;
    }
    return candidate;
}

static Span<ITable::Entry> LinearizeITable(ClassLinker *classLinker, Class *base, Span<Class *> classInterfaces,
                                           PandaUnorderedSet<Class *> &&interfaces)
{
    auto allocator = classLinker->GetAllocator();
    Span<ITable::Entry> itable {interfaces.empty() ? nullptr : allocator->AllocArray<ITable::Entry>(interfaces.size()),
                                interfaces.size()};

    for (auto &entry : itable) {
        entry.SetMethods({nullptr, nullptr});
    }

    if (base != nullptr) {
        auto superItable = base->GetITable().Get();
        for (size_t i = 0; i < superItable.size(); i++) {
            itable[i] = superItable[i].Copy(allocator);
            interfaces.erase(superItable[i].GetInterface());
        }
    }

    size_t const superItableSize = base != nullptr ? base->GetITable().Size() : 0;

    size_t shift = superItableSize;

    for (auto interface : classInterfaces) {
        auto table = interface->GetITable().Get();
        for (auto &item : table) {
            auto iterator = interfaces.find(item.GetInterface());
            if (iterator != interfaces.end()) {
                itable[shift++] = item.Copy(allocator);
                interfaces.erase(item.GetInterface());
            }
        }
        auto iterator = interfaces.find(interface);
        if (iterator != interfaces.end()) {
            itable[shift++].SetInterface(interface);
            interfaces.erase(interface);
        }
    }

    ASSERT(interfaces.empty());
    return itable;
}

// add interfaces
// interfaces of a superclass (they are located before others)
// self interfaces and interfaces of self interfaces
// add methods if it's not an interface (methods are located only for classes)
bool EtsITableBuilder::Build(ClassLinker *classLinker, Class *base, Span<Class *> classInterfaces, bool isInterface)
{
    PandaUnorderedSet<Class *> interfaces;

    if (base != nullptr) {
        auto superItable = base->GetITable().Get();
        for (auto item : superItable) {
            interfaces.insert(item.GetInterface());
        }
    }

    for (auto interface : classInterfaces) {
        auto table = interface->GetITable().Get();
        for (auto item : table) {
            interfaces.insert(item.GetInterface());
        }
        interfaces.insert(interface);
    }

    Span<ITable::Entry> itable = LinearizeITable(classLinker, base, classInterfaces, std::move(interfaces));

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

bool EtsITableBuilder::Resolve(Class *klass)
{
    if (klass->IsInterface()) {
        return true;
    }

    UpdateClass(klass);
    for (size_t i = itable_.Size(); i > 0; i--) {
        auto entry = itable_[i - 1];
        auto methods = entry.GetInterface()->GetVirtualMethods();
        for (size_t j = 0; j < methods.size(); j++) {
            auto res = FindMethodInVTable(klass, &methods[j], errorHandler_);
            if (!res.has_value()) {
                return false;
            }
            if (res.value() == nullptr) {
                res = &methods[j];
            }
            entry.GetMethods()[j] = res.value();
        }
    }
    return true;
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
    size_t idxM = 0;
    for (size_t i = 0; i < itable.Size(); i++) {
        auto entry = itable[i];
        auto interface = entry.GetInterface();
        LOG(DEBUG, CLASS_LINKER) << "[ interface - " << idxI++ << " ] " << interface->GetName() << ":";
        auto methods = entry.GetMethods();
        for (auto *method : methods) {
            LOG(DEBUG, CLASS_LINKER) << "[ method - " << idxM++ << " ] " << method->GetFullName() << " - "
                                     << method->GetFileId().GetOffset();
        }
    }
#endif  // NDEBUG
}

}  // namespace ark::ets
