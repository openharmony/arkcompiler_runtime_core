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
#ifndef PANDA_RUNTIME_VTABLE_BUILDER_BASE_H
#define PANDA_RUNTIME_VTABLE_BUILDER_BASE_H

#include "libarkbase/macros.h"
#include "libarkbase/utils/hash.h"
#include "libarkbase/utils/utf.h"
#include "libarkfile/class_data_accessor-inl.h"
#include "libarkfile/file-inl.h"
#include "libarkfile/file_items.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "runtime/class_linker_context.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/runtime.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/include/vtable_builder_interface.h"
#include "runtime/mem/internal_arena_allocator.h"

namespace ark {

class ClassLinker;
class ClassLinkerContext;

class MethodInfo {
public:
    MethodInfo(const Method *method, size_t index, ClassLinkerContext *ctx, panda_file::File::EntityId classId)
        : name_(method->GetName()),
          protoId_(method->GetProtoId()),
          accessFlags_(method->GetAccessFlags()),
          ctx_(ctx),
          vmethodIndex_(index),
          classId_(classId),
          nameHash_(GetHash32String(name_.data))
    {
    }

    explicit MethodInfo(Method *method, bool isBase = false)
        : name_(method->GetName()),
          protoId_(method->GetProtoId()),
          accessFlags_(method->GetAccessFlags()),
          isBase_(isBase),
          isInterfaceMethod_(method->GetClass()->IsInterface()),
          ctx_(method->GetClass()->GetLoadContext()),
          method_(method),
          nameHash_(GetHash32String(name_.data))
    {
    }

    const panda_file::File::StringData &GetName() const
    {
        return name_;
    }

    uint32_t GetNameHash() const
    {
        return nameHash_;
    }

    const uint8_t *GetClassName() const
    {
        return method_ != nullptr ? method_->GetClass()->GetDescriptor()
                                  : protoId_.GetPandaFile().GetStringData(classId_).data;
    }

    Method::ProtoId GetProtoId() const
    {
        return protoId_;
    }

    Method *GetMethod() const
    {
        return method_;
    }

    size_t GetVirtualMethodIndex() const
    {
        return vmethodIndex_;
    }

    bool IsAbstract() const
    {
        return (accessFlags_ & ACC_ABSTRACT) != 0;
    }

    bool IsFinal() const
    {
        return (accessFlags_ & ACC_FINAL) != 0;
    }

    bool IsPublic() const
    {
        return (accessFlags_ & ACC_PUBLIC) != 0;
    }

    bool IsProtected() const
    {
        return (accessFlags_ & ACC_PROTECTED) != 0;
    }

    bool IsPrivate() const
    {
        return (accessFlags_ & ACC_PRIVATE) != 0;
    }

    bool IsInterfaceMethod() const
    {
        return isInterfaceMethod_;
    }

    bool IsBase() const
    {
        return isBase_;
    }

    ClassLinkerContext *GetLoadContext() const
    {
        return ctx_;
    }

    ~MethodInfo() = default;

    bool operator==(MethodInfo &other) = delete;

    DEFAULT_COPY_CTOR(MethodInfo);
    NO_COPY_OPERATOR(MethodInfo);
    NO_MOVE_SEMANTIC(MethodInfo);

private:
    panda_file::File::StringData name_;
    Method::ProtoId protoId_;
    uint32_t accessFlags_;
    bool isBase_ {false};
    bool isInterfaceMethod_ {false};

    ClassLinkerContext *ctx_ {nullptr};

    // Either method_ or vmethodIndex_+classId_ is initialized
    Method *method_ {nullptr};
    uint32_t vmethodIndex_ {0};
    panda_file::File::EntityId classId_;
    uint32_t nameHash_;
};

class VTableInfo {
public:
    explicit VTableInfo(mem::InternalArenaAllocator &allocator) : vmethods_(allocator), copiedMethods_(allocator) {}

    struct MethodEntry {
        explicit MethodEntry(size_t index) : index_(index) {}

        MethodInfo const *CandidateOr(MethodInfo const *orig) const
        {
            return candidate_ != nullptr ? candidate_ : orig;
        }

        size_t GetIndex() const
        {
            return index_;
        }

        MethodInfo const *GetCandidate() const
        {
            return candidate_;
        }

        void SetCandidate(MethodInfo const *candidate)
        {
            candidate_ = candidate;
        }

    private:
        size_t index_ {};
        MethodInfo const *candidate_ {};
    };

    struct CopiedMethodEntry {
        explicit CopiedMethodEntry(size_t index) : index_(index) {}

        size_t GetIndex() const
        {
            return index_;
        }

        CopiedMethod::Status GetStatus() const
        {
            return status_;
        }

        void SetStatus(CopiedMethod::Status status)
        {
            status_ = status;
        }

    private:
        size_t index_ {};
        CopiedMethod::Status status_ {CopiedMethod::Status::ORDINARY};
    };

    auto &Methods()
    {
        return vmethods_;
    }

    auto &Methods() const
    {
        return vmethods_;
    }

    auto &CopiedMethods()
    {
        return copiedMethods_;
    }

    auto &CopiedMethods() const
    {
        return copiedMethods_;
    }

    void Reserve(size_t methodCount);

    void AddEntry(const MethodInfo *info);

    void ReplaceEntryWith(const MethodInfo *prev, const MethodInfo *current);

    CopiedMethodEntry &AddCopiedEntry(const MethodInfo *info);

    CopiedMethodEntry &UpdateCopiedEntry(const MethodInfo *orig, const MethodInfo *repl);

