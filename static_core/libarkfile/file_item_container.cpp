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

#include "libarkfile/file_item_container.h"
#include <cstdint>
#include <type_traits>
#include "libarkbase/macros.h"
#include <libarkfile/include/file_format_version.h>
#include "libarkfile/pgo.h"

namespace ark::panda_file {

namespace {
bool HasNestedItems(BaseItem *item)
{
    // Keep in sync with BaseItem::Visit overrides. Today only ClassItem owns nested fields/methods.
    return item->GetItemType() == ItemTypes::CLASS_ITEM;
}

template <class Callback>
void VisitNestedItems(BaseItem *item, Callback &&cb)
{
    ASSERT(HasNestedItems(item));
    switch (item->GetItemType()) {
        case ItemTypes::CLASS_ITEM: {
            bool visitMore = true;
            auto visitor = [&cb, &visitMore](BaseItem *paramItem) {
                visitMore = cb(paramItem);
                return visitMore;
            };
            auto *classItem = static_cast<ClassItem *>(item);
            classItem->VisitFields(visitor);
            if (visitMore) {
                classItem->VisitMethods(visitor);
            }
            return;
        }
        default:
            break;
    }
    UNREACHABLE();
}

bool IsReusableRegionBoundary(BaseItem *item, BaseItem *containerEnd)
{
    return item == nullptr || item == containerEnd || item->NeedsEmit();
}

bool IsReusableRegionIndexItem(IndexedItem *item)
{
    if (item->NeedsEmit()) {
        return true;
    }
    if (item->GetIndexType() != IndexType::CLASS) {
        return false;
    }
    return static_cast<TypeItem *>(item)->GetType().IsPrimitive();
}
}  // namespace

class ItemDeduper {
public:
    template <class T>
    T *Deduplicate(T *item)
    {
        static_assert(std::is_base_of_v<BaseItem, T>);
        if (auto iter = alreadyDedupedItems_.find(item); iter != alreadyDedupedItems_.end()) {
            ASSERT(item->GetItemType() == iter->second->GetItemType());
            return static_cast<T *>(iter->second);
        }

        ItemData itemData(item);
        auto it = items_.find(itemData);
        if (it == items_.cend()) {
            items_.insert(itemData);
            return item;
        }

        auto resultItem = it->GetItem();
        ASSERT(item->GetItemType() == resultItem->GetItemType());
        auto result = static_cast<T *>(resultItem);
        if (item != result) {
            alreadyDedupedItems_.emplace(item, result);
            if constexpr (!std::is_same_v<T, LineNumberProgramItem>) {
                item->SetNeedsEmit(false);
            }
        }

        return result;
    }

    LineNumberProgramItem *Deduplicate(LineNumberProgramItem *item)
    {
        auto *deduplicated = Deduplicate<LineNumberProgramItem>(item);
        alreadyDedupedItems_.try_emplace(item, deduplicated);
        return deduplicated;
    }

    size_t GetUniqueCount() const
    {
        return items_.size();
    }

private:
    class ItemWriter : public Writer {
    public:
        ItemWriter(std::vector<uint8_t> *buf, size_t offset) : buf_(buf), offset_(offset) {}
        ~ItemWriter() override = default;

        NO_COPY_SEMANTIC(ItemWriter);
        NO_MOVE_SEMANTIC(ItemWriter);

        using Writer::WriteBytes;

        bool WriteByte(uint8_t byte) override
        {
            buf_->push_back(byte);
            ++offset_;
            return true;
        }

        bool WriteBytes(const std::vector<uint8_t> &bytes) override
        {
            return WriteBytes(bytes.data(), bytes.size());
        }

        bool WriteBytes(const uint8_t *bytes, size_t size) override
        {
            if (size == 0) {
                return true;
            }
            buf_->insert(buf_->end(), bytes, bytes + size);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            offset_ += size;
            return true;
        }

        size_t GetOffset() const override
        {
            return offset_;
        }

    private:
        std::vector<uint8_t> *buf_;
        size_t offset_;
    };

    class ItemData {
    public:
        explicit ItemData(BaseItem *item) : item_(item)
        {
            Initialize();
        }

        ~ItemData() = default;

        DEFAULT_COPY_SEMANTIC(ItemData);
        NO_MOVE_SEMANTIC(ItemData);

        BaseItem *GetItem() const
        {
            return item_;
        }

        uint32_t GetHash() const
        {
            ASSERT(IsInitialized());
            return hash_;
        }

        bool operator==(const ItemData &itemData) const noexcept
        {
            ASSERT(IsInitialized());
            return data_ == itemData.data_;
        }

    private:
        bool IsInitialized() const
        {
            return !data_.empty();
        }

        void Initialize()
        {
            ASSERT(item_->NeedsEmit());

            data_.reserve(item_->GetSize());
            ItemWriter writer(&data_, item_->GetOffset());
            [[maybe_unused]] auto res = item_->Write(&writer);
            ASSERT(res);
            ASSERT(data_.size() == item_->GetSize());

            hash_ = GetHash32(data_.data(), data_.size());
        }

        BaseItem *item_;
        uint32_t hash_ {0};
        std::vector<uint8_t> data_;
    };

    struct ItemHash {
        size_t operator()(const ItemData &itemData) const noexcept
        {
            return itemData.GetHash();
        }
    };

