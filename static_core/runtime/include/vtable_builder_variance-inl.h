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
#ifndef PANDA_RUNTIME_VTABLE_BUILDER_VARIANCE_INL_H
#define PANDA_RUNTIME_VTABLE_BUILDER_VARIANCE_INL_H

#include "runtime/include/vtable_builder_variance.h"
#include "runtime/include/vtable_builder_base-inl.h"

namespace ark {

template <class ProtoCompatibility, class OverridePred>
bool VarianceVTableBuilder<ProtoCompatibility, OverridePred>::IsOverriddenBy(const ClassLinkerContext *ctx,
                                                                             Method::ProtoId const &base,
                                                                             Method::ProtoId const &derv)
{
    if (&base.GetPandaFile() == &derv.GetPandaFile() && base.GetEntityId() == derv.GetEntityId()) {
        return true;  // same-file same-entityId bypasses proto compat
    }
    return ProtoCompatibility {}(ctx, base, derv);
}

template <class ProtoCompatibility, class OverridePred>
// CC-OFFNXT(G.FMT.07, G.FUD.05) solid logic
typename VarianceVTableBuilder<ProtoCompatibility, OverridePred>::ClassOverrideResult
VarianceVTableBuilder<ProtoCompatibility, OverridePred>::HasClassMethodOverride(Method &imethod)
{
    auto *ctx = imethod.GetClass()->GetLoadContext();
    auto imethodProto = imethod.GetProtoId();
    MethodInfo imethodInfo(&imethod);
    uint32_t imethodNameHash = GetHash32String(imethodInfo.GetName().data);

    MethodInfo const *superclassResolved = nullptr;
    size_t superclassVtableIdx = 0;
    if (numVmethods_ == 0) {
        for (auto it = SameNameMethodInfoIterator(vtable_.Methods(), &imethodInfo); !it.IsEmpty(); it.Next()) {
            auto &[info, entry] = it.Value();
            if (info->IsInterfaceMethod() || !info->IsBase() || entry.GetCandidate() != nullptr) {
                continue;
            }
            if (IsOverriddenBy(ctx, imethodProto, info->GetProtoId())) {
                return {ClassOverrideResult::Kind::SINGLE, entry.GetIndex()};
            }
        }
        return {ClassOverrideResult::Kind::NONE, 0};
    }
    InternalArenaVector<std::pair<MethodInfo const *, size_t>> ownMethods(allocator_);
    for (auto it = SameNameMethodInfoIterator(vtable_.Methods(), &imethodInfo); !it.IsEmpty(); it.Next()) {
        auto &[info, entry] = it.Value();
        MethodInfo const *effective = entry.CandidateOr(info);
        if (UNLIKELY(effective->IsInterfaceMethod())) {
            continue;
        }
        if (info->IsBase()) {
            if (entry.GetCandidate() == nullptr && superclassResolved == nullptr &&
                IsOverriddenBy(ctx, imethodProto, info->GetProtoId())) {
                superclassResolved = info;
                superclassVtableIdx = entry.GetIndex();
            }
            if (UNLIKELY(entry.GetCandidate() != nullptr)) {
                ownMethods.push_back({effective, entry.GetIndex()});
            }
            continue;
        }
        ownMethods.push_back({effective, entry.GetIndex()});
    }

    if (ownMethodNameHashes_.find(imethodNameHash) == ownMethodNameHashes_.end()) {
        if (superclassResolved != nullptr) {
            return {ClassOverrideResult::Kind::SINGLE, superclassVtableIdx};
        }
        return {ClassOverrideResult::Kind::NONE, 0};
    }

    auto const &targetProto = superclassResolved != nullptr ? superclassResolved->GetProtoId() : imethodProto;
    MethodInfo const *match0 = nullptr;
    MethodInfo const *match1 = nullptr;
    size_t resolvedVtableIdx = 0;
    for (auto const &[effective, index] : ownMethods) {
        if (IsOverriddenBy(ctx, targetProto, effective->GetProtoId())) {
            if (effective == match0 || effective == match1) {
                continue;
            }
            if (match0 == nullptr) {
                match0 = effective;
                resolvedVtableIdx = index;
            } else if (match1 == nullptr) {
                match1 = effective;
            } else {
                return {ClassOverrideResult::Kind::CONFLICT, 0};
            }
        }
    }

    size_t count = static_cast<size_t>(match0 != nullptr) + static_cast<size_t>(match1 != nullptr);
    if (superclassResolved != nullptr) {
        if (count > 1) {
            return {ClassOverrideResult::Kind::CONFLICT, 0};
        }
        return {ClassOverrideResult::Kind::SINGLE, superclassVtableIdx};
    }
    if (count > 1) {
        return {ClassOverrideResult::Kind::CONFLICT, 0};
    }
    if (count == 1) {
        return {ClassOverrideResult::Kind::SINGLE, resolvedVtableIdx};
    }
    return {ClassOverrideResult::Kind::NONE, 0};
}

template <class ProtoCompatibility, class OverridePred>
// CC-OFFNXT(G.FMT.07) project code style
typename VarianceVTableBuilder<ProtoCompatibility, OverridePred>::ClassOverrideResult
VarianceVTableBuilder<ProtoCompatibility, OverridePred>::GetInheritedClassOverride(Method *inherited) const
{
    auto idx = inherited->GetVTableIndex();
    if (idx >= baseMethodInfoByIndex_.size()) {
        return {ClassOverrideResult::Kind::NONE, 0};
    }
    auto it = vtable_.Methods().find(baseMethodInfoByIndex_[idx]);
    if (it == vtable_.Methods().end()) {
        return {ClassOverrideResult::Kind::NONE, 0};
    }
    if (it->second.GetCandidate() == nullptr) {
        return {ClassOverrideResult::Kind::NONE, 0};
    }
    return {ClassOverrideResult::Kind::SINGLE, it->second.GetIndex()};
}

template <class ProtoCompatibility, class OverridePred>
void VarianceVTableBuilder<ProtoCompatibility, OverridePred>::RebuildOwnMethodNameHashes()
{
    ownMethodNameHashes_.clear();
    for (auto const &[info, entry] : vtable_.Methods()) {
        MethodInfo const *effective = entry.CandidateOr(info);
        if (effective->IsInterfaceMethod()) {
            continue;
        }
        if (!info->IsBase() || entry.GetCandidate() != nullptr) {
            ownMethodNameHashes_.insert(GetHash32String(info->GetName().data));
        }
    }
}

template <class ProtoCompatibility, class OverridePred>
bool VarianceVTableBuilder<ProtoCompatibility, OverridePred>::OwnMethodsShadowInterface(ITable itable)
{
    for (size_t i = 0; i < itable.Size(); i++) {
        auto *iface = itable[i].GetInterface();
        for (auto &method : iface->GetVirtualMethods()) {
            uint32_t nameHash = GetHash32String(method.GetName().data);
            if (ownMethodNameHashes_.count(nameHash) > 0) {
                return true;
            }
        }
    }
    return false;
}

static inline uint32_t InterfaceDepth(Class *iface)
{
    uint32_t depth = 1;
    for (auto &base : iface->GetInterfaces()) {
        depth = std::max(depth, InterfaceDepth(base) + 1);
    }
    return depth;
}

template <class ProtoCompatibility, class OverridePred>
bool VarianceVTableBuilder<ProtoCompatibility, OverridePred>::ProcessClassMethod(const MethodInfo *info)
{
    auto *ctx = info->GetLoadContext();
    bool compatibleFound = false;

    for (auto it = SameNameMethodInfoIterator(vtable_.Methods(), info); !it.IsEmpty(); it.Next()) {
        auto &[itInfo, itEntry] = it.Value();

        if (!itInfo->IsBase()) {
            continue;
        }
        if (IsOverriddenBy(ctx, itInfo->GetProtoId(), info->GetProtoId()) && OverridePred()(itInfo, info)) {
            if (UNLIKELY(itInfo->IsFinal())) {
                OnVTableConflict(errorHandler_, ClassLinker::Error::OVERRIDES_FINAL, "Method overrides final method",
                                 info, itInfo);
                return false;
            }
            if (UNLIKELY(itEntry.GetCandidate() != nullptr)) {
                continue;  // swallowed, count>=2 caught later
            }
            itEntry.SetCandidate(info);
            compatibleFound = true;
        }
    }

    if (!compatibleFound) {
        vtable_.AddEntry(info);
    } else {
        vtableAppendedOnly_ = false;
    }
    return true;
}

template <class ProtoCompatibility, class OverridePred>
bool VarianceVTableBuilder<ProtoCompatibility, OverridePred>::ProcessProxyClassMethod(const MethodInfo *info)
{
    auto *ctx = info->GetLoadContext();

    bool compatibleExists = false;
    for (auto it = SameNameMethodInfoIterator(vtable_.Methods(), info); !it.IsEmpty(); it.Next()) {
        auto &[itInfo, itEntry] = it.Value();

        if (IsOverriddenBy(ctx, itInfo->GetProtoId(), info->GetProtoId())) {
            vtable_.ReplaceEntryWith(itInfo, info);
            compatibleExists = true;
            break;
        }
        if (IsOverriddenBy(ctx, info->GetProtoId(), itInfo->GetProtoId())) {
            compatibleExists = true;
            break;
        }
    }

    if (!compatibleExists) {
        vtable_.AddEntry(info);
    }

    return true;
}

template <class ProtoCompatibility, class OverridePred>
bool VarianceVTableBuilder<ProtoCompatibility, OverridePred>::ProcessDefaultMethod([[maybe_unused]] ITable itable,
                                                                                   [[maybe_unused]] size_t itableIdx,
                                                                                   MethodInfo *methodInfo)
{
    auto *ctx = methodInfo->GetLoadContext();

    for (auto it = SameNameMethodInfoIterator(vtable_.Methods(), methodInfo); !it.IsEmpty(); it.Next()) {
        MethodInfo const *itinfo = it.Value().second.CandidateOr(it.Value().first);
        if (itinfo->IsInterfaceMethod()) {
            continue;
        }
        if (IsOverriddenBy(ctx, methodInfo->GetProtoId(), itinfo->GetProtoId())) {
            return true;  // default suppressed by class method, no CopiedMethod needed
        }
    }

    for (auto it = SameNameMethodInfoIterator(vtable_.CopiedMethods(), methodInfo); !it.IsEmpty(); it.Next()) {
        if (it.Value().first->GetMethod() == methodInfo->GetMethod()) {
            return true;  // ptr identity not proto, same default already placed from another iface
        }
    }

    vtable_.AddCopiedEntry(methodInfo);
    return true;
}

template <class ProtoCompatibility, class OverridePred>
// CC-OFFNXT(G.FUD.05) perf critical, solid logic
void VarianceVTableBuilder<ProtoCompatibility, OverridePred>::ResolveInterfaceMethods(ITable itable,
                                                                                      size_t superItableSize)
{
    static constexpr size_t INTERFACE_METHOD_CANDIDATE_INDEX_THRESHOLD = 64U;

    ASSERT(dispatches_.empty());
    if (itable.Size() == 0) {
        return;
    }

    if (itable.Size() == superItableSize && numVmethods_ == 0) {
        return;
    }

    auto *ctx = itable[0].GetInterface()->GetLoadContext();
    if (numVmethods_ > 0) {
        RebuildOwnMethodNameHashes();
    } else {
        ownMethodNameHashes_.clear();
    }
    interfaceMethodCandidatesByName_.clear();
    useInterfaceMethodCandidateIndex_ = itable.Size() >= INTERFACE_METHOD_CANDIDATE_INDEX_THRESHOLD;
    if (useInterfaceMethodCandidateIndex_) {
        for (size_t i = 0; i < itable.Size(); i++) {
            auto *iface = itable[i].GetInterface();
            uint32_t depth = InterfaceDepth(iface);
            for (auto &method : iface->GetVirtualMethods()) {
                uint32_t nameHash = GetHash32String(method.GetName().data);
                auto it = interfaceMethodCandidatesByName_.find(nameHash);
                InternalArenaVector<IfaceMethodCandidate> *bucket = nullptr;
                if (it == interfaceMethodCandidatesByName_.end()) {
                    bucket = allocator_.New<InternalArenaVector<IfaceMethodCandidate>>(allocator_);
                    interfaceMethodCandidatesByName_.insert({nameHash, bucket});
                } else {
                    bucket = it->second;
                }
                bucket->push_back({&method, iface, depth});
            }
        }
    }

    bool stableInherited = false;
    if (itable.Size() == superItableSize) {
        if (vtable_.GetVTableSize() == baseVTableSize_) {
            stableInherited = true;
        } else if (vtableAppendedOnly_ && numVmethods_ > 0 && !OwnMethodsShadowInterface(itable)) {
            stableInherited = true;
        }
    }
    if (stableInherited) {
        dispatches_.push_back({IfaceMethodDispatch::Kind::REMAP_VTABLE, nullptr, 0, 0, 0});
    }

    for (size_t i = 0; i < itable.Size(); i++) {
        auto *iface = itable[i].GetInterface();
        auto methods = iface->GetVirtualMethods();
        for (size_t j = 0; j < methods.size(); j++) {
            auto &method = methods[j];
            auto classOverride = ClassOverrideResult {};
            if (i < superItableSize) {
                // CC-OFFNXT(G.FUN.01-CPP) solid logic
                if (itable.Size() == superItableSize && numVmethods_ > 0 && !stableInherited) {
                    uint32_t nameHash = GetHash32String(method.GetName().data);
                    if (ownMethodNameHashes_.count(nameHash) == 0) {
                        continue;
                    }
                }
                auto *inherited = itable[i].GetMethods()[j];
                // CC-OFFNXT(G.FUN.01-CPP) solid logic
                if (inherited != nullptr && !inherited->GetClass()->IsInterface() &&
                    !inherited->IsDefaultInterfaceMethod()) {
                    if (stableInherited) {
                        continue;
                    }
                    classOverride = GetInheritedClassOverride(inherited);
                    if (classOverride.kind == ClassOverrideResult::Kind::NONE) {
                        continue;
                    }
                    dispatches_.push_back(
                        {IfaceMethodDispatch::Kind::CLASS_METHOD, &method, classOverride.vtableIndex, i, j});
                    continue;
                }
                // CC-OFFNXT(G.FUN.01-CPP) solid logic
                if (stableInherited) {
                    classOverride = HasClassMethodOverride(method);
                    if (classOverride.kind == ClassOverrideResult::Kind::NONE) {
                        continue;
                    }
                }
            }
            ResolveSingleInterfaceMethod(itable, i, iface, j, method, ctx, classOverride);
        }
    }
}

template <class ProtoCompatibility, class OverridePred>
void VarianceVTableBuilder<ProtoCompatibility, OverridePred>::ResolveSingleInterfaceMethod(
    ITable itable, size_t itableIndex, Class *iface, size_t methodIndex, Method &imethod, ClassLinkerContext *ctx,
    ClassOverrideResult classOverride)
{
    if (classOverride.kind == ClassOverrideResult::Kind::NONE) {
        classOverride = HasClassMethodOverride(imethod);
    }
    if (classOverride.kind == ClassOverrideResult::Kind::CONFLICT) {
        auto *cm = FindOrCreateConflictCopiedMethod(&imethod);
        dispatches_.push_back(
            {IfaceMethodDispatch::Kind::COPIED_CONFLICT, cm, GetCopiedMethodIndex(cm), itableIndex, methodIndex});
        return;
    }
    if (classOverride.kind == ClassOverrideResult::Kind::SINGLE) {
        dispatches_.push_back(
            {IfaceMethodDispatch::Kind::CLASS_METHOD, &imethod, classOverride.vtableIndex, itableIndex, methodIndex});
        return;
    }
    auto disp = ResolveByElimination(iface, imethod, ctx, itable);
    if (disp.kind == IfaceMethodDispatch::Kind::COPIED_ORDINARY ||
        disp.kind == IfaceMethodDispatch::Kind::COPIED_CONFLICT) {
        disp.resolveIndex = GetCopiedMethodIndex(disp.method);
    }
    disp.itableIndex = itableIndex;
    disp.methodIndex = methodIndex;
    dispatches_.push_back(disp);
}

template <class ProtoCompatibility, class OverridePred>
// CC-OFFNXT(G.FMT.07) project code style
auto VarianceVTableBuilder<ProtoCompatibility, OverridePred>::CollectCandidates(Class *iface, Method &imethod,
                                                                                ClassLinkerContext *ctx, ITable itable)
    -> InternalArenaVector<typename VarianceVTableBuilder<ProtoCompatibility, OverridePred>::IfaceMethodCandidate>
{
    InternalArenaVector<IfaceMethodCandidate> candidates(allocator_);
    if (!useInterfaceMethodCandidateIndex_) {
        for (size_t k = 0; k < itable.Size(); k++) {
            auto *otherIface = itable[k].GetInterface();
            uint32_t depth = InterfaceDepth(otherIface);
            for (auto &method : otherIface->GetVirtualMethods()) {
                uint32_t nameHash = GetHash32String(method.GetName().data);
                auto it = interfaceMethodCandidatesByName_.find(nameHash);
                InternalArenaVector<IfaceMethodCandidate> *bucket = nullptr;
                if (it == interfaceMethodCandidatesByName_.end()) {
                    bucket = allocator_.New<InternalArenaVector<IfaceMethodCandidate>>(allocator_);
                    interfaceMethodCandidatesByName_.insert({nameHash, bucket});
                } else {
                    bucket = it->second;
                }
                bucket->push_back({&method, otherIface, depth});
            }
        }
        useInterfaceMethodCandidateIndex_ = true;
    }
    uint32_t nameHash = GetHash32String(imethod.GetName().data);
    auto bucketIt = interfaceMethodCandidatesByName_.find(nameHash);
    if (bucketIt == interfaceMethodCandidatesByName_.end()) {
        return candidates;
    }
    auto targetDepth = InterfaceDepth(iface);
    for (auto const &candidate : *bucketIt->second) {
        if (candidate.depth < targetDepth) {
            continue;
        }
        if (!iface->IsAssignableFrom(candidate.iface)) {
            continue;
        }
        if (IsOverriddenBy(ctx, imethod.GetProtoId(), candidate.method->GetProtoId())) {
            candidates.push_back(candidate);
        }
    }
    return candidates;
}

// deepest-first with back-pruning, order-independent maximal set
template <class ProtoCompatibility, class OverridePred>
void VarianceVTableBuilder<ProtoCompatibility, OverridePred>::EliminateCandidates(
    InternalArenaVector<IfaceMethodCandidate> &candidates, ClassLinkerContext *ctx)
{
    std::sort(candidates.begin(), candidates.end(),
              [](const IfaceMethodCandidate &a, const IfaceMethodCandidate &b) { return a.depth > b.depth; });

    InternalArenaVector<IfaceMethodCandidate> survivors(allocator_);
    for (auto &c : candidates) {
        bool eliminated = false;
        for (auto &s : survivors) {
            if (c.depth > s.depth) {
                continue;
            }
            if (c.iface->IsAssignableFrom(s.iface) &&
                IsOverriddenBy(ctx, c.method->GetProtoId(), s.method->GetProtoId())) {
                eliminated = true;
                break;
            }
        }
        if (!eliminated) {
            size_t writeIdx = 0;
            for (size_t si = 0; si < survivors.size(); si++) {
                if (survivors[si].depth == c.depth && survivors[si].iface->IsAssignableFrom(c.iface) &&
                    IsOverriddenBy(ctx, survivors[si].method->GetProtoId(), c.method->GetProtoId())) {
                    continue;
                }
                survivors[writeIdx++] = survivors[si];
            }
            survivors.resize(writeIdx);
            survivors.push_back(c);
        }
    }
    candidates = std::move(survivors);
}

// mixed abstract+default is conflict, all-abstract is AME
template <class ProtoCompatibility, class OverridePred>
IfaceMethodDispatch VarianceVTableBuilder<ProtoCompatibility, OverridePred>::ClassifySurvivors(
    InternalArenaVector<IfaceMethodCandidate> &survivors, Method *imethod)
{
    bool hasAbstract = false;
    bool hasDefault = false;
    for (auto &s : survivors) {
        if (s.method->IsAbstract()) {
            hasAbstract = true;
        } else {
            hasDefault = true;
        }
    }

    if (survivors.size() == 1 && hasAbstract) {
        return {IfaceMethodDispatch::Kind::AME, survivors[0].method};
    }
    if (survivors.size() == 1) {
        auto *survivor = survivors[0].method;
        for (auto &[info, entry] : vtable_.CopiedMethods()) {
            if (info->GetMethod() == survivor) {
                return {IfaceMethodDispatch::Kind::COPIED_ORDINARY, survivor};
            }
        }

        auto classOverride = HasClassMethodOverride(*survivor);
        if (classOverride.kind == ClassOverrideResult::Kind::SINGLE) {
            return {IfaceMethodDispatch::Kind::CLASS_METHOD, survivor, classOverride.vtableIndex};
        }
        return {IfaceMethodDispatch::Kind::AME, survivor};
    }
    if (hasAbstract && !hasDefault) {
        return {IfaceMethodDispatch::Kind::AME, survivors.back().method};  // arbitrary survivor for AME
    }
    return {IfaceMethodDispatch::Kind::COPIED_CONFLICT, FindOrCreateConflictCopiedMethod(imethod)};
}

template <class ProtoCompatibility, class OverridePred>
IfaceMethodDispatch VarianceVTableBuilder<ProtoCompatibility, OverridePred>::ResolveByElimination(
    Class *iface, Method &imethod, ClassLinkerContext *ctx, ITable itable)
{
    auto candidates = CollectCandidates(iface, imethod, ctx, itable);
    ASSERT(!candidates.empty());
    bool hasDefault = false;
    for (auto &candidate : candidates) {
        if (!candidate.method->IsAbstract()) {
            hasDefault = true;
            break;
        }
    }
    if (!hasDefault) {
        return {IfaceMethodDispatch::Kind::AME, candidates.back().method};
    }
    if (candidates.size() != 1) {
        EliminateCandidates(candidates, ctx);
    }
    return ClassifySurvivors(candidates, &imethod);
}

template <class ProtoCompatibility, class OverridePred>
size_t VarianceVTableBuilder<ProtoCompatibility, OverridePred>::GetCopiedMethodIndex(Method *method)
{
    for (auto &[info, entry] : vtable_.CopiedMethods()) {
        if (info->GetMethod() == method) {
            return entry.GetIndex();
        }
    }
    return 0;  // not found yields index 0, caller must not rely on this
}

// never upgrades ORDINARY to CONFLICT, per-interface independence
template <class ProtoCompatibility, class OverridePred>
Method *VarianceVTableBuilder<ProtoCompatibility, OverridePred>::FindOrCreateConflictCopiedMethod(Method *imethod)
{
    for (auto &[info, entry] : vtable_.CopiedMethods()) {
        if (info->GetMethod() == imethod && entry.GetStatus() == CopiedMethod::Status::CONFLICT) {
            return imethod;
        }
    }

    auto *info = allocator_.New<MethodInfo>(imethod);
    // stack MethodInfo would dangle as CopiedMethods key
    vtable_.AddCopiedEntry(info).SetStatus(CopiedMethod::Status::CONFLICT);
    return imethod;
}

}  // namespace ark

#endif  // PANDA_RUNTIME_VTABLE_BUILDER_VARIANCE_INL_H
