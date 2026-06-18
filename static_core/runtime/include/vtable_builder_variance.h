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
#ifndef PANDA_RUNTIME_VTABLE_BUILDER_VARIANCE_H
#define PANDA_RUNTIME_VTABLE_BUILDER_VARIANCE_H

#include "mem/internal_arena_allocator.h"
#include "runtime/class_linker_context.h"
#include "runtime/include/vtable_builder_base.h"

namespace ark {

template <class ProtoCompatibility, class OverridePred, class SharedArenaAllocator>
class VarianceVTableBuilder final : public VTableBuilderBase<false> {
public:
    explicit VarianceVTableBuilder(ClassLinkerErrorHandler *errHandler) : VTableBuilderBase(errHandler)
    {
        mem::InternalArenaAllocator *allocator = SharedArenaAllocator::Get(needReleaseAllocator_);
        currentPosition_ = allocator->GetAllocatedSize();
        SetAllocator(allocator);
        allocator->MakeContainer(ownMethodNameHashes_);
        allocator->MakeContainer(interfaceMethodCandidatesByName_);
    }

    ~VarianceVTableBuilder() override
    {
        allocator_->Resize(currentPosition_);
        if (needReleaseAllocator_) {
            SharedArenaAllocator::Release(allocator_);
        }
    }

    NO_COPY_SEMANTIC(VarianceVTableBuilder);
    NO_MOVE_SEMANTIC(VarianceVTableBuilder);

private:
    [[nodiscard]] bool ProcessClassMethod(const MethodInfo *info) override;
    [[nodiscard]] bool ProcessDefaultMethod(ITable itable, size_t itableIdx, MethodInfo *methodInfo) override;

    [[nodiscard]] bool ProcessProxyClassMethod(const MethodInfo *info) override;

    void ResolveInterfaceMethodsHook(ITable itable, size_t superItableSize) override
    {
        ResolveInterfaceMethods(itable, superItableSize);
    }

    struct IfaceMethodCandidate {
        Method *method {};
        Class *iface {};
        uint32_t depth {};
    };

    struct ClassOverrideResult {
        enum class Kind { NONE, SINGLE, CONFLICT };
        Kind kind = Kind::NONE;
        size_t vtableIndex = 0;
    };

    void ResolveInterfaceMethods(ITable itable, size_t superItableSize);
    void ResolveSingleInterfaceMethod(ITable itable, size_t itableIndex, Class *iface, size_t methodIndex,
                                      Method &imethod, ClassLinkerContext *ctx, ClassOverrideResult classOverride);
    IfaceMethodDispatch ResolveByElimination(Class *iface, Method &imethod, ClassLinkerContext *ctx, ITable itable);

    InternalArenaVector<IfaceMethodCandidate> CollectCandidates(Class *iface, Method &imethod, ClassLinkerContext *ctx,
                                                                ITable itable);

    void EliminateCandidates(InternalArenaVector<IfaceMethodCandidate> &candidates, ClassLinkerContext *ctx);
    IfaceMethodDispatch ClassifySurvivors(InternalArenaVector<IfaceMethodCandidate> &survivors, Method *imethod);
    ClassOverrideResult HasClassMethodOverride(Method &imethod);
    void RebuildOwnMethodNameHashes();
    bool OwnMethodsShadowInterface(ITable itable);
    Method *FindOrCreateConflictCopiedMethod(Method *imethod);
    size_t GetCopiedMethodIndex(Method *method);

    static bool IsOverriddenBy(const ClassLinkerContext *ctx, Method::ProtoId const &base, Method::ProtoId const &derv);
    ClassOverrideResult GetInheritedClassOverride(Method *inherited) const;

    InternalArenaUnorderedSet<uint32_t> *ownMethodNameHashes_ {nullptr};
    InternalArenaUnorderedMap<uint32_t, InternalArenaVector<IfaceMethodCandidate> *> *interfaceMethodCandidatesByName_ {
        nullptr};
    bool useInterfaceMethodCandidateIndex_ {false};
    bool needReleaseAllocator_ {false};
    size_t currentPosition_ {0};
};

}  // namespace ark

#endif  // PANDA_RUNTIME_VTABLE_BUILDER_VARIANCE_H