    std::unordered_set<ItemData, ItemHash> items_;
    std::unordered_map<BaseItem *, BaseItem *> alreadyDedupedItems_;
};

// NOTE(nsizov): make method for items deletion
template <class T, class C, class I, class P, class E, class... Args>
// CC-OFFNXT(G.FUN.01-CPP) solid logic
[[nodiscard]] static T *GetOrInsert(C &map, I &items, const P &pos, const E &key, bool isForeign, Args &&...args)
{
    auto it = map.find(key);
    if (it != map.cend()) {
        auto *item = it->second;
        if (item->IsForeign() == isForeign) {
            return static_cast<T *>(item);
        }

        return nullptr;
    }

    auto ii = items.insert(pos, std::make_unique<T>(std::forward<Args>(args)...));
    auto *item = static_cast<T *>(ii->get());

    [[maybe_unused]] auto res = map.insert({key, item});
    ASSERT(res.second);
    return item;
}

ItemContainer::ItemContainer()
{
    itemsEnd_ = items_.insert(items_.end(), std::make_unique<EndItem>());
    annotationItemsEnd_ = items_.insert(items_.end(), std::make_unique<EndItem>());
    codeItemsEnd_ = items_.insert(items_.end(), std::make_unique<EndItem>());
    debugItemsEnd_ = items_.insert(items_.end(), std::make_unique<EndItem>());
    end_ = debugItemsEnd_->get();
}

ClassItem *ItemContainer::GetOrCreateClassItem(const std::string &str)
{
    [[maybe_unused]] auto item = GetOrInsert<ClassItem>(classMap_, items_, itemsEnd_, str, false, str);
    ASSERT(item != nullptr);
    return item;
}

ForeignClassItem *ItemContainer::GetOrCreateForeignClassItem(const std::string &str)
{
    return GetOrInsert<ForeignClassItem>(classMap_, foreignItems_, foreignItems_.end(), str, true, str);
}

StringItem *ItemContainer::GetOrCreateStringItem(const std::string &str)
{
    auto it = classMap_.find(str);
    if (it != classMap_.cend()) {
        return it->second->GetNameItem();
    }

    [[maybe_unused]] auto item = GetOrInsert<StringItem>(stringMap_, items_, itemsEnd_, str, false, str);
    ASSERT(item != nullptr);
    return item;
}

void ItemContainer::CreateMetadataItem(std::vector<uint8_t> metadata)
{
    metadataItem_ = std::make_unique<MetadataItem>(std::move(metadata));
}

LiteralArrayItem *ItemContainer::GetOrCreateLiteralArrayItem(const std::string &id)
{
    [[maybe_unused]] auto item = GetOrInsert<LiteralArrayItem>(literalarrayMap_, items_, itemsEnd_, id, false);
    ASSERT(item != nullptr);
    return item;
}

ScalarValueItem *ItemContainer::GetOrCreateIntegerValueItem(uint32_t v)
{
    [[maybe_unused]] auto item = GetOrInsert<ScalarValueItem>(intValueMap_, items_, itemsEnd_, v, false, v);
    ASSERT(item != nullptr);
    return item;
}

ScalarValueItem *ItemContainer::GetOrCreateLongValueItem(uint64_t v)
{
    [[maybe_unused]] auto item = GetOrInsert<ScalarValueItem>(longValueMap_, items_, itemsEnd_, v, false, v);
    ASSERT(item != nullptr);
    return item;
}

ScalarValueItem *ItemContainer::GetOrCreateFloatValueItem(float v)
{
    [[maybe_unused]] auto item =
        GetOrInsert<ScalarValueItem>(floatValueMap_, items_, itemsEnd_, bit_cast<uint32_t>(v), false, v);
    ASSERT(item != nullptr);
    return item;
}

ScalarValueItem *ItemContainer::GetOrCreateDoubleValueItem(double v)
{
    [[maybe_unused]] auto item =
        GetOrInsert<ScalarValueItem>(doubleValueMap_, items_, itemsEnd_, bit_cast<uint64_t>(v), false, v);
    ASSERT(item != nullptr);
    return item;
}

ScalarValueItem *ItemContainer::GetOrCreateIdValueItem(BaseItem *v)
{
    [[maybe_unused]] auto item = GetOrInsert<ScalarValueItem>(idValueMap_, items_, itemsEnd_, v, false, v);
    ASSERT(item != nullptr);
    return item;
}

ProtoItem *ItemContainer::GetOrCreateProtoItem(TypeItem *retType, const std::vector<MethodParamItem> &params)
{
    ProtoKey key(retType, params);
    [[maybe_unused]] auto item = GetOrInsert<ProtoItem>(protoMap_, items_, itemsEnd_, key, false, retType, params);
    ASSERT(item != nullptr);
    return item;
}

PrimitiveTypeItem *ItemContainer::GetOrCreatePrimitiveTypeItem(Type type)
{
    return GetOrCreatePrimitiveTypeItem(type.GetId());
}

PrimitiveTypeItem *ItemContainer::GetOrCreatePrimitiveTypeItem(Type::TypeId type)
{
    [[maybe_unused]] auto item =
        GetOrInsert<PrimitiveTypeItem>(primitiveTypeMap_, items_, itemsEnd_, type, false, type);
    ASSERT(item != nullptr);
    return item;
}

LineNumberProgramItem *ItemContainer::CreateLineNumberProgramItem()
{
    auto it = items_.insert(debugItemsEnd_, std::make_unique<LineNumberProgramItem>());
    auto *item = static_cast<LineNumberProgramItem *>(it->get());
    lineNumberProgramItems_.push_back(item);
    [[maybe_unused]] auto res = lineNumberProgramIndexItem_.Add(item);
    ASSERT(res);
    return item;
}

void ItemContainer::IncRefLineNumberProgramItem(LineNumberProgramItem *it)
{
    lineNumberProgramIndexItem_.IncRefCount(it);
}

void ItemContainer::DeduplicateLineNumberProgram(DebugInfoItem *item, ItemDeduper *deduper)
{
    auto *lineNumberProgram = item->GetLineNumberProgram();
    auto *deduplicated = deduper->Deduplicate(lineNumberProgram);
    if (deduplicated != lineNumberProgram) {
        item->SetLineNumberProgram(deduplicated);
        ASSERT(deduplicated->GetRefCount() > 0);
        deduplicated->IncRefCount();
        lineNumberProgram->DecRefCount();
        if (lineNumberProgram->GetRefCount() == 0) {
            lineNumberProgram->SetNeedsEmit(false);
        }
    }
}

void ItemContainer::DeduplicateDebugInfo(MethodItem *method, ItemDeduper *debugInfoDeduper,
                                         ItemDeduper *lineNumberProgramDeduper)
{
    auto *debugItem = method->GetDebugInfo();
    if (debugItem == nullptr) {
        return;
    }

    DeduplicateLineNumberProgram(debugItem, lineNumberProgramDeduper);

    auto *deduplicated = debugInfoDeduper->Deduplicate(debugItem);
    if (deduplicated != debugItem) {
        method->SetDebugInfo(deduplicated);
        auto *lineNumberProgram = debugItem->GetLineNumberProgram();
        lineNumberProgram->DecRefCount();
        if (lineNumberProgram->GetRefCount() == 0) {
            lineNumberProgram->SetNeedsEmit(false);
        }
    }
}

static void DeduplicateCode(MethodItem *method, ItemDeduper *codeDeduper)
{
    auto *codeItem = method->GetCode();
    if (codeItem == nullptr) {
        return;
    }

    auto *deduplicated = codeDeduper->Deduplicate(codeItem);
    if (deduplicated != codeItem) {
        method->SetCode(deduplicated);
        deduplicated->AddMethod(method);  // we need it for Profile-Guided optimization
    }
}

void ItemContainer::DeduplicateCodeAndDebugInfo()
{
    ItemDeduper lineNumberProgramDeduper;
    ItemDeduper debugDeduper;
    ItemDeduper codeDeduper;

    lineNumberProgramIndexItem_.ClearItems();

    for (auto &p : classMap_) {
        auto *item = p.second;
        if (item->IsForeign()) {
            continue;
        }

        auto *classItem = static_cast<ClassItem *>(item);

        classItem->VisitMethods([this, &debugDeduper, &lineNumberProgramDeduper, &codeDeduper](BaseItem *paramItem) {
            auto *methodItem = static_cast<MethodItem *>(paramItem);
            DeduplicateDebugInfo(methodItem, &debugDeduper, &lineNumberProgramDeduper);
            DeduplicateCode(methodItem, &codeDeduper);
            return true;
        });
    }

    RebuildLineNumberProgramIndexItems();
}

static void DeduplicateAnnotationValue(AnnotationItem *annotationItem, ItemDeduper *deduper)
{
    auto *elems = annotationItem->GetElements();
    const auto &tags = annotationItem->GetTags();

    for (size_t i = 0; i < elems->size(); i++) {
        auto tag = tags[i];

        // try to dedupe only ArrayValueItems
        switch (tag.GetItem()) {
            case 'K':
            case 'L':
            case 'M':
            case 'N':
            case 'O':
            case 'P':
            case 'Q':
            case 'R':
            case 'S':
            case 'T':
            case 'U':
            case 'V':
            case 'W':
            case 'X':
            case 'Y':
            case 'Z':
            case '@':
                break;
            default:
                continue;
        }

        auto &elem = (*elems)[i];
        auto *value = elem.GetValue();
        auto *deduplicated = deduper->Deduplicate(value);
        if (deduplicated != value) {
            elem.SetValue(deduplicated);
        }
    }
}

static void DeduplicateAnnotations(std::vector<AnnotationItem *> *items, ItemDeduper *annotationDeduper,
                                   ItemDeduper *valueDeduper)
{
    for (auto &item : *items) {
        DeduplicateAnnotationValue(item, valueDeduper);
        auto *deduplicated = annotationDeduper->Deduplicate(item);
        if (deduplicated != item) {
            item = deduplicated;
        }
    }
}

void ItemContainer::DeduplicateAnnotations()
{
    ItemDeduper valueDeduper;
    ItemDeduper annotationDeduper;

    for (auto &p : classMap_) {
        auto *item = p.second;
        if (item->IsForeign()) {
            continue;
        }

        auto *classItem = static_cast<ClassItem *>(item);

        panda_file::DeduplicateAnnotations(classItem->GetRuntimeAnnotations(), &annotationDeduper, &valueDeduper);
        panda_file::DeduplicateAnnotations(classItem->GetAnnotations(), &annotationDeduper, &valueDeduper);
        panda_file::DeduplicateAnnotations(classItem->GetRuntimeTypeAnnotations(), &annotationDeduper, &valueDeduper);
        panda_file::DeduplicateAnnotations(classItem->GetTypeAnnotations(), &annotationDeduper, &valueDeduper);

        classItem->VisitMethods([&annotationDeduper, &valueDeduper](BaseItem *paramItem) {
            auto *methodItem = static_cast<MethodItem *>(paramItem);
            panda_file::DeduplicateAnnotations(methodItem->GetRuntimeAnnotations(), &annotationDeduper, &valueDeduper);
            panda_file::DeduplicateAnnotations(methodItem->GetAnnotations(), &annotationDeduper, &valueDeduper);
            panda_file::DeduplicateAnnotations(methodItem->GetRuntimeTypeAnnotations(), &annotationDeduper,
                                               &valueDeduper);
            panda_file::DeduplicateAnnotations(methodItem->GetTypeAnnotations(), &annotationDeduper, &valueDeduper);
            return true;
        });

        classItem->VisitFields([&annotationDeduper, &valueDeduper](BaseItem *paramItem) {
            auto *fieldItem = static_cast<FieldItem *>(paramItem);
            panda_file::DeduplicateAnnotations(fieldItem->GetRuntimeAnnotations(), &annotationDeduper, &valueDeduper);
            panda_file::DeduplicateAnnotations(fieldItem->GetAnnotations(), &annotationDeduper, &valueDeduper);
            panda_file::DeduplicateAnnotations(fieldItem->GetRuntimeTypeAnnotations(), &annotationDeduper,
                                               &valueDeduper);
            panda_file::DeduplicateAnnotations(fieldItem->GetTypeAnnotations(), &annotationDeduper, &valueDeduper);
            return true;
        });
    }
}

void ItemContainer::DeduplicateItems(bool computeLayout)
{
    if (computeLayout) {
        ComputeLayout();
    }
    DeduplicateCodeAndDebugInfo();
    DeduplicateAnnotations();
}

void ItemContainer::MarkLiteralarrayMap()
{
    for (auto &entry : literalarrayMap_) {
        entry.second->SetDependencyMark();
    }
}

void ItemContainer::CleanupArrayValueItems(ValueItem *value)
{
    auto arrayValue = value->GetAsArray();
    auto *vecItems = arrayValue->GetMutableItems();
    auto newEnd = std::partition(vecItems->begin(), vecItems->end(), [](const ScalarValueItem &valueItem) {
        if (valueItem.GetType() == ValueItem::Type::ID) {
            auto realitem = valueItem.GetIdItem();
            return realitem->GetDependencyMark();
        }
        return true;
    });
    vecItems->erase(newEnd, vecItems->end());
}

void ItemContainer::DeleteReferenceFromAnno(AnnotationItem *annoItem)
{
    auto &elements = *annoItem->GetElements();
    for (auto element = elements.begin(); element != elements.end();) {
        bool nodelete = true;
        auto *value = element->GetValue();
        if (value->GetType() == ValueItem::Type::ARRAY) {
            CleanupArrayValueItems(value);
        } else if (value->GetType() == ValueItem::Type::ID) {
            auto scalarValue = value->GetAsScalar();
            nodelete = scalarValue->GetIdItem()->GetDependencyMark();
        }
        if (nodelete) {
            ++element;
        } else {
            element = elements.erase(element);
        }
    }
}

uint32_t ItemContainer::DeleteItems()
{
    // ANNOTATION_ITEM reference other item by
    // AnnotationItem->std::vector<Elem>
    // Elem->ValueItem
    // ValueItem->BaseItem if ValueItem::Type == Type::ID
    // BaseItem should be remove from AnnotationItem before delete
    auto shouldRemoveAnnotationItem = [this](const std::unique_ptr<BaseItem> &item) -> bool {
        if (item->GetItemType() != ItemTypes::ANNOTATION_ITEM) {
            return false;
        }
        auto annoItem = static_cast<AnnotationItem *>(item.get());
        DeleteReferenceFromAnno(annoItem);
        return !item->GetDependencyMark() && item->NeedsEmit();
    };
    items_.remove_if(shouldRemoveAnnotationItem);
    std::unordered_set<LineNumberProgramItem *> removedLineNumberPrograms;
    items_.remove_if([this, &removedLineNumberPrograms](const std::unique_ptr<BaseItem> &item) {
        if (!item->GetDependencyMark()) {
            // lineNumberProgramIndexItem_ reference LineNumberProgramItem.
            // remove from lnp before item deleted if item not necessary.
            if (item->GetItemType() == ItemTypes::LINE_NUMBER_PROGRAM_ITEM && item->NeedsEmit()) {
                auto lnpitem = static_cast<LineNumberProgramItem *>(item.get());
                lineNumberProgramIndexItem_.Remove(lnpitem);
                removedLineNumberPrograms.insert(lnpitem);
            }
        }
        return !item->GetDependencyMark() && item->NeedsEmit();
    });
    if (!removedLineNumberPrograms.empty()) {
        auto newEnd = std::remove_if(
            lineNumberProgramItems_.begin(), lineNumberProgramItems_.end(),
            [&removedLineNumberPrograms](auto *item) { return removedLineNumberPrograms.count(item) != 0; });
        lineNumberProgramItems_.erase(newEnd, lineNumberProgramItems_.end());
    }
    return 0;
}

uint32_t ItemContainer::DeleteForeignItems()
{
    auto newEnd = std::partition(foreignItems_.begin(), foreignItems_.end(), [](const std::unique_ptr<BaseItem> &item) {
        return item->GetDependencyMark() || !item->NeedsEmit();
    });
    foreignItems_.erase(newEnd, foreignItems_.end());
    return 0;
}

void ItemContainer::FillExportMap()
{
    for (auto &it : classMap_) {
        if (!it.second->IsForeign() && it.first.rfind("ETSGLOBAL") != std::string::npos) {
            exportMap_.insert({it.first, it.second});
        }
    }
}

uint32_t ItemContainer::ComputeLayout()
{
    return ComputeLayout(true);
}

uint32_t ItemContainer::ComputeLayout(bool rebuildRegionSection)
{
    return ComputeLayout(rebuildRegionSection, true);
}

uint32_t ItemContainer::ComputeLayout(bool rebuildRegionSection, bool updateOrderIndexes)
{
    FillExportMap();

    uint32_t numClasses = classMap_.size();
    uint32_t numLiteralarrays = literalarrayMap_.size();
    uint32_t numExportClasses = exportMap_.size();
    uint32_t classIdxOffset = sizeof(File::Header);
    uint32_t exportDataOffset = classIdxOffset + numClasses * ID_SIZE;  // immediately after class table
    uint32_t metadataSize =
        File::METADATA_FLAG_SIZE + (MetadataItem::IsNullOrEmpty(metadataItem_) ? 0 : metadataItem_->Size() + ID_SIZE);
    uint32_t curOffset = exportDataOffset + (numExportClasses + numLiteralarrays) * ID_SIZE + metadataSize;

    if (updateOrderIndexes) {
        UpdateOrderIndexes();
    }
    UpdateLiteralIndexes();

    if (rebuildRegionSection) {
        RebuildRegionSection();
        regionSectionBuilt_ = true;
    } else {
        ASSERT(regionSectionBuilt_);
    }
    RebuildLineNumberProgramIndex();

    regionSectionItem_.SetOffset(curOffset);
    regionSectionItem_.ComputeLayout();
    curOffset += regionSectionItem_.GetSize();

    for (auto &item : foreignItems_) {
        curOffset = RoundUp(curOffset, item->Alignment());
        item->SetOffset(curOffset);
        item->ComputeLayout();
        curOffset += item->GetSize();
    }

    for (auto &item : items_) {
        if (!item->NeedsEmit()) {
            continue;
        }

        curOffset = RoundUp(curOffset, item->Alignment());
        item->SetOffset(curOffset);
        item->ComputeLayout();
        curOffset += item->GetSize();
    }

    // Line number program should be last because it's size is known only after deduplication
    curOffset = RoundUp(curOffset, lineNumberProgramIndexItem_.Alignment());
    lineNumberProgramIndexItem_.SetOffset(curOffset);
    lineNumberProgramIndexItem_.ComputeLayout();
    curOffset += lineNumberProgramIndexItem_.GetSize();

    end_->SetOffset(curOffset);

    return curOffset;
}

void ItemContainer::RebuildLineNumberProgramIndex()
{
    lineNumberProgramIndexItem_.Reset();
    lineNumberProgramIndexItem_.UpdateItems(nullptr, nullptr);
}

void ItemContainer::RebuildLineNumberProgramIndexItems()
{
    lineNumberProgramIndexItem_.ClearItems();
    for (auto *lineNumberProgram : lineNumberProgramItems_) {
        if (!lineNumberProgram->NeedsEmit()) {
            continue;
        }

        [[maybe_unused]] auto res = lineNumberProgramIndexItem_.Add(lineNumberProgram);
        ASSERT(res);
    }
}

void ItemContainer::RebuildRegionSection()
{
    regionSectionItem_.Reset();
    std::vector<IndexedItem *> addedItems;

    for (auto &item : foreignItems_) {
        ProcessIndexDependecies(item.get(), &addedItems);
    }

    for (auto &item : items_) {
        if (!item->NeedsEmit()) {
            continue;
        }

        ProcessIndexDependecies(item.get(), &addedItems);
    }

    if (!regionSectionItem_.IsEmpty()) {
        regionSectionItem_.GetCurrentHeader()->SetEnd(end_);
    }

    regionSectionItem_.UpdateItems();
}

bool ItemContainer::CanReuseRegionSectionForItem(BaseItem *item)
{
    auto validateDeps = [](BaseItem *regionItem) {
        for (auto *dep : regionItem->GetIndexDependencies()) {
            if (!IsReusableRegionIndexItem(dep) || !dep->HasIndex(regionItem)) {
                return false;
            }
        }
        return true;
    };
    if (!validateDeps(item)) {
        return false;
    }
    if (!HasNestedItems(item)) {
        return true;
    }

    bool canReuse = true;
    auto validateNestedDeps = [&validateDeps, &canReuse](BaseItem *paramItem) {
        if (canReuse) {
            canReuse = validateDeps(paramItem);
        }
        return canReuse;
    };
    VisitNestedItems(item, validateNestedDeps);
    return canReuse;
}

bool ItemContainer::CanReuseRegionSection(bool updateOrderIndexes, bool checkDependencies)
{
    if (!regionSectionBuilt_ || !regionSectionItem_.CanReuse(end_)) {
        return false;
    }

    if (!checkDependencies) {
        return true;
    }

    // Re-check with current emission state after final deduplication.
    if (updateOrderIndexes) {
        UpdateOrderIndexes();
    }

    for (auto &item : foreignItems_) {
        if (!CanReuseRegionSectionForItem(item.get())) {
            return false;
        }
    }

    for (auto &item : items_) {
        if (!item->NeedsEmit()) {
            continue;
        }
        if (!CanReuseRegionSectionForItem(item.get())) {
            return false;
        }
    }

    return true;
}

void ItemContainer::UpdateOrderIndexes()
{
    size_t idx = 0;

    for (auto &item : foreignItems_) {
        item->SetOrderIndex(idx++);
        if (HasNestedItems(item.get())) {
            VisitNestedItems(item.get(), [&idx](BaseItem *paramItem) {
                paramItem->SetOrderIndex(idx++);
                return true;
            });
        }
    }

    for (auto &item : items_) {
        if (!item->NeedsEmit()) {
            continue;
        }

        item->SetOrderIndex(idx++);
        if (HasNestedItems(item.get())) {
            VisitNestedItems(item.get(), [&idx](BaseItem *paramItem) {
                paramItem->SetOrderIndex(idx++);
                return true;
            });
        }
    }

    end_->SetOrderIndex(idx++);
}

void ItemContainer::UpdateLiteralIndexes()
{
    size_t idx = 0;

    for (auto &it : literalarrayMap_) {
        it.second->SetIndex(idx++);
    }
}

void ItemContainer::ReorderItems(ark::panda_file::pgo::ProfileOptimizer *profileOpt)
{
    profileOpt->ProfileGuidedRelayout(items_);
}

void ItemContainer::ProcessIndexDependecies(BaseItem *item, std::vector<IndexedItem *> *addedItems)
{
    if (regionSectionItem_.IsEmpty()) {
        regionSectionItem_.AddHeader();
        regionSectionItem_.GetCurrentHeader()->SetStart(item);
    }

    if (regionSectionItem_.GetCurrentHeader()->Add(item, addedItems)) {
        return;
    }

    regionSectionItem_.GetCurrentHeader()->SetEnd(item);
    regionSectionItem_.AddHeader();
    regionSectionItem_.GetCurrentHeader()->SetStart(item);

    if (!regionSectionItem_.GetCurrentHeader()->Add(item, addedItems)) {
        LOG(FATAL, PANDAFILE) << "Cannot add item index dependencies";
    }
}

bool ItemContainer::WriteHeaderIndexInfo(Writer *writer)
{
    if (!writer->Write<uint32_t>(classMap_.size())) {
        return false;
    }

    uint32_t headerSize = sizeof(File::Header);
    if (!writer->Write<uint32_t>(headerSize)) {
        return false;
    }

    uint32_t metadataSize =
        File::METADATA_FLAG_SIZE + (MetadataItem::IsNullOrEmpty(metadataItem_) ? 0 : metadataItem_->Size() + ID_SIZE);
    uint32_t exportDataSize = exportMap_.size() * ID_SIZE + metadataSize;
    if (!writer->Write<uint32_t>(exportDataSize)) {
        return false;
    }

    uint32_t classMapSize = classMap_.size() * ID_SIZE;
    uint32_t exportMapOffset = headerSize + classMapSize;
    if (!writer->Write<uint32_t>(exportMapOffset)) {
        return false;
    }

    if (!writer->Write<uint32_t>(lineNumberProgramIndexItem_.GetNumItems())) {
        return false;
    }

    if (!writer->Write<uint32_t>(lineNumberProgramIndexItem_.GetOffset())) {
        return false;
    }

    if (!writer->Write<uint32_t>(literalarrayMap_.size())) {
        return false;
    }

    uint32_t literalarrayIdxOffset = headerSize + classMapSize + exportDataSize;
    if (!writer->Write<uint32_t>(literalarrayIdxOffset)) {
        return false;
    }

    if (!writer->Write<uint32_t>(regionSectionItem_.GetNumHeaders())) {
        return false;
    }

    size_t indexSectionOff = literalarrayIdxOffset + literalarrayMap_.size() * ID_SIZE;
    return writer->Write<uint32_t>(indexSectionOff);
}

bool ItemContainer::WriteHeader(Writer *writer, ssize_t *checksumOffset, bool rebuildRegionSection,
                                bool updateOrderIndexes)
{
    uint32_t fileSize = ComputeLayout(rebuildRegionSection, updateOrderIndexes);
    writer->ReserveBufferCapacity(fileSize);

    const auto *magic = reinterpret_cast<const uint8_t *>(File::MAGIC.data());
    if (!writer->WriteBytes(magic, File::MAGIC.size())) {
        return false;
    }

    *checksumOffset = static_cast<ssize_t>(writer->GetOffset());
    uint32_t checksum = 0;
    if (!writer->Write(checksum)) {
        return false;
    }
    writer->CountChecksum(true);

    const auto &version = bytecodeVersion_.has_value() ? bytecodeVersion_.value() : VERSION;
    if (!writer->WriteBytes(version.data(), version.size())) {
        return false;
    }

    if (!writer->Write(fileSize)) {
        return false;
    }

    uint32_t foreignOffset = GetForeignOffset();
    if (!writer->Write(foreignOffset)) {
        return false;
    }

    uint32_t foreignSize = GetForeignSize();
    if (!writer->Write(foreignSize)) {
        return false;
    }

    if (!writer->Write<uint32_t>(static_cast<uint32_t>(isQuickened_))) {
        return false;
    }

    return WriteHeaderIndexInfo(writer);
}

bool ItemContainer::WriteExportData(Writer *writer)
{
    // Write export class idx and metadata
    auto isMetadataSkipped = MetadataItem::IsNullOrEmpty(
        metadataItem_);  // If metadata emitting is disabled (e.g. by compilation option), it'd be empty
    // NOLINTNEXTLINE(readability-implicit-bool-conversion)
    if (!writer->Write<uint32_t>(!isMetadataSkipped)) {
        return false;
    }
    if (!isMetadataSkipped && !writer->Write<uint32_t>(metadataItem_->Size())) {
        return false;
    }
    for (auto &entry : exportMap_) {
        if (!writer->Write(entry.second->GetOffset())) {
            return false;
        }
    }

    return isMetadataSkipped || metadataItem_->Write(writer);
}

bool ItemContainer::PrepareRegionSectionForWrite(RegionSectionMode regionSectionMode, bool *rebuildRegionSection,
                                                 bool *updateOrderIndexes)
{
    if (regionSectionMode == RegionSectionMode::REBUILD) {
        return true;
    }

    UpdateOrderIndexes();
    *updateOrderIndexes = false;
    const bool checkDependencies = regionSectionMode == RegionSectionMode::REUSE_REQUIRED;
    if (!CanReuseRegionSection(false, checkDependencies)) {
        return false;
    }

    *rebuildRegionSection = false;
    return true;
}

bool ItemContainer::WriteBody(Writer *writer)
{
    // Write class index table.
    for (auto &entry : classMap_) {
        if (!writer->Write(entry.second->GetOffset())) {
            return false;
        }
    }

    if (!WriteExportData(writer)) {
        return false;
    }

    // Write literal array index table.
    for (auto &entry : literalarrayMap_) {
        if (!writer->Write(entry.second->GetOffset())) {
            return false;
        }
    }

    if (!regionSectionItem_.Write(writer)) {
        return false;
    }

    for (auto &item : foreignItems_) {
        if (!writer->Align(item->Alignment()) || !item->Write(writer)) {
            return false;
        }
    }

    for (auto &item : items_) {
        if (item->NeedsEmit() && (!writer->Align(item->Alignment()) || !item->Write(writer))) {
            return false;
        }
    }

    return writer->Align(lineNumberProgramIndexItem_.Alignment()) && lineNumberProgramIndexItem_.Write(writer);
}

bool ItemContainer::Write(Writer *writer, bool deduplicateItems, bool computeLayout)
{
    return Write(writer, deduplicateItems, computeLayout, RegionSectionMode::REBUILD);
}

bool ItemContainer::Write(Writer *writer, bool deduplicateItems, bool computeLayout,
                          RegionSectionMode regionSectionMode)
{
    if (regionSectionMode != RegionSectionMode::REBUILD && computeLayout) {
        return false;
    }

    if (deduplicateItems) {
        DeduplicateItems(computeLayout);
    }

    ssize_t checksumOffset = -1;
    auto rebuildRegionSection = true;
    auto updateOrderIndexes = true;
    if (!PrepareRegionSectionForWrite(regionSectionMode, &rebuildRegionSection, &updateOrderIndexes)) {
        return false;
    }
    if (!WriteHeader(writer, &checksumOffset, rebuildRegionSection, updateOrderIndexes)) {
        return false;
    }
    ASSERT(checksumOffset != -1);

    if (!WriteBody(writer)) {
        return false;
    }

    writer->CountChecksum(false);
    if (!writer->WriteChecksum(checksumOffset)) {
        return false;
    }

    return writer->FinishWrite();
}

std::map<std::string, size_t> ItemContainer::GetStat()
{
    std::map<std::string, size_t> stat;

    DeduplicateItems();
    ComputeLayout();

    stat["header_item"] = sizeof(File::Header);
    stat["class_idx_item"] = classMap_.size() * ID_SIZE;
    stat["export_table_idx_item"] = exportMap_.size() * ID_SIZE;
    stat["line_number_program_idx_item"] = lineNumberProgramIndexItem_.GetNumItems() * ID_SIZE;
    stat["literalarray_idx"] = literalarrayMap_.size() * ID_SIZE;

    stat["region_section_item"] = regionSectionItem_.GetSize();
    stat["foreign_item"] = GetForeignSize();

    size_t numIns = 0;
    size_t codesize = 0;
    for (auto &item : items_) {
        if (!item->NeedsEmit()) {
            continue;
        }

        const auto &name = item->GetName();
        size_t size = item->GetSize();
        auto it = stat.find(name);
        if (it != stat.cend()) {
            stat[name] += size;
        } else if (size != 0) {
            stat[name] = size;
        }
        if (name == "code_item") {
            numIns += static_cast<CodeItem *>(item.get())->GetNumInstructions();
            codesize += static_cast<CodeItem *>(item.get())->GetCodeSize();
        }
    }
    stat["instructions_number"] = numIns;
    stat["codesize"] = codesize;

    return stat;
}

void ItemContainer::DumpItemsStat(std::ostream &os) const
{
    struct Stat {
        size_t n;
        size_t totalSize;
    };

    std::map<std::string, Stat> stat;

    auto collectStat = [&stat](auto &items) {
        for (auto &item : items) {
            if (!item->NeedsEmit()) {
                continue;
            }

            const auto &name = item->GetName();
            size_t size = item->GetSize();
            auto it = stat.find(name);
            if (it != stat.cend()) {
                stat[name].n += 1;
                stat[name].totalSize += size;
            } else if (size != 0) {
                stat[name] = {1, size};
            }
        }
    };

    collectStat(foreignItems_);
    collectStat(items_);

    for (auto &[name, elem] : stat) {
        os << name << ":" << std::endl;
        os << "    n          = " << elem.n << std::endl;
        os << "    total size = " << elem.totalSize << std::endl;
    }
}

size_t ItemContainer::GetForeignOffset() const
{
    if (foreignItems_.empty()) {
        return 0;
    }

    return foreignItems_.front()->GetOffset();
}

size_t ItemContainer::GetForeignSize() const
{
    if (foreignItems_.empty()) {
        return 0;
    }

    size_t begin = foreignItems_.front()->GetOffset();
    size_t end = foreignItems_.back()->GetOffset() + foreignItems_.back()->GetSize();

    return end - begin;
}

bool ItemContainer::RegionHeaderItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());
    ASSERT(start_ != nullptr);
    ASSERT(start_->GetOffset() != 0);
    ASSERT(end_ != nullptr);
    ASSERT(end_->GetOffset() != 0);

    if (!writer->Write<uint32_t>(start_->GetOffset())) {
        return false;
    }

    if (!writer->Write<uint32_t>(end_->GetOffset())) {
        return false;
    }

    for (auto *indexItem : indexes_) {
        if (!writer->Write<uint32_t>(indexItem->GetNumItems())) {
            return false;
        }

        ASSERT(indexItem->GetOffset() != 0);
        if (!writer->Write<uint32_t>(indexItem->GetOffset())) {
            return false;
        }
    }

    return true;
}

