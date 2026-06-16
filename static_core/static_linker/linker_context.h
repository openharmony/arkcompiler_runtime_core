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

#ifndef PANDA_LINKER_CONTEXT_H
#define PANDA_LINKER_CONTEXT_H

#include <forward_list>
#include <functional>
#include <array>
#include <unordered_map>
#include <vector>

#include "libarkfile/bytecode_instruction.h"
#include "libarkfile/file_items.h"
#include "libarkfile/file_reader.h"
#include "libarkfile/file_item_container.h"

#include "linker.h"
#include "libarkbase/macros.h"

namespace ark::static_linker {

template <class Container>
void ClearAndReleaseStorage(Container &container)
{
    // clear() keeps capacity; swapping with an empty container releases the backing storage.
    container.clear();
    Container empty;
    container.swap(empty);
}

class Context;

class CodePatcher {
public:
    struct IndexedChange {
        BytecodeInstruction inst;
        panda_file::MethodItem *mi;
        panda_file::IndexedItem *it;
    };

    struct StringChange {
        BytecodeInstruction inst;
        const panda_file::File *file;
        panda_file::File::EntityId oldId;
        const panda_file::StringItem *oldItem;
        panda_file::MethodItem *mi;
        panda_file::StringItem *it {};
    };

    struct LiteralArrayChange {
        BytecodeInstruction inst;
        panda_file::MethodItem *mi;
        panda_file::LiteralArrayItem *old;

        panda_file::LiteralArrayItem *it {};
    };

    struct DebugInfoPatch {
        const panda_file::File *file;
        panda_file::ItemContainer *cont;
        panda_file::DebugInfoItem *debugInfo;
        panda_file::File::EntityId debugInfoId;
        panda_file::MethodItem *method;
        bool patchLineNumberProgram;
    };

    using Change = std::variant<IndexedChange, StringChange, LiteralArrayChange>;
    using NonBytecodeChange = std::variant<std::string, DebugInfoPatch>;

    void Add(Change c);
    void AddNonBc(NonBytecodeChange c);

    void ApplyDeps(Context *ctx);

    void TryDeletePatch();
    void PatchBytecode(const std::pair<size_t, size_t> range);
    void PatchDebug();
    void PatchBytecodeRanges(const std::pair<size_t, size_t> range);
    void AddStringDependency();

    void Devour(CodePatcher &&p);
    void AddBytecodePatchRange(std::pair<size_t, size_t> range);

    void ReserveChanges(size_t size)
    {
        changes_.reserve(size);
    }

    void ReserveRanges(size_t size)
    {
        bytecodePatchRanges_.reserve(size);
    }

    void Clear()
    {
        ClearAndReleaseStorage(changes_);
        ClearAndReleaseStorage(nonBcChanges_);
        ClearAndReleaseStorage(bytecodePatchRanges_);
    }

    size_t GetSize() const
    {
        return changes_.size();
    }

    size_t GetBytecodePatchRangeCount() const
    {
        return bytecodePatchRanges_.size();
    }

private:
    static void ApplyStringChange(StringChange *change, Context *ctx);
    static void ApplyLiteralArrayChange(LiteralArrayChange *change, Context *ctx);
    static void ApplyStringDependency(std::string *value, Context *ctx);
    void RebuildBytecodePatchRanges(const std::vector<size_t> &newIndexes, size_t skippedIndex);

    std::vector<Change> changes_;
    std::vector<NonBytecodeChange> nonBcChanges_;
    std::vector<std::pair<size_t, size_t>> bytecodePatchRanges_;
};

struct CodeData {
    std::vector<uint8_t> *code;
    panda_file::MethodItem *omi;
    panda_file::MethodItem *nmi;
    const panda_file::FileReader *fileReader;

    bool patchLnp;
};

class Helpers {
public:
    static std::vector<panda_file::Type> BreakProto(panda_file::ProtoItem *p);

    static void FillProtoTypes(panda_file::ProtoItem *p, std::vector<panda_file::Type> &out);
};

class Context {
public:
    explicit Context(Config conf);

    ~Context();

    NO_COPY_SEMANTIC(Context);
    NO_MOVE_SEMANTIC(Context);