    size_t GetVTableSize() const
    {
        return vmethods_.size() + copiedMethods_.size();
    }

    void UpdateClass(Class *klass) const;

    void DumpMappings();

    static void DumpVTable([[maybe_unused]] Class *klass);

private:
    struct MethodNameHash {
        uint32_t operator()(const MethodInfo *methodInfo) const
        {
            return methodInfo->GetNameHash();
        }
    };

    InternalArenaUnorderedMap<MethodInfo const *, MethodEntry, MethodNameHash> vmethods_;
    InternalArenaUnorderedMap<MethodInfo const *, CopiedMethodEntry, MethodNameHash> copiedMethods_;
};

template <typename Pred, typename UMap>
class FilterBucketIterator {
public:
    using LocalIter = typename UMap::local_iterator;

    FilterBucketIterator(Pred pred, UMap &umap, const typename UMap::key_type &key) : pred_(pred)
    {
        if (umap.bucket_count() == 0) {
            return;
        }
        valid_ = true;
        auto const bucket = umap.bucket(key);
        iter_ = umap.begin(bucket);
        endIter_ = umap.end(bucket);
        Advance();
    }

    bool IsEmpty() const
    {
        return !valid_ || iter_ == endIter_;
    }

    typename UMap::reference Value()
    {
        ASSERT(!IsEmpty());
        return *iter_;
    }

    void Next()
    {
        ASSERT(!IsEmpty());
        ++iter_;
        Advance();
    }

private:
    void Advance()
    {
        while (!IsEmpty() && !pred_(iter_)) {
            ++iter_;
        }
    }

    bool valid_ {};
    LocalIter iter_ {};
    LocalIter endIter_ {};
    Pred pred_;
};

template <typename UMap>
auto SameNameMethodInfoIterator(UMap &umap, MethodInfo const *info)
{
    auto pred = [info](auto other) { return other->first->GetName() == info->GetName(); };
    return FilterBucketIterator(pred, umap, info);
}

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
template <bool VISIT_SUPERITABLE>
class VTableBuilderBase : public VTableBuilder {
public:
    bool Build(panda_file::ClassDataAccessor *cda, Span<const Method> vmethods, Class *baseClass, ITable itable,
               ClassLinkerContext *ctx) override;

    bool Build(Span<Method> vmethods, Class *baseClass, ITable itable, bool isInterface) override;

    bool FilterProxyClassMethods(Span<Method *> input, PandaVector<Method *> *output, Class *baseClass) override;

    void UpdateClass(Class *klass) const override;

    size_t GetNumVirtualMethods() const override
    {
        return numVmethods_;
    }

    size_t GetVTableSize() const override
    {
        return vtable_->GetVTableSize();
    }

    Span<const CopiedMethod> GetCopiedMethods() const override
    {
        return orderedCopiedMethods_.ToConst();
    }

    Span<const IfaceMethodDispatch> GetIfaceMethodDispatches() const override
    {
        ASSERT(dispatches_ != nullptr);
        return {dispatches_->data(), dispatches_->size()};
    }

    mem::InternalArenaAllocator *GetAllocator() override
    {
        return allocator_;
    }

protected:
    explicit VTableBuilderBase(ClassLinkerErrorHandler *errHandler) : errorHandler_(errHandler) {}

    void SetAllocator(mem::InternalArenaAllocator *allocator)
    {
        ASSERT(allocator != nullptr);

        allocator_ = allocator;
        vtable_ = allocator->New<VTableInfo>(*allocator);
        allocator->MakeContainer(dispatches_);
    }

    [[nodiscard]] virtual bool ProcessClassMethod(const MethodInfo *info) = 0;
    [[nodiscard]] virtual bool ProcessDefaultMethod(ITable itable, size_t itableIdx, MethodInfo *methodInfo) = 0;

    [[nodiscard]] virtual bool ProcessProxyClassMethod([[maybe_unused]] const MethodInfo *info)
    {
        UNREACHABLE();
    }

    virtual void ResolveInterfaceMethodsHook([[maybe_unused]] ITable itable, [[maybe_unused]] size_t superItableSize) {}

    void BuildOrderedCopiedMethods();

    [[nodiscard]] bool CollectProxyMethods(PandaVector<Method *> *output);

    mem::InternalArenaAllocator *allocator_ {nullptr};
    VTableInfo *vtable_ {nullptr};
    size_t numVmethods_ {0};
    size_t baseVTableSize_ {0};
    Span<CopiedMethod> orderedCopiedMethods_ {};
    InternalArenaVector<IfaceMethodDispatch> *dispatches_ {nullptr};
    Span<MethodInfo> baseMethodInfoByIndex_ {};
    bool vtableAppendedOnly_ {true};
    ClassLinkerErrorHandler *errorHandler_ {nullptr};

private:
    void BuildForInterface(Span<const Method> vmethods);

    void AddBaseMethods(Class *baseClass);

    [[nodiscard]] bool AddClassMethods(Span<const Method> vmethods, ClassLinkerContext *ctx,
                                       panda_file::File::EntityId classId);

    [[nodiscard]] bool AddClassMethods(Span<Method> vmethods);

    [[nodiscard]] bool AddProxyClassMethods(Span<Method *> methods);

    [[nodiscard]] bool AddDefaultInterfaceMethods(ITable itable, size_t superItableSize);

    bool hasDefaultMethods_ {false};
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

}  // namespace ark

#endif  // PANDA_RUNTIME_VTABLE_BUILDER_BASE_H