bool ItemContainer::RegionHeaderItem::AddIndexDependency(IndexedItem *item, std::vector<IndexedItem *> *addedItems)
{
    auto type = item->GetIndexType();
    ASSERT(type != IndexType::NONE);

    auto *indexItem = GetIndexByType(type);

    if (indexItem->Has(item)) {
        return true;
    }

    if (!indexItem->Add(item)) {
        return false;
    }

    addedItems->push_back(item);
    return true;
}

bool ItemContainer::RegionHeaderItem::AddIndexDependencies(const std::list<IndexedItem *> &items,
                                                           std::vector<IndexedItem *> *addedItems)
{
    for (auto *item : items) {
        if (!AddIndexDependency(item, addedItems)) {
            return false;
        }
    }
    return true;
}

bool ItemContainer::RegionHeaderItem::Add(BaseItem *item, std::vector<IndexedItem *> *addedItems)
{
    addedItems->clear();
    addedItems->reserve(item->GetIndexDependencies().size());

    if (!AddIndexDependencies(item->GetIndexDependencies(), addedItems)) {
        Remove(*addedItems);
        return false;
    }

    if (!HasNestedItems(item)) {
        return true;
    }

    bool added = true;
    auto addNestedDeps = [this, &added, addedItems](BaseItem *paramItem) {
        if (added) {
            added = AddIndexDependencies(paramItem->GetIndexDependencies(), addedItems);
        }
        return true;
    };
    VisitNestedItems(item, addNestedDeps);
    if (!added) {
        Remove(*addedItems);
        return false;
    }

    return true;
}