    void Write(const std::string &out);

    void Read(const std::vector<std::string> &input);

    void Merge();

    void Parse();

    void ComputeLayout();

    void Patch();

    void TryDelete();

    void ReleasePreWriteState();

    const std::unordered_map<panda_file::BaseItem *, panda_file::BaseItem *> &GetKnownItems() const
    {
        if (preWriteStateReleased_) {
            return emptyKnownItems_;
        }
        return knownItems_;
    }

    panda_file::ItemContainer &GetContainer()
    {
        return cont_;
    }

    const panda_file::ItemContainer &GetContainer() const
    {
        return cont_;
    }

    const Result &GetResult() const
    {
        return result_;
    }

    bool HasErrors() const
    {
        return !result_.errors.empty();
    }

private:
    friend class CodePatcher;

    Config conf_;
    Result result_;
    panda_file::ItemContainer cont_;

    std::vector<std::function<void()>> deferredFailedAnnotations_;

    std::vector<CodeData> codeDatas_;
    CodePatcher patcher_;

    std::forward_list<panda_file::FileReader> readers_;
    bool preWriteStateReleased_ {false};
    std::unordered_map<panda_file::BaseItem *, panda_file::BaseItem *> knownItems_;
    std::unordered_map<panda_file::BaseItem *, panda_file::BaseItem *> emptyKnownItems_;

    // Output item provenance used to add source file names to diagnostics.
    std::vector<std::pair<const panda_file::BaseItem *, const panda_file::FileReader *>> cameFrom_;
    size_t literalArrayId_ {};

    struct ClassMergeRecord {
        panda_file::ClassItem *oldItem;
        panda_file::ClassItem *newItem;
        const panda_file::FileReader *reader;
    };
    std::vector<ClassMergeRecord> classMergeRecords_;

    // Foreign member resolution is keyed by item identity.  Ordering is irrelevant, so typed hash keys avoid
    // std::map tuple comparisons in the merge hot path.
    static constexpr size_t HASH_MAGIC = 0x9E3779B9U;  // CC-OFF(G.NAM.03-CPP) project code style
    static constexpr size_t HASH_LEFT_SHIFT = 6U;      // CC-OFF(G.NAM.03-CPP) project code style
    static constexpr size_t HASH_RIGHT_SHIFT = 2U;     // CC-OFF(G.NAM.03-CPP) project code style

    class ForeignFieldKey {
    public:
        ForeignFieldKey(panda_file::BaseClassItem *klass, panda_file::StringItem *name, panda_file::TypeItem *type)
            : klass_(klass), name_(name), type_(type)
        {
        }

        bool operator==(const ForeignFieldKey &other) const
        {
            return klass_ == other.klass_ && name_ == other.name_ && type_ == other.type_;
        }

        panda_file::BaseClassItem *GetClass() const
        {
            return klass_;
        }

        panda_file::StringItem *GetName() const
        {
            return name_;
        }

        panda_file::TypeItem *GetType() const
        {
            return type_;
        }

    private:
        panda_file::BaseClassItem *klass_;
        panda_file::StringItem *name_;
        panda_file::TypeItem *type_;
    };

    class ForeignMethodKey {
    public:
        ForeignMethodKey(panda_file::BaseClassItem *klass, panda_file::StringItem *name, panda_file::ProtoItem *proto,
                         uint32_t accessFlags)
            : klass_(klass), name_(name), proto_(proto), accessFlags_(accessFlags)
        {
        }

        bool operator==(const ForeignMethodKey &other) const
        {
            return klass_ == other.klass_ && name_ == other.name_ && proto_ == other.proto_ &&
                   accessFlags_ == other.accessFlags_;
        }

        panda_file::BaseClassItem *GetClass() const
        {
            return klass_;
        }

        panda_file::StringItem *GetName() const
        {
            return name_;
        }

        panda_file::ProtoItem *GetProto() const
        {
            return proto_;
        }

        uint32_t GetAccessFlags() const
        {
            return accessFlags_;
        }

    private:
        panda_file::BaseClassItem *klass_;
        panda_file::StringItem *name_;
        panda_file::ProtoItem *proto_;
        uint32_t accessFlags_;
    };

