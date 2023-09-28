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

#include "libpandafile/bytecode_instruction.h"
#include "libpandafile/bytecode_instruction-inl.h"
#include "libpandafile/file_items.h"
#include "libpandafile/debug_info_updater-inl.h"

#include "linker_context.h"

namespace panda::static_linker {

namespace {

class LinkerDebugInfoUpdater : public panda_file::DebugInfoUpdater<LinkerDebugInfoUpdater> {
public:
    using Super = panda_file::DebugInfoUpdater<LinkerDebugInfoUpdater>;

    LinkerDebugInfoUpdater(const panda_file::File *file, panda_file::ItemContainer *cont) : Super(file), cont_(cont) {}

    panda_file::StringItem *GetOrCreateStringItem(const std::string &s)
    {
        return cont_->GetOrCreateStringItem(s);
    }

    panda_file::BaseClassItem *GetType([[maybe_unused]] panda_file::File::EntityId type_id,
                                       const std::string &type_name)
    {
        auto *cm = cont_->GetClassMap();
        auto iter = cm->find(type_name);
        ASSERT(iter != cm->end());
        return iter->second;
    }

private:
    panda_file::ItemContainer *cont_;
};

class LinkerDebugInfoScrapper : public panda_file::DebugInfoUpdater<LinkerDebugInfoScrapper> {
public:
    using Super = panda_file::DebugInfoUpdater<LinkerDebugInfoScrapper>;

    LinkerDebugInfoScrapper(const panda_file::File *file, CodePatcher *patcher) : Super(file), patcher_(patcher) {}

    panda_file::StringItem *GetOrCreateStringItem(const std::string &s)
    {
        patcher_->Add(s);
        return nullptr;
    }

    panda_file::BaseClassItem *GetType([[maybe_unused]] panda_file::File::EntityId type_id,
                                       [[maybe_unused]] const std::string &type_name)
    {
        return nullptr;
    }

private:
    CodePatcher *patcher_;
};
}  // namespace

void CodePatcher::Add(Change c)
{
    changes_.emplace_back(std::move(c));
}

void CodePatcher::Devour(CodePatcher &&p)
{
    const auto old_size = changes_.size();
    changes_.insert(changes_.end(), std::move_iterator(p.changes_.begin()), std::move_iterator(p.changes_.end()));
    const auto new_size = changes_.size();
    p.changes_.clear();

    ranges_.emplace_back(old_size, new_size);
}

void CodePatcher::AddRange(std::pair<size_t, size_t> range)
{
    ranges_.push_back(range);
}

void CodePatcher::ApplyLiteralArrayChange(LiteralArrayChange &lc, Context *ctx)
{
    auto id = ctx->literal_array_id_++;
    lc.it = ctx->GetContainer().GetOrCreateLiteralArrayItem(std::to_string(id));

    auto &old_its = lc.old->GetItems();
    auto new_its = std::vector<panda_file::LiteralItem>();
    new_its.reserve(old_its.size());

    for (const auto &i : old_its) {
        using LIT = panda_file::LiteralItem::Type;
        switch (i.GetType()) {
            case LIT::B1:
            case LIT::B2:
            case LIT::B4:
            case LIT::B8:
                new_its.emplace_back(i);
                break;
            case LIT::STRING: {
                auto str = ctx->StringFromOld(i.GetValue<panda_file::StringItem *>());
                new_its.emplace_back(str);
                break;
            }
            case LIT::METHOD: {
                auto meth = i.GetValue<panda_file::MethodItem *>();
                auto iter = ctx->known_items_.find(meth);
                ASSERT(iter != ctx->known_items_.end());
                ASSERT(iter->second->GetItemType() == panda_file::ItemTypes::METHOD_ITEM);
                new_its.emplace_back(static_cast<panda_file::MethodItem *>(iter->second));
                break;
            }
            default:
                UNREACHABLE();
        }
    }

    lc.it->AddItems(new_its);
}

void CodePatcher::ApplyDeps(Context *ctx)
{
    for (auto &v : changes_) {
        std::visit(
            [this, ctx](auto &a) {
                using T = std::remove_cv_t<std::remove_reference_t<decltype(a)>>;
                // IndexedChange, StringChange, LiteralArrayChange, std::string, std::function<void()>>
                if constexpr (std::is_same_v<T, IndexedChange>) {
                    a.mi->AddIndexDependency(a.it);
                } else if constexpr (std::is_same_v<T, StringChange>) {
                    a.it = ctx->GetContainer().GetOrCreateStringItem(a.str);
                } else if constexpr (std::is_same_v<T, LiteralArrayChange>) {
                    ApplyLiteralArrayChange(a, ctx);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    ctx->GetContainer().GetOrCreateStringItem(a);
                } else if constexpr (std::is_same_v<T, std::function<void()>>) {
                    // nothing
                } else {
                    UNREACHABLE();
                }
            },
            v);
    }
}

void CodePatcher::Patch(const std::pair<size_t, size_t> range)
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
                    auto off = a.it->GetOffset();
                    a.inst.UpdateId(BytecodeId(off));
                } else if constexpr (std::is_same_v<T, LiteralArrayChange>) {
                    auto off = a.it->GetIndex();
                    a.inst.UpdateId(BytecodeId(off));
                } else if constexpr (std::is_same_v<T, std::string>) {
                    // nothing
                } else if constexpr (std::is_same_v<T, std::function<void()>>) {
                    a();
                } else {
                    UNREACHABLE();
                }
            },
            v);
    }
}