void ItemContainer::RegionHeaderItem::Remove(const std::vector<IndexedItem *> &items)
{
    for (auto *item : items) {
        auto type = item->GetIndexType();
        ASSERT(type != IndexType::NONE);

        auto *indexItem = GetIndexByType(type);
        indexItem->Remove(item);
    }
}

bool ItemContainer::IndexItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());

    const auto writeItem = [writer](IndexedItem *item) { return writer->Write<uint32_t>(item->GetOffset()); };

    if (useVectorIndex_) {
        for (auto *item : vectorIndex_) {
            if (!writeItem(item)) {
                return false;
            }
        }
        return true;
    }

    for (auto *item : index_) {
        if (!writeItem(item)) {
            return false;
        }
    }

    return true;
}

ItemTypes ItemContainer::IndexItem::GetItemType() const
{
    switch (type_) {
        case IndexType::CLASS:
            return ItemTypes::CLASS_INDEX_ITEM;
        case IndexType::METHOD:
            return ItemTypes::METHOD_INDEX_ITEM;
        case IndexType::FIELD:
            return ItemTypes::FIELD_INDEX_ITEM;
        case IndexType::PROTO:
            return ItemTypes::PROTO_INDEX_ITEM;
        case IndexType::LINE_NUMBER_PROG:
            return ItemTypes::LINE_NUMBER_PROGRAM_INDEX_ITEM;
        default:
            break;
    }

    UNREACHABLE();
}