    class ForeignFieldKeyHash {
    public:
        size_t operator()(const ForeignFieldKey &key) const
        {
            return CombineHash(CombineHash(CombineHash(0, key.GetClass()), key.GetName()), key.GetType());
        }
    };

    class ForeignMethodKeyHash {
    public:
        size_t operator()(const ForeignMethodKey &key) const
        {
            auto hash = CombineHash(CombineHash(CombineHash(0, key.GetClass()), key.GetName()), key.GetProto());
            return hash ^ (std::hash<uint32_t> {}(key.GetAccessFlags()) + HASH_MAGIC + (hash << HASH_LEFT_SHIFT) +
                           (hash >> HASH_RIGHT_SHIFT));
        }
    };

    class MethodResolutionKey {
    public:
        MethodResolutionKey(panda_file::BaseClassItem *klass, panda_file::StringItem *name,
                            panda_file::ProtoItem *proto)
            : klass_(klass), name_(name), proto_(proto)
        {
        }

        bool operator==(const MethodResolutionKey &other) const
        {
            return klass_ == other.klass_ && name_ == other.name_ && proto_ == other.proto_;
        }

        panda_file::BaseClassItem *GetClass() const
        {
            return klass_;
        }

        panda_file::StringItem *GetName() const
        {
            return name_;
        }

        panda_file::ProtoItem *GetProto() const
        {
            return proto_;
        }

    private:
        panda_file::BaseClassItem *klass_;
        panda_file::StringItem *name_;
        panda_file::ProtoItem *proto_;
    };

    class MethodResolutionKeyHash {
    public:
        size_t operator()(const MethodResolutionKey &key) const
        {
            return CombineHash(CombineHash(CombineHash(0, key.GetClass()), key.GetName()), key.GetProto());
        }
    };

    static size_t CombineHash(size_t seed, const void *ptr)
    {
        return seed ^
               (std::hash<const void *> {}(ptr) + HASH_MAGIC + (seed << HASH_LEFT_SHIFT) + (seed >> HASH_RIGHT_SHIFT));
    }

    struct ProtoData {
        panda_file::ProtoItem *proto;
        std::vector<panda_file::MethodParamItem> params;
    };

    std::unordered_map<ForeignFieldKey, panda_file::ForeignFieldItem *, ForeignFieldKeyHash> foreignFields_;
    std::unordered_map<ForeignMethodKey, panda_file::ForeignMethodItem *, ForeignMethodKeyHash> foreignMethods_;
    std::unordered_map<MethodResolutionKey, panda_file::MethodItem *, MethodResolutionKeyHash> resolvedMethods_;
    using ResolvedField = std::variant<panda_file::FieldItem *, panda_file::ForeignClassItem *>;

    std::unordered_map<ForeignFieldKey, ResolvedField, ForeignFieldKeyHash> resolvedFields_;

    // Merge repeatedly converts old-file entities into output-container entities.  These caches keep that conversion
    // single-pass for common proto/string/literal-array references.
    std::unordered_map<panda_file::ProtoItem *, ProtoData> protoCache_;
    std::unordered_map<const panda_file::StringItem *, panda_file::StringItem *> stringCache_;
    std::unordered_map<panda_file::LiteralArrayItem *, std::vector<panda_file::LiteralItem>> literalArrayCache_;

    std::vector<panda_file::Type> protoTypsScratch_;

    static constexpr size_t K_PRIMITIVE_TYPE_CACHE_SIZE = 16U;
    std::array<panda_file::PrimitiveTypeItem *, K_PRIMITIVE_TYPE_CACHE_SIZE> primitiveTypeCache_ {};

    panda_file::PrimitiveTypeItem *PrimitiveTypeFromCache(panda_file::Type::TypeId id);

    // The initial merge scan gathers counts for reserve() and records foreign items for the required deterministic
    // merge order: regular class declarations, foreign classes, regular bodies, then foreign members.
    struct MergeItemBuckets {
        size_t totalItems {};
        size_t methodItems {};
        size_t cameFromItems {};
        std::vector<std::pair<panda_file::BaseItem *, const panda_file::FileReader *>> foreignClasses;
        std::vector<std::pair<panda_file::BaseItem *, const panda_file::FileReader *>> foreignMembers;
    };