void Context::ProcessCodeData(CodePatcher &p, CodeData *data)
{
    using Flags = panda::BytecodeInst<panda::BytecodeInstMode::FAST>::Flags;
    using EntityId = panda_file::File::EntityId;

    const auto my_id = EntityId(data->omi->GetOffset());
    auto *items = data->file_reader->GetItems();

    if (data->code != nullptr) {
        auto inst = BytecodeInstruction(data->code->data());
        size_t offset = 0;
        const auto limit = data->code->size();

        auto file_ptr = data->file_reader->GetFilePtr();

        using Resolver = EntityId (panda_file::File::*)(EntityId id, panda_file::File::Index idx) const;

        auto make_with_id = [&](Resolver resolve) {
            auto idx = inst.GetId().AsIndex();
            auto old_id = (file_ptr->*resolve)(my_id, idx);
            auto iter = items->find(old_id);
            ASSERT(iter != items->end());
            auto as_indexed = static_cast<panda_file::IndexedItem *>(iter->second);
            auto found = known_items_.find(as_indexed);
            ASSERT(found != known_items_.end());
            p.Add(CodePatcher::IndexedChange {inst, data->nmi, static_cast<panda_file::IndexedItem *>(found->second)});
        };

        while (offset < limit) {
            if (inst.HasFlag(Flags::TYPE_ID)) {
                make_with_id(&panda_file::File::ResolveClassIndex);
                // inst.UpdateId()
            } else if (inst.HasFlag(Flags::METHOD_ID)) {
                make_with_id(&panda_file::File::ResolveMethodIndex);
            } else if (inst.HasFlag(Flags::FIELD_ID)) {
                make_with_id(&panda_file::File::ResolveFieldIndex);
            } else if (inst.HasFlag(Flags::STRING_ID)) {
                BytecodeId b_id = inst.GetId();
                auto old_id = b_id.AsFileId();

                auto s_data = file_ptr->GetStringData(old_id);
                auto item_str = std::string(utf::Mutf8AsCString(s_data.data));
                p.Add(CodePatcher::StringChange {inst, std::move(item_str)});
            } else if (inst.HasFlag(Flags::LITERALARRAY_ID)) {
                BytecodeId b_id = inst.GetId();
                auto old_idx = b_id.AsRawValue();
                auto arrs = file_ptr->GetLiteralArrays();
                ASSERT(old_idx < arrs.size());
                auto off = arrs[old_idx];
                auto iter = items->find(EntityId(off));
                ASSERT(iter != items->end());
                ASSERT(iter->second->GetItemType() == panda_file::ItemTypes::LITERAL_ARRAY_ITEM);
                p.Add(
                    CodePatcher::LiteralArrayChange {inst, static_cast<panda_file::LiteralArrayItem *>(iter->second)});
            }

            offset += inst.GetSize();
            inst = inst.GetNext();
        }
    }

    if (auto *dbg = data->omi->GetDebugInfo(); data->patch_lnp && dbg != nullptr) {
        auto file = data->file_reader->GetFilePtr();
        auto scrapper = LinkerDebugInfoScrapper(file, &p);
        auto off = data->omi->GetDebugInfo()->GetOffset();
        ASSERT(off != 0);
        auto e_id = panda_file::File::EntityId(off);
        scrapper.Scrap(e_id);

        auto new_dbg = data->nmi->GetDebugInfo();
        p.Add([file, this, new_dbg, e_id]() {
            auto updater = LinkerDebugInfoUpdater(file, &cont_);
            updater.Emit(new_dbg, e_id);
        });
    }
}

}  // namespace panda::static_linker
