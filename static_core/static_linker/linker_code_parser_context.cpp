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

#include "libarkfile/bytecode_instruction.h"
#include "libarkfile/bytecode_instruction-inl.h"
#include "libarkfile/file_items.h"
#include "libarkfile/debug_info_updater-inl.h"

#include "linker_context.h"

namespace ark::static_linker {

namespace {

class LinkerDebugInfoUpdater : public panda_file::DebugInfoUpdater<LinkerDebugInfoUpdater> {
public:
    using Super = panda_file::DebugInfoUpdater<LinkerDebugInfoUpdater>;

    LinkerDebugInfoUpdater(const panda_file::File *file, panda_file::ItemContainer *cont) : Super(file), cont_(cont) {}

    panda_file::StringItem *GetOrCreateStringItem(const std::string &s)
    {
        return cont_->GetOrCreateStringItem(s);
    }

    panda_file::BaseClassItem *GetType([[maybe_unused]] panda_file::File::EntityId typeId, const std::string &typeName)
    {
        auto *cm = cont_->GetClassMap();
        auto iter = cm->find(typeName);
        if (iter != cm->end()) {
            return iter->second;
        }
        return nullptr;
    }

private:
    panda_file::ItemContainer *cont_;
};

class LinkerDebugInfoScrapper : public panda_file::DebugInfoUpdater<LinkerDebugInfoScrapper> {
public:
    using Super = panda_file::DebugInfoUpdater<LinkerDebugInfoScrapper>;

    LinkerDebugInfoScrapper(const panda_file::File *file, CodePatcher *patcher,
                            const std::map<std::string, panda_file::BaseClassItem *> *classMap)
        : Super(file), patcher_(patcher), classMap_(classMap)
    {
    }

    panda_file::StringItem *GetOrCreateStringItem(const std::string &s)
    {
        patcher_->Add(s);
        return nullptr;
    }

