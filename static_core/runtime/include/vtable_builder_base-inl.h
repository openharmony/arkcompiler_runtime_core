/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_VTABLE_BUILDER_BASE_INL_H
#define PANDA_RUNTIME_VTABLE_BUILDER_BASE_INL_H

#include "runtime/include/vtable_builder_base.h"

namespace ark {

void OnVTableConflict(ClassLinkerErrorHandler *errHandler, ClassLinker::Error error, std::string_view msg,
                      MethodInfo const *info1, MethodInfo const *info2);

void OnVTableConflict(ClassLinkerErrorHandler *errHandler, ClassLinker::Error error, std::string_view msg,
                      Method const *info1, Method const *info2);

inline void VTableInfo::AddEntry(const MethodInfo *info)
{
    [[maybe_unused]] auto res = vmethods_.insert({info, MethodEntry(vmethods_.size())});
    ASSERT(res.second);
}

inline void VTableInfo::Reserve(size_t methodCount)
{
    vmethods_.reserve(methodCount);
}

inline void VTableInfo::ReplaceEntryWith(const MethodInfo *prev, const MethodInfo *current)
{
    ASSERT(prev != current);

    auto it = vmethods_.find(prev);
    ASSERT(it != vmethods_.end());

    size_t idx = it->second.GetIndex();

    vmethods_.erase(it);

    [[maybe_unused]] auto res = vmethods_.insert({current, MethodEntry(idx)});
    ASSERT(res.second);
}

inline VTableInfo::CopiedMethodEntry &VTableInfo::AddCopiedEntry(const MethodInfo *info)
{
    auto res = copiedMethods_.insert({info, CopiedMethodEntry(copiedMethods_.size())});
    ASSERT(res.second);
    return res.first->second;
}

inline VTableInfo::CopiedMethodEntry &VTableInfo::UpdateCopiedEntry(const MethodInfo *orig, const MethodInfo *repl)
{
    auto it = copiedMethods_.find(orig);
    ASSERT(it != copiedMethods_.end());
    CopiedMethodEntry entry = it->second;
    copiedMethods_.erase(it);
    return copiedMethods_.emplace_hint(it, repl, entry)->second;
}

template <bool VISIT_SUPERITABLE>
void VTableBuilderBase<VISIT_SUPERITABLE>::BuildForInterface(Span<const Method> vmethods)
{
    for (const auto &method : vmethods) {
        if (!method.IsAbstract()) {
            hasDefaultMethods_ = true;
            break;
        }
    }
    numVmethods_ = vmethods.size();
}

template <bool VISIT_SUPERITABLE>
void VTableBuilderBase<VISIT_SUPERITABLE>::AddBaseMethods(Class *baseClass)
{
    if (baseClass != nullptr) {
        auto baseVTable = baseClass->GetVTable();
        size_t size = baseVTableSize_ = baseVTable.size();
        baseMethodInfoByIndex_ = Span {allocator_->AllocArray<MethodInfo>(size), size};

        for (size_t i = 0; i < size; i++) {
            const auto &method = baseVTable[i];
            auto *mi = new (&baseMethodInfoByIndex_[i]) MethodInfo(method, true);
            vtable_->AddEntry(mi);
        }
    }
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::AddClassMethods(Span<const Method> vmethods, ClassLinkerContext *ctx,
                                                           panda_file::File::EntityId classId)
{
    size_t numVmethods = vmethods.size();
    auto classMethods = allocator_->AllocArray<MethodInfo>(numVmethods);
    Span<MethodInfo> methodInfos(classMethods, numVmethods);
    ASSERT(classMethods != nullptr);

    for (size_t i = 0; i < numVmethods; i++) {
        new (&methodInfos[i]) MethodInfo(&vmethods[i], i, ctx, classId);
    }

    for (size_t i = numVmethods; i-- > 0;) {
        if (!ProcessClassMethod(&methodInfos[i])) {
            return false;
        }
    }
    numVmethods_ = numVmethods;
    return true;
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::AddClassMethods(Span<Method> vmethods)
{
    size_t numVmethods = vmethods.size();
    auto classMethods = allocator_->AllocArray<MethodInfo>(numVmethods);
    Span<MethodInfo> methodInfos(classMethods, numVmethods);
    ASSERT(classMethods != nullptr);

    for (size_t i = 0; i < numVmethods; i++) {
        new (&methodInfos[i]) MethodInfo(&vmethods[i]);
    }

    for (size_t i = numVmethods; i-- > 0;) {
        if (!ProcessClassMethod(&methodInfos[i])) {
            return false;
        }
    }
    numVmethods_ = numVmethods;
    return true;
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::AddProxyClassMethods(Span<Method *> methods)
{
    size_t numVmethods = methods.size();
    auto *classMethods = allocator_->AllocArray<MethodInfo>(numVmethods);
    Span<MethodInfo> methodInfos(classMethods, numVmethods);
    ASSERT(classMethods != nullptr);

    for (size_t i = 0; i < numVmethods; i++) {
        new (&methodInfos[i]) MethodInfo(methods[i]);
    }

    for (size_t i = numVmethods; i-- > 0;) {
        if (!ProcessProxyClassMethod(&methodInfos[i])) {
            return false;
        }
    }
    numVmethods_ = numVmethods;
    return true;
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::AddDefaultInterfaceMethods(ITable itable, size_t superItableSize)
{
    // NOTE(vpukhov): avoid traversing the whole itable and handle conflicting super vtable methods in a separate pass
    size_t const traverseUpTo = VISIT_SUPERITABLE ? 0 : superItableSize;

    for (size_t i = itable.Size(); i != traverseUpTo;) {
        i--;
        auto iface = itable[i].GetInterface();
        if (!iface->HasDefaultMethods()) {
            continue;
        }

        auto methods = iface->GetVirtualMethods();
        for (auto &method : methods) {
            if (method.IsAbstract()) {
                continue;  // abstracts never become CopiedMethods
            }
            MethodInfo *info = allocator_->New<MethodInfo>(&method);
            if (!ProcessDefaultMethod(itable, i, info)) {
                return false;
            }
        }
    }
    return true;
}

// must run after hook, step 3 creates CONFLICT copied methods
template <bool VISIT_SUPERITABLE>
void VTableBuilderBase<VISIT_SUPERITABLE>::BuildOrderedCopiedMethods()
{
    auto copiedMethods = vtable_->CopiedMethods();
    orderedCopiedMethods_ = Span {allocator_->AllocArray<CopiedMethod>(copiedMethods.size()), copiedMethods.size()};

    for (auto const &[info, entry] : copiedMethods) {
        CopiedMethod copied(info->GetMethod());
        copied.SetStatus(entry.GetStatus());
        orderedCopiedMethods_[entry.GetIndex()] = copied;
    }
}

// When ClassLinker creates the VTable, the Class has not been created yet, so it needs to use ClassDataAccessor to
// obtain the class information. Do not use the "methods" to obtain class information.
template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::Build(panda_file::ClassDataAccessor *cda, Span<const Method> vmethods,
                                                 Class *baseClass, ITable itable, ClassLinkerContext *ctx)
{
    if (cda->IsInterface()) {
        BuildForInterface(vmethods);
        return true;
    }

    vtable_->Reserve((baseClass != nullptr ? baseClass->GetVTable().size() : 0) + vmethods.size());
    AddBaseMethods(baseClass);
    if (!AddClassMethods(vmethods, ctx, cda->GetClassId())) {
        return false;
    }
    if (!AddDefaultInterfaceMethods(itable, baseClass != nullptr ? baseClass->GetITable().Size() : 0)) {
        return false;
    }
    ResolveInterfaceMethodsHook(itable, baseClass != nullptr ? baseClass->GetITable().Size() : 0);
    BuildOrderedCopiedMethods();
    return true;
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::Build(Span<Method> vmethods, Class *baseClass, ITable itable,
                                                 bool isInterface)
{
    if (isInterface) {
        BuildForInterface(vmethods.ToConst());
        return true;
    }

    vtable_->Reserve((baseClass != nullptr ? baseClass->GetVTable().size() : 0) + vmethods.size());
    AddBaseMethods(baseClass);
    if (!AddClassMethods(vmethods)) {
        return false;
    }
    if (!AddDefaultInterfaceMethods(itable, baseClass != nullptr ? baseClass->GetITable().Size() : 0)) {
        return false;
    }
    ResolveInterfaceMethodsHook(itable, baseClass != nullptr ? baseClass->GetITable().Size() : 0);
    BuildOrderedCopiedMethods();
    return true;
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::FilterProxyClassMethods(Span<Method *> input, PandaVector<Method *> *output,
                                                                   Class *baseClass)
{
    AddBaseMethods(baseClass);
    if (!AddProxyClassMethods(input)) {
        return false;
    }

    if (!CollectProxyMethods(output)) {
        return false;
    }

    return true;
}

template <bool VISIT_SUPERITABLE>
void VTableBuilderBase<VISIT_SUPERITABLE>::UpdateClass(Class *klass) const
{
    if (klass->IsInterface()) {
        if (hasDefaultMethods_) {
            klass->SetHasDefaultMethods();
        }

        size_t idx = 0;
        for (auto &method : klass->GetVirtualMethods()) {
            method.SetVTableIndex(idx++);
        }
    }

    vtable_->UpdateClass(klass);
}

template <bool VISIT_SUPERITABLE>
bool VTableBuilderBase<VISIT_SUPERITABLE>::CollectProxyMethods(PandaVector<Method *> *output)
{
    auto &methodsUmap = vtable_->Methods();
    for (auto it : methodsUmap) {
        if (!it.first->IsBase()) {
            output->push_back(it.first->GetMethod());
        }
    }
    for (auto it : orderedCopiedMethods_) {
        output->push_back(it.GetMethod());
    }
    return true;
}

}  // namespace ark

#endif  // PANDA_RUNTIME_VTABLE_BUILDER_BASE_H