bool ItemContainer::IndexItem::Add(IndexedItem *item)
{
    auto size = GetNumItems();
    ASSERT(size <= maxIndex_);

    if (size == maxIndex_) {
        return false;
    }

    if (useVectorIndex_) {
        // RegionHeaderItem checks membership before calling Add; keep this path append-only.
        ASSERT(!Has(item));
        vectorIndex_.push_back(item);
        AddToIndexSet(item->GetItemAllocId());
        return true;
    }

    auto res = index_.insert(item);
    ASSERT(res.second);
    if (!res.second) {
        return false;
    }
    AddToIndexSet(item->GetItemAllocId());
    return res.second;
}

bool ItemContainer::IndexItem::CanReuse() const
{
    if (useVectorIndex_) {
        return std::all_of(vectorIndex_.cbegin(), vectorIndex_.cend(),
                           [](auto *item) { return IsReusableRegionIndexItem(item); });
    }
    return std::all_of(index_.cbegin(), index_.cend(), [](auto *item) { return IsReusableRegionIndexItem(item); });
}

bool ItemContainer::RegionHeaderItem::CanReuse(BaseItem *containerEnd) const
{
    if (!IsReusableRegionBoundary(start_, containerEnd) || !IsReusableRegionBoundary(end_, containerEnd)) {
        return false;
    }
    return std::all_of(indexes_.cbegin(), indexes_.cend(), [](const auto *index) { return index->CanReuse(); });
}