    MergeItemBuckets CollectMergeItemBuckets();

    panda_file::BaseClassItem *ClassFromOld(panda_file::BaseClassItem *old);

    panda_file::TypeItem *TypeFromOld(panda_file::TypeItem *old);

    panda_file::StringItem *StringFromOld(const panda_file::StringItem *s);

    static std::string GetStr(const panda_file::StringItem *si);

    void MergeClass(const panda_file::FileReader *reader, panda_file::ClassItem *ni, panda_file::ClassItem *oi);

    panda_file::ClassItem *GetOrCreateRegularClass(const std::string &name, const panda_file::FileReader *reader);

    void AddRegularClasses();

    void FillRegularClasses();

    void MergeMethod(const panda_file::FileReader *reader, panda_file::ClassItem *clz, panda_file::MethodItem *oi);

    void MergeField(const panda_file::FileReader *reader, panda_file::ClassItem *clz, panda_file::FieldItem *oi);

    void MergeForeignMethod(const panda_file::FileReader *reader, panda_file::ForeignMethodItem *fm);

    void MergeForeignMethodCreate(const panda_file::FileReader *reader, panda_file::BaseClassItem *clz,
                                  panda_file::ForeignMethodItem *fm, panda_file::StringItem *name,
                                  panda_file::ProtoItem *proto);

    void MergeForeignField(const panda_file::FileReader *reader, panda_file::ForeignFieldItem *ff);

    void MergeForeignFieldCreate(const panda_file::FileReader *reader, panda_file::BaseClassItem *clz,
                                 panda_file::ForeignFieldItem *ff, panda_file::StringItem *name,
                                 panda_file::TypeItem *type);

    struct FieldResolutionData {
        const panda_file::FileReader *reader;
        panda_file::ForeignFieldItem *oldField;
        ForeignFieldKey key;
        panda_file::StringItem *name;
        panda_file::TypeItem *type;
    };

    void ApplyResolvedField(const FieldResolutionData &data, ResolvedField resolvedField, bool cacheResult);

    std::pair<bool, bool> UpdateDebugInfo(panda_file::MethodItem *ni, panda_file::MethodItem *oi);

    void CreateTryBlocks(panda_file::MethodItem *ni, panda_file::CodeItem *nci, panda_file::MethodItem *oi,
                         panda_file::CodeItem *oci);

    bool IsSameProto(panda_file::ProtoItem *op1, panda_file::ProtoItem *op2);

    template <typename T>
    struct AddAnnotationImplData {
        const panda_file::FileReader *reader;
        T *ni;
        T *oi;
        size_t from;
        size_t retriesLeft;
    };

    template <typename T, typename Getter, typename Adder>
    void AddAnnotationImpl(AddAnnotationImplData<T> ad, Getter getter, Adder adder);

    template <typename T>
    void TransferAnnotations(const panda_file::FileReader *reader, T *ni, T *oi);

    const ProtoData &GetProto(panda_file::ProtoItem *p);

    bool IsSameType(ark::panda_file::TypeItem *nevv, ark::panda_file::TypeItem *old);

    void ProcessCodeData(CodePatcher &p, CodeData *data);

    void ApplyPatchDependencies();

    size_t EstimatePatchChanges(size_t start, size_t end) const;

    void ProcessCodeDataRange(CodePatcher *patcher, size_t start, size_t end);

    void MakeChangeWithId(CodePatcher &p, CodeData *data);

    using ItemsMap = std::map<panda_file::File::EntityId, panda_file::BaseItem *>;

    using IndexResolver = panda_file::File::EntityId (panda_file::File::*)(panda_file::File::EntityId,
                                                                           panda_file::File::Index) const;

    struct InstructionIdContext {
        CodePatcher &patcher;
        const panda_file::File &file;
        const ItemsMap &items;
        panda_file::File::EntityId methodId;
        CodeData &data;
    };

    void HandleStringId(const InstructionIdContext &ctx, const BytecodeInstruction &inst);