    panda_file::BaseClassItem *GetType([[maybe_unused]] panda_file::File::EntityId typeId, const std::string &typeName)
    {
        if (classMap_->find(typeName) == classMap_->end()) {
            // This action must create a string for primitive types.
            GetOrCreateStringItem(typeName);
        }
        return nullptr;
    }

private:
    CodePatcher *patcher_;
    const std::map<std::string, panda_file::BaseClassItem *> *classMap_;
};

void PatchDebugInfo(const CodePatcher::DebugInfoPatch &patch)
{
    auto updater = LinkerDebugInfoUpdater(patch.file, patch.cont);
    auto *constantPool = patch.debugInfo->GetConstantPool();
    auto *lineNumberProgram = patch.debugInfo->GetLineNumberProgram();
    if (patch.patchLineNumberProgram || lineNumberProgram->GetData().empty()) {
        updater.Emit(lineNumberProgram, constantPool, patch.debugInfoId);
        return;
    }

    // The shared line-number program is emitted by its owner method; this debug item only needs argument data.
    panda_file::LineNumberProgramItemBase argumentOnlyProgram;
    updater.Emit(&argumentOnlyProgram, constantPool, patch.debugInfoId);
}
}  // namespace

void CodePatcher::Add(Change c)
{
    changes_.emplace_back(std::move(c));
}

void CodePatcher::Devour(CodePatcher &&p)
{
    const auto oldSize = changes_.size();
    changes_.insert(changes_.end(), std::move_iterator(p.changes_.begin()), std::move_iterator(p.changes_.end()));
    for (auto range : p.bytecodePatchRanges_) {
        bytecodePatchRanges_.emplace_back(range.first + oldSize, range.second + oldSize);
    }
    ClearAndReleaseStorage(p.changes_);
    ClearAndReleaseStorage(p.bytecodePatchRanges_);
}

void CodePatcher::AddBytecodePatchRange(std::pair<size_t, size_t> range)
{
    if (range.first != range.second) {
        bytecodePatchRanges_.push_back(range);
    }
}

void CodePatcher::ApplyStringChange(StringChange *change, Context *ctx)
{
    if (change->oldItem != nullptr) {
        change->it = ctx->StringFromOld(change->oldItem);
    } else {
        auto sData = change->file->GetStringData(change->oldId);
        change->it = ctx->GetContainer().GetOrCreateStringItem(utf::Mutf8AsCString(sData.data));
    }
    change->file = nullptr;
    change->oldItem = nullptr;
}

void CodePatcher::ApplyLiteralArrayChange(LiteralArrayChange *change, Context *ctx)
{
    auto id = ctx->literalArrayId_++;
    change->it = ctx->cont_.GetOrCreateLiteralArrayItem(std::to_string(id));
    if (auto cached = ctx->literalArrayCache_.find(change->old); cached != ctx->literalArrayCache_.end()) {
        change->it->AddItems(cached->second);
        return;
    }

    auto &oldItems = change->old->GetItems();
    auto newItems = std::vector<panda_file::LiteralItem>();
    newItems.reserve(oldItems.size());

    for (const auto &item : oldItems) {
        using LiteralType = panda_file::LiteralItem::Type;
        switch (item.GetType()) {
            case LiteralType::B1:
            case LiteralType::B2:
            case LiteralType::B4:
            case LiteralType::B8:
                newItems.emplace_back(item);
                break;
            case LiteralType::STRING:
                newItems.emplace_back(ctx->StringFromOld(item.GetValue<panda_file::StringItem *>()));
                break;
            case LiteralType::METHOD: {
                auto *method = item.GetValue<panda_file::MethodItem *>();
                auto known = ctx->knownItems_.find(method);
                ASSERT(known != ctx->knownItems_.end());
                ASSERT(known->second->GetItemType() == panda_file::ItemTypes::METHOD_ITEM);
                newItems.emplace_back(static_cast<panda_file::MethodItem *>(known->second));
                break;
            }
            default:
                UNREACHABLE();
        }
    }

    change->it->AddItems(newItems);
    ctx->literalArrayCache_.emplace(change->old, std::move(newItems));
}

void CodePatcher::ApplyStringDependency(std::string *value, Context *ctx)
{
    auto stringItem = ctx->GetContainer().GetOrCreateStringItem(*value);
    stringItem->SetDependencyMark();
    std::string().swap(*value);
}

void CodePatcher::ApplyDeps(Context *ctx)
{
    for (auto &v : changes_) {
        if (auto stringChange = std::get_if<StringChange>(&v); stringChange != nullptr) {
            ApplyStringChange(stringChange, ctx);
        } else if (auto literalArrayChange = std::get_if<LiteralArrayChange>(&v); literalArrayChange != nullptr) {
            ApplyLiteralArrayChange(literalArrayChange, ctx);
        } else if (auto stringDependency = std::get_if<std::string>(&v); stringDependency != nullptr) {
            ApplyStringDependency(stringDependency, ctx);
        }
    }
}

bool CodePatcher::IsBytecodePatchChange(const Change &change)
{
    return std::holds_alternative<IndexedChange>(change) || std::holds_alternative<StringChange>(change) ||
           std::holds_alternative<LiteralArrayChange>(change);
}

void CodePatcher::RebuildBytecodePatchRanges(const std::vector<size_t> &newIndexes, size_t skippedIndex)
{
    auto newRanges = std::vector<std::pair<size_t, size_t>>();
    newRanges.reserve(bytecodePatchRanges_.size());
    for (auto [start, end] : bytecodePatchRanges_) {
        auto newStart = skippedIndex;
        auto newEnd = skippedIndex;
        auto hasBytecodePatch = false;
        for (auto idx = start; idx < end; idx++) {
            auto mapped = newIndexes[idx];
            if (mapped == skippedIndex) {
                continue;
            }
            newStart = newStart == skippedIndex ? mapped : newStart;
            newEnd = mapped + 1U;
            hasBytecodePatch = hasBytecodePatch || IsBytecodePatchChange(changes_[mapped]);
        }
        if (hasBytecodePatch) {
            newRanges.emplace_back(newStart, newEnd);
        }
    }
    bytecodePatchRanges_.swap(newRanges);
}

void CodePatcher::TryDeletePatch()
{
    auto shouldDelete = [](auto &a) {
        using T = std::remove_cv_t<std::remove_reference_t<decltype(a)>>;
        if constexpr (std::is_same_v<T, IndexedChange>) {
            return !a.mi->GetDependencyMark();
        } else if constexpr (std::is_same_v<T, StringChange>) {
            return !a.mi->GetDependencyMark();
        } else if constexpr (std::is_same_v<T, LiteralArrayChange>) {
            return !a.mi->GetDependencyMark();
        } else if constexpr (std::is_same_v<T, DebugInfoPatch>) {
            return !a.method->GetDependencyMark();
        } else if constexpr (std::is_same_v<T, std::string>) {
            return false;
        } else {
            UNREACHABLE();
        }
    };

    auto skippedIndex = changes_.size();
    auto newIndexes = std::vector<size_t>(changes_.size(), skippedIndex);
    auto out = changes_.begin();
    size_t newIndex = 0;
    size_t oldIndex = 0;
    for (auto it = changes_.begin(); it != changes_.end(); ++it) {
        if (std::visit(shouldDelete, *it)) {
            oldIndex++;
            continue;
        }
        newIndexes[oldIndex++] = newIndex++;
        if (out != it) {
            *out = std::move(*it);
        }
        ++out;
    }
    changes_.erase(out, changes_.end());
    // Keep strip-unused on the parallel patch path: compaction changes indexes, but method-local bytecode ranges
    // remain independent after their surviving entries are remapped.
    RebuildBytecodePatchRanges(newIndexes, skippedIndex);
}

void CodePatcher::Patch(const std::pair<size_t, size_t> range)
{
    PatchBytecode(range);
    PatchDebug(range);
}

void CodePatcher::PatchBytecode(const std::pair<size_t, size_t> range)
{
    for (size_t i = range.first; i < range.second; i++) {
        auto &v = changes_[i];
        std::visit(
            [](auto &a) {
                using T = std::remove_cv_t<std::remove_reference_t<decltype(a)>>;
                if constexpr (std::is_same_v<T, IndexedChange>) {
                    auto idx = a.it->GetIndex(a.mi);
                    a.inst.UpdateId(BytecodeId(idx));
                } else if constexpr (std::is_same_v<T, StringChange>) {
                    ASSERT(a.it != nullptr);
                    auto off = a.it->GetOffset();
                    a.inst.UpdateId(BytecodeId(off));
                } else if constexpr (std::is_same_v<T, LiteralArrayChange>) {
                    auto off = a.it->GetIndex();
                    a.inst.UpdateId(BytecodeId(off));
                } else if constexpr (std::is_same_v<T, std::string>) {
                    // nothing
                } else if constexpr (std::is_same_v<T, DebugInfoPatch>) {
                    // Debug info patching mutates debug structures and is handled by PatchDebug().
                } else {
                    UNREACHABLE();
                }
            },
            v);
    }
}

void CodePatcher::PatchBytecodeRanges(const std::pair<size_t, size_t> range)
{
    for (size_t i = range.first; i < range.second; i++) {
        PatchBytecode(bytecodePatchRanges_[i]);
    }
}

void CodePatcher::PatchDebug(const std::pair<size_t, size_t> range)
{
    for (size_t i = range.first; i < range.second; i++) {
        auto &v = changes_[i];
        std::visit(
            [](auto &a) {
                using T = std::remove_cv_t<std::remove_reference_t<decltype(a)>>;
                if constexpr (std::is_same_v<T, DebugInfoPatch>) {
                    PatchDebugInfo(a);
                } else if constexpr (std::is_same_v<T, IndexedChange> || std::is_same_v<T, StringChange> ||
                                     std::is_same_v<T, LiteralArrayChange> || std::is_same_v<T, std::string>) {
                    // Bytecode patching is handled by PatchBytecode().
                } else {
                    UNREACHABLE();
                }
            },
            v);
    }
}

void CodePatcher::AddStringDependency()
{
    auto markStringDependency = [](auto &a) {
        using T = std::remove_cv_t<std::remove_reference_t<decltype(a)>>;
        if constexpr (std::is_same_v<T, StringChange>) {
            if (a.mi->GetDependencyMark()) {
                ASSERT(a.it != nullptr);
                a.it->SetDependencyMark();
            }
        }
    };

    for (auto it = changes_.begin(); it != changes_.end();) {
        auto &v = *it;
        std::visit(markStringDependency, v);
        ++it;
    }
}

void Context::HandleStringId(const InstructionIdContext &ctx, const BytecodeInstruction &inst)
{
    BytecodeId bId = inst.GetId();
    auto oldId = bId.AsFileId();
    auto iter = ctx.items.find(oldId);
    if (iter != ctx.items.end()) {
        ASSERT(iter->second->GetItemType() == panda_file::ItemTypes::STRING_ITEM);
        ctx.patcher.Add(CodePatcher::StringChange {inst, &ctx.file, oldId,
                                                   static_cast<panda_file::StringItem *>(iter->second), ctx.data.nmi});
        return;
    }
    ctx.patcher.Add(CodePatcher::StringChange {inst, &ctx.file, oldId, nullptr, ctx.data.nmi});
}

void Context::HandleLiteralArrayId(const InstructionIdContext &ctx, const BytecodeInstruction &inst)
{
    BytecodeId bId = inst.GetId();
    auto oldIdx = bId.AsRawValue();
    auto arrs = ctx.file.GetLiteralArrays();
    ASSERT(oldIdx < arrs.size());
    auto off = arrs[oldIdx];
    auto iter = ctx.items.find(panda_file::File::EntityId(off));
    ASSERT(iter != ctx.items.end());
    ASSERT(iter->second->GetItemType() == panda_file::ItemTypes::LITERAL_ARRAY_ITEM);

    auto *old = static_cast<panda_file::LiteralArrayItem *>(iter->second);
    ctx.patcher.Add(CodePatcher::LiteralArrayChange {inst, ctx.data.nmi, old});
}

void Context::HandleIndexedId(const InstructionIdContext &ctx, const BytecodeInstruction &inst, IndexResolver resolve)
{
    auto idx = inst.GetId().AsIndex();
    auto oldId = (ctx.file.*resolve)(ctx.methodId, idx);
    auto iter = ctx.items.find(oldId);
    ASSERT(iter != ctx.items.end());
    auto *asIndexed = static_cast<panda_file::IndexedItem *>(iter->second);
    auto found = knownItems_.find(asIndexed);
    ASSERT(found != knownItems_.end());
    auto *newItem = static_cast<panda_file::IndexedItem *>(found->second);
    ctx.data.nmi->AddIndexDependency(newItem);
    ctx.patcher.Add(CodePatcher::IndexedChange {inst, ctx.data.nmi, newItem});
}

Context::IndexResolver Context::GetIndexResolver(const BytecodeInstruction &inst)
{
    using Flags = ark::BytecodeInst<ark::BytecodeInstMode::FAST>::Flags;
    if (inst.HasFlag(Flags::TYPE_ID)) {
        return &panda_file::File::ResolveClassIndex;
    }
    if (inst.HasFlag(Flags::METHOD_ID) || inst.HasFlag(Flags::STATIC_METHOD_ID)) {
        return &panda_file::File::ResolveMethodIndex;
    }
    if (inst.HasFlag(Flags::FIELD_ID) || inst.HasFlag(Flags::STATIC_FIELD_ID)) {
        return &panda_file::File::ResolveFieldIndex;
    }
    return nullptr;
}

void Context::HandleInstructionId(const InstructionIdContext &ctx, const BytecodeInstruction &inst)
{
    using Flags = ark::BytecodeInst<ark::BytecodeInstMode::FAST>::Flags;

    if (auto resolve = GetIndexResolver(inst); resolve != nullptr) {
        HandleIndexedId(ctx, inst, resolve);
        return;
    }
    if (inst.HasFlag(Flags::STRING_ID)) {
        HandleStringId(ctx, inst);
    } else if (inst.HasFlag(Flags::LITERALARRAY_ID)) {
        HandleLiteralArrayId(ctx, inst);
    }
}

void Context::MakeChangeWithId(CodePatcher &p, CodeData *data)
{
    using EntityId = panda_file::File::EntityId;

    const auto myId = EntityId(data->omi->GetOffset());
    auto *items = data->fileReader->GetItems();
    auto inst = BytecodeInstruction(data->code->data());
    size_t offset = 0;
    const auto limit = data->code->size();

    const InstructionIdContext idContext {p, *data->fileReader->GetFilePtr(), *items, myId, *data};

    while (offset < limit) {
        const auto format = inst.GetFormat();
        const auto instSize = BytecodeInstruction::Size(format);
        if (offset + instSize > limit) {
            LOG(FATAL, STATIC_LINKER) << "Invalid code size: " << limit << ", offset: " << offset
                                      << ", instruction size: " << instSize;
            break;
        }
        if (!BytecodeInstruction::HasId(format, 0)) {
            offset += instSize;
            inst = inst.JumpTo(instSize);
            continue;
        }

        HandleInstructionId(idContext, inst);

        offset += instSize;
        inst = inst.JumpTo(instSize);
    }
    if (offset != limit) {
        LOG(FATAL, STATIC_LINKER) << "Code size mismatch: expected " << limit << ", got " << offset;
    }
}

void Context::ProcessCodeData(CodePatcher &p, CodeData *data)
{
    if (data->code != nullptr) {
        MakeChangeWithId(p, data);
    }

    auto *dbg = data->omi->GetDebugInfo();
    if (dbg == nullptr || conf_.stripDebugInfo) {
        return;
    }

    // Collect string items for each method with debug information.
    auto file = data->fileReader->GetFilePtr();
    auto scrapper = LinkerDebugInfoScrapper(file, &p, cont_.GetClassMap());
    auto off = dbg->GetOffset();
    ASSERT(off != 0);
    auto eId = panda_file::File::EntityId(off);
    scrapper.Scrap(eId);

    auto newDbg = data->nmi->GetDebugInfo();
    p.Add(CodePatcher::DebugInfoPatch {file, &cont_, newDbg, eId, data->nmi, data->patchLnp});
}

}  // namespace ark::static_linker