bool ItemContainer::RegionSectionItem::CanReuse(BaseItem *containerEnd) const
{
    return std::all_of(headers_.cbegin(), headers_.cend(),
                       [containerEnd](const auto &header) { return header.CanReuse(containerEnd); });
}

void ItemContainer::RegionSectionItem::AddHeader()
{
    std::vector<IndexItem *> indexItems;
    for (size_t i = 0; i < INDEX_COUNT_16; i++) {
        auto type = static_cast<IndexType>(i);
        indexes_.emplace_back(type, MAX_INDEX_16, true);
        indexItems.push_back(&indexes_.back());
    }
    headers_.emplace_back(indexItems);
}

size_t ItemContainer::RegionSectionItem::CalculateSize() const
{
    size_t size = headers_.size() * sizeof(File::RegionHeader);
    for (auto &indexItem : indexes_) {
        size += indexItem.GetSize();
    }
    return size;
}

void ItemContainer::RegionSectionItem::ComputeLayout()
{
    size_t offset = GetOffset();

    for (auto &header : headers_) {
        header.SetOffset(offset);
        header.ComputeLayout();
        offset += header.GetSize();
    }

    for (auto &index : indexes_) {
        index.SetOffset(offset);
        index.ComputeLayout();
        offset += index.GetSize();
    }
}