    void HandleIndexedId(const InstructionIdContext &ctx, const BytecodeInstruction &inst, IndexResolver resolve);

    void HandleLiteralArrayId(const InstructionIdContext &ctx, const BytecodeInstruction &inst);

    void AddItemToKnown(panda_file::BaseItem *item, const std::map<std::string, panda_file::BaseClassItem *> &cm,
                        const panda_file::FileReader &reader);

    void MergeItem(panda_file::BaseItem *item, const panda_file::FileReader &reader);

    void HandleCandidates(const panda_file::FileReader *reader, const std::vector<panda_file::FieldItem *> &candidates,
                          panda_file::ForeignFieldItem *ff);

    bool MethodFind(const std::string &className, const std::string &methodName,
                    std::map<std::string, panda_file::BaseClassItem *> &classesMap);

    bool FileFind(const std::string &fileName, std::map<std::string, panda_file::BaseClassItem *> &classesMap);

    bool HandleEntryDependencies();

    class ErrorDetail {
    public:
        using InfoType = std::variant<const panda_file::BaseItem *, std::string>;

        ErrorDetail(std::string name, const panda_file::BaseItem *item1) : name_(std::move(name)), info_(item1)
        {
            ASSERT(item1 != nullptr);
        }

        explicit ErrorDetail(std::string name, std::string data = "") : name_(std::move(name)), info_(std::move(data))
        {
        }

        const std::string &GetName() const
        {
            return name_;
        }

        const InfoType &GetInfo() const
        {
            return info_;
        }

    private:
        std::string name_;
        InfoType info_;
    };

    class ErrorToStringWrapper {
    public:
        ErrorToStringWrapper(Context *ctx, ErrorDetail error, size_t indent)
            : error_(std::move(error)), indent_(indent), ctx_(ctx)
        {
        }

        DEFAULT_COPY_SEMANTIC(ErrorToStringWrapper);
        DEFAULT_MOVE_SEMANTIC(ErrorToStringWrapper);

        ~ErrorToStringWrapper() = default;

        friend std::ostream &operator<<(std::ostream &o, const ErrorToStringWrapper &self);

    private:
        void Print(std::ostream &o) const;
        void PrintInfo(std::ostream &o, const std::string &info) const;
        void PrintInfo(std::ostream &o, const panda_file::BaseItem *info) const;

        ErrorDetail error_;
        size_t indent_;
        Context *ctx_;
    };

    ErrorToStringWrapper ErrorToString(ErrorDetail error, size_t indent = 0)
    {
        return ErrorToStringWrapper {this, std::move(error), indent};
    }

    friend std::ostream &operator<<(std::ostream &o, const ErrorToStringWrapper &self);

    void Error(const std::string &msg, const std::vector<ErrorDetail> &details,
               const panda_file::FileReader *reader = nullptr);

    using FieldSearchResult = std::variant<std::monostate, panda_file::FieldItem *, panda_file::ForeignClassItem *>;
    using MethodSearchResult = std::variant<bool, panda_file::MethodItem *>;

    FieldSearchResult TryFindField(panda_file::BaseClassItem *klass, const std::string &name,
                                   panda_file::TypeItem *expectedType,
                                   std::vector<panda_file::FieldItem *> *badCandidates);

    MethodSearchResult TryFindMethod(panda_file::BaseClassItem *klass, panda_file::StringItem *name,
                                     panda_file::ProtoItem *proto, std::vector<ErrorDetail> *relatedItems);

    std::variant<panda_file::AnnotationItem *, ErrorDetail> AnnotFromOld(panda_file::AnnotationItem *oa);

    std::variant<panda_file::ValueItem *, ErrorDetail> ArrayValueFromOld(panda_file::ValueItem *oi);

    std::variant<panda_file::ValueItem *, ErrorDetail> ValueFromOld(panda_file::ValueItem *oi);

    std::variant<panda_file::BaseItem *, ErrorDetail> ScalarValueIdFromOld(panda_file::BaseItem *oi);
};

std::ostream &operator<<(std::ostream &o, const static_linker::Context::ErrorToStringWrapper &self);

}  // namespace ark::static_linker

#endif
