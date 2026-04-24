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

// recomputed per candidate, no caching
static inline uint32_t InterfaceDepth(Class *iface)
{
    uint32_t depth = 0;
    for (auto &base : iface->GetInterfaces()) {
        depth = std::max(depth, InterfaceDepth(base));
    }
    return depth + 1;
}

template <class ProtoCompatibility, class OverridePred>
bool VarianceVTableBuilder<ProtoCompatibility, OverridePred>::IsOverriddenBy(const ClassLinkerContext *ctx,
                                                                             Method::ProtoId const &base,
                                                                             Method::ProtoId const &derv)
{
    if (&base.GetPandaFile() == &derv.GetPandaFile() && base.GetEntityId() == derv.GetEntityId()) {
        return true;  // same-file same-entityId bypasses proto compat
    }
    return ProtoCompatibility(ctx)(base, derv);
}

template <class ProtoCompatibility, class OverridePred>
// CC-OFFNXT(G.FMT.07, G.FUD.05) solid logic
typename VarianceVTableBuilder<ProtoCompatibility, OverridePred>::ClassOverrideResult
VarianceVTableBuilder<ProtoCompatibility, OverridePred>::HasClassMethodOverride(Method &imethod)
{
    auto *ctx = imethod.GetClass()->GetLoadContext();
    auto imethodName = imethod.GetName();
    auto imethodProto = imethod.GetProtoId();

    MethodInfo const *superclassResolved = nullptr;
    size_t superclassVtableIdx = 0;
    for (auto &[info, entry] : vtable_.Methods()) {
        if (!info->IsBase()) {
            continue;
        }
        if (entry.GetCandidate() != nullptr) {
            continue;  // candidate-on-base is T's own override, not inherited
        }
        if (info->IsInterfaceMethod()) {
            continue;
        }
        if (info->GetName() != imethodName) {
            continue;
        }
        if (IsOverriddenBy(ctx, imethodProto, info->GetProtoId())) {
            superclassResolved = info;
            superclassVtableIdx = entry.GetIndex();
            break;
        }
    }

    auto const &targetProto = superclassResolved != nullptr ? superclassResolved->GetProtoId() : imethodProto;
    size_t count = 0;
    size_t resolvedVtableIdx = 0;
    ArenaUnorderedSet<MethodInfo const *> seen(allocator_.Adapter());
    // candidate-on-base is T's own decl, not inherited, include here
    for (auto &[info, entry] : vtable_.Methods()) {
        MethodInfo const *effective = entry.CandidateOr(info);
        if (effective->IsInterfaceMethod()) {
            continue;
        }
        if (effective->GetName() != imethodName) {
            continue;
        }
        bool isOwnMethod = !info->IsBase() || entry.GetCandidate() != nullptr;
        if (!isOwnMethod) {
            continue;
        }
        if (IsOverriddenBy(ctx, targetProto, effective->GetProtoId())) {
            if (seen.insert(effective).second) {
                count++;
                resolvedVtableIdx = entry.GetIndex();
                // dedup by MethodInfo ptr identity, contravariant params can alias
            }
        }
    }

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
void VarianceVTableBuilder<ProtoCompatibility, OverridePred>::ResolveInterfaceMethods(ITable itable,
                                                                                      size_t superItableSize)
{
    ASSERT(dispatches_.empty());
    if (itable.Size() == 0) {
        return;
    }

    if (itable.Size() == superItableSize && numVmethods_ == 0) {
        return;  // new vmethods can shadow inherited defaults
    }

    auto *ctx = itable[0].GetInterface()->GetLoadContext();

    for (size_t i = 0; i < itable.Size(); i++) {
        auto *iface = itable[i].GetInterface();
        auto methods = iface->GetVirtualMethods();
        // re-resolves inherited entries too, new vmethods may override them
        for (auto &method : methods) {
            ResolveSingleInterfaceMethod(itable, iface, method, ctx);
        }
    }
}

template <class ProtoCompatibility, class OverridePred>
void VarianceVTableBuilder<ProtoCompatibility, OverridePred>::ResolveSingleInterfaceMethod(ITable itable, Class *iface,
                                                                                           Method &imethod,
                                                                                           ClassLinkerContext *ctx)
{
    auto classOverride = HasClassMethodOverride(imethod);
    if (classOverride.kind == ClassOverrideResult::Kind::CONFLICT) {
        auto *cm = FindOrCreateConflictCopiedMethod(&imethod);
        dispatches_.push_back({IfaceMethodDispatch::Kind::COPIED_CONFLICT, cm, GetCopiedMethodIndex(cm)});
        return;
    }
    if (classOverride.kind == ClassOverrideResult::Kind::SINGLE) {
        dispatches_.push_back({IfaceMethodDispatch::Kind::CLASS_METHOD, &imethod, classOverride.vtableIndex});
        return;  // class method found, skip elimination entirely
    }
    auto disp = ResolveByElimination(iface, imethod, ctx, itable);
    if (disp.kind == IfaceMethodDispatch::Kind::COPIED_ORDINARY ||
        disp.kind == IfaceMethodDispatch::Kind::COPIED_CONFLICT) {
        disp.resolveIndex = GetCopiedMethodIndex(disp.method);
    }
    dispatches_.push_back(disp);
}

template <class ProtoCompatibility, class OverridePred>
// CC-OFFNXT(G.FMT.07) project code style
ArenaVector<typename VarianceVTableBuilder<ProtoCompatibility, OverridePred>::IfaceMethodCandidate>
VarianceVTableBuilder<ProtoCompatibility, OverridePred>::CollectCandidates(Class *iface, Method &imethod,
                                                                           ClassLinkerContext *ctx, ITable itable)
{
    ArenaVector<IfaceMethodCandidate> candidates(allocator_.Adapter());
    for (size_t k = 0; k < itable.Size(); k++) {
        auto *otherIface = itable[k].GetInterface();
        if (!iface->IsAssignableFrom(otherIface)) {
            continue;  // unrelated itable entries invisible to this iface
        }
        for (auto &otherMethod : otherIface->GetVirtualMethods()) {
            if (otherMethod.GetName() != imethod.GetName()) {
                continue;
            }
            if (IsOverriddenBy(ctx, imethod.GetProtoId(), otherMethod.GetProtoId())) {
                candidates.push_back({&otherMethod, otherIface, InterfaceDepth(otherIface)});
            }
        }
    }
    return candidates;
}

// deepest-first with back-pruning, order-independent maximal set
template <class ProtoCompatibility, class OverridePred>
void VarianceVTableBuilder<ProtoCompatibility, OverridePred>::EliminateCandidates(
    ArenaVector<IfaceMethodCandidate> &candidates, ClassLinkerContext *ctx)
{
    std::sort(candidates.begin(), candidates.end(),
              [](const IfaceMethodCandidate &a, const IfaceMethodCandidate &b) { return a.depth > b.depth; });

    ArenaVector<IfaceMethodCandidate> survivors(allocator_.Adapter());
    for (auto &c : candidates) {
        bool eliminated = false;
        for (auto &s : survivors) {
            if (c.depth > s.depth) {
                continue;  // deeper iface cannot be eliminated by shallower
            }
            if (c.iface->IsAssignableFrom(s.iface) &&
                IsOverriddenBy(ctx, c.method->GetProtoId(), s.method->GetProtoId())) {
                eliminated = true;
                break;  // first surviving subinterface wins
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
    ArenaVector<IfaceMethodCandidate> &survivors, Method *imethod)
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
        return {IfaceMethodDispatch::Kind::COPIED_ORDINARY, survivors[0].method};
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
    EliminateCandidates(candidates, ctx);
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