bool ItemContainer::RegionSectionItem::Write(Writer *writer)
{
    ASSERT(GetOffset() == writer->GetOffset());

    for (auto &header : headers_) {
        if (!header.Write(writer)) {
            return false;
        }
    }

    for (auto &index : indexes_) {
        if (!index.Write(writer)) {
            return false;
        }
    }

    return true;
}

ItemContainer::ProtoKey::ProtoKey(TypeItem *retType, const std::vector<MethodParamItem> &params)
{
    Add(retType);
    for (const auto &param : params) {
        Add(param.GetType());
    }
    size_t shortyHash = std::hash<std::string>()(shorty_);
    size_t retTypeHash = std::hash<TypeItem *>()(retType);
    // combine hashes of shorty and ref_types
    hash_ = ark::MergeHashes(shortyHash, retTypeHash);
    // combine hashes of all param types
    for (const auto &item : params) {
        size_t paramTypeHash = std::hash<TypeItem *>()(item.GetType());
        hash_ = ark::MergeHashes(hash_, paramTypeHash);
    }
}

void ItemContainer::ProtoKey::Add(TypeItem *item)
{
    auto type = item->GetType();
    shorty_.append(Type::GetSignatureByTypeId(type));
    if (type.IsReference()) {
        refTypes_.push_back(item);
    }
}

}  // namespace ark::panda_file
