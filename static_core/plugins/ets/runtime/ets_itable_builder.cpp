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

#include "ets_itable_builder.h"

#include "runtime/include/class_linker.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/mem/panda_containers.h"

namespace panda::ets {

static Method *FindMethodInVTable(Class *klass, Method *method)
{
    auto vtable = klass->GetVTable();
    for (size_t i = vtable.size(); i > 0; i--) {
        auto idx = i - 1;
        if (vtable[idx]->GetName() == method->GetName() && vtable[idx]->GetProtoId() == method->GetProtoId()) {
            return vtable[idx];
        }
    }

    return nullptr;
}

// add interfaces
// interfaces of a superclass (they are located before others)
// self interfaces and interfaces of self interfaces
// add methods if it's not an interface (methods are located only for classes)
void EtsITableBuilder::Build(ClassLinker *class_linker, Class *base, Span<Class *> class_interfaces, bool is_interface)
{
    PandaUnorderedSet<Class *> interfaces;

    if (base != nullptr) {
        auto super_itable = base->GetITable().Get();
        for (auto item : super_itable) {
            interfaces.insert(item.GetInterface());
        }
    }

    for (auto interface : class_interfaces) {
        auto table = interface->GetITable().Get();
        for (auto item : table) {
            interfaces.insert(item.GetInterface());
        }
        interfaces.insert(interface);
    }

    auto allocator = class_linker->GetAllocator();
    ITable::Entry *array = nullptr;
    if (!interfaces.empty()) {
        array = allocator->AllocArray<ITable::Entry>(interfaces.size());
    }

    Span<ITable::Entry> itable {array, interfaces.size()};

    for (auto &entry : itable) {
        entry.SetMethods({nullptr, nullptr});
    }

    if (base != nullptr) {
        auto super_itable = base->GetITable().Get();
        for (size_t i = 0; i < super_itable.size(); i++) {
            itable[i] = super_itable[i].Copy(allocator);
            interfaces.erase(super_itable[i].GetInterface());
        }
    }

    size_t super_itable_size = 0;
    if (base != nullptr) {
        super_itable_size = base->GetITable().Size();
    }

    size_t shift = super_itable_size;

    for (auto interface : class_interfaces) {
        auto table = interface->GetITable().Get();
        for (auto &item : table) {
            auto iterator = interfaces.find(item.GetInterface());
            if (iterator != interfaces.end()) {
                itable[shift] = item.Copy(allocator);
                interfaces.erase(item.GetInterface());
                shift += 1;
            }
        }
        auto iterator = interfaces.find(interface);
        if (iterator != interfaces.end()) {
            itable[shift].SetInterface(interface);
            interfaces.erase(interface);
            shift += 1;
        }
    }

    ASSERT(interfaces.empty());

    if (!is_interface) {
        for (size_t i = super_itable_size; i < itable.Size(); i++) {
            auto &entry = itable[i];
            auto methods = entry.GetInterface()->GetVirtualMethods();
            Method **methods_alloc = nullptr;
            if (!methods.Empty()) {
                methods_alloc = allocator->AllocArray<Method *>(methods.size());
            }
            Span<Method *> methods_array = {methods_alloc, methods.size()};
            entry.SetMethods(methods_array);
        }
    }

    itable_ = ITable(itable);
}

void EtsITableBuilder::Resolve(Class *klass)
{
    if (klass->IsInterface()) {
        return;
    }

    for (size_t i = itable_.Size(); i > 0; i--) {
        auto entry = itable_[i - 1];
        auto methods = entry.GetInterface()->GetVirtualMethods();
        for (size_t j = 0; j < methods.size(); j++) {
            auto *res = FindMethodInVTable(klass, &methods[j]);
            if (res == nullptr) {
                res = &methods[j];
            }

            entry.GetMethods()[j] = res;
        }
    }
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
    size_t idx_i = 0;
    size_t idx_m = 0;
    for (size_t i = 0; i < itable.Size(); i++) {
        auto entry = itable[i];
        auto interface = entry.GetInterface();
        LOG(DEBUG, CLASS_LINKER) << "[ interface - " << idx_i++ << " ] " << interface->GetName() << ":";
        auto methods = entry.GetMethods();
        for (auto *method : methods) {
            LOG(DEBUG, CLASS_LINKER) << "[ method - " << idx_m++ << " ] " << method->GetFullName() << " - "
                                     << method->GetFileId().GetOffset();
        }
    }
#endif  // NDEBUG
}

}  // namespace panda::ets
