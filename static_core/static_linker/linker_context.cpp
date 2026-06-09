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

#include <algorithm>
#include <atomic>
#include <thread>
#include <type_traits>
#include <variant>

#include "libarkfile/file_items.h"
#include "libarkfile/file_reader.h"
#include "libarkfile/proto_data_accessor-inl.h"
#include "libarkbase/utils/utf.h"

#include "linker_context.h"

namespace {
template <typename T>
auto DerefPtrRef(T &&v) -> std::conditional_t<std::is_pointer_v<T>, std::remove_pointer_t<T>, T> &
{
    static_assert(std::is_pointer_v<T> || std::is_reference_v<T>);
    if constexpr (std::is_pointer_v<T>) {
        return *v;
    } else {
        return v;
    }
}
}  // namespace

namespace ark::static_linker {

void Context::AddItemToKnown(panda_file::BaseItem *item, const std::map<std::string, panda_file::BaseClassItem *> &cm,
                             const panda_file::FileReader &reader)
{
    auto *fc = static_cast<panda_file::ForeignClassItem *>(item);
    auto name = GetStr(fc->GetNameItem());
    if (auto iter = cm.find(name); iter != cm.end()) {
        knownItems_[fc] = iter->second;
    } else {
        auto clz = cont_.GetOrCreateForeignClassItem(name);
        cameFrom_.emplace_back(clz, &reader);
        knownItems_[fc] = clz;
    }
}

void Context::MergeItem(panda_file::BaseItem *item, const panda_file::FileReader &reader)
{
    switch (item->GetItemType()) {
        case panda_file::ItemTypes::FOREIGN_METHOD_ITEM:
            MergeForeignMethod(&reader, static_cast<panda_file::ForeignMethodItem *>(item));
            break;
        case panda_file::ItemTypes::FOREIGN_FIELD_ITEM:
            MergeForeignField(&reader, static_cast<panda_file::ForeignFieldItem *>(item));
            break;
        default:;
    }
}

void Context::Merge()
{
    // Dependency scope widens through the file: types can reference types, methods can reference types, and code can
    // reference any indexed item.  Build old item -> new item mappings before users of those mappings are merged.
    // Merge has to preserve this order:
    //   1. forward-declare regular classes,
    //   2. map referenced foreign classes,
    //   3. fill regular class bodies,
    //   4. resolve foreign members.
    // Later stages rely on old item -> new item mappings built here.
    auto &cm = *cont_.GetClassMap();
    auto buckets = CollectMergeItemBuckets();
    knownItems_.reserve(buckets.totalItems);
    cameFrom_.reserve(buckets.cameFromItems);
    codeDatas_.reserve(buckets.methodItems);
    protoCache_.reserve(buckets.methodItems);

    AddRegularClasses();

    for (const auto &[item, reader] : buckets.foreignClasses) {
        AddItemToKnown(item, cm, *reader);
    }

    FillRegularClasses();

    for (const auto &[item, reader] : buckets.foreignMembers) {
        MergeItem(item, *reader);
    }

    for (const auto &f : deferredFailedAnnotations_) {
        f();
    }
    deferredFailedAnnotations_.clear();
}

Context::MergeItemBuckets Context::CollectMergeItemBuckets()
{
    auto buckets = MergeItemBuckets {};
    for (auto &reader : readers_) {
        for (const auto &[o, i] : *reader.GetItems()) {
            // Keep old entity offsets available for bytecode/debug id resolution, and collect the work lists used by
            // the ordered merge below.
            i->SetOffset(o.GetOffset());
            buckets.totalItems++;
            switch (i->GetItemType()) {
                case panda_file::ItemTypes::CLASS_ITEM:
                    buckets.cameFromItems++;
                    break;
                case panda_file::ItemTypes::FIELD_ITEM:
                    buckets.cameFromItems++;
                    break;
                case panda_file::ItemTypes::METHOD_ITEM:
                    buckets.methodItems++;
                    buckets.cameFromItems++;
                    break;
                case panda_file::ItemTypes::FOREIGN_CLASS_ITEM:
                    buckets.cameFromItems++;
                    buckets.foreignClasses.emplace_back(i, &reader);
                    break;
                case panda_file::ItemTypes::FOREIGN_METHOD_ITEM:
                case panda_file::ItemTypes::FOREIGN_FIELD_ITEM:
                    buckets.cameFromItems++;
                    buckets.foreignMembers.emplace_back(i, &reader);
                    break;
                default:
                    break;
            }
        }
    }
    return buckets;
}

panda_file::ClassItem *Context::GetOrCreateRegularClass(const std::string &name, const panda_file::FileReader *reader)
{
    auto &cm = *cont_.GetClassMap();
    if (auto iter = cm.find(name); iter != cm.end()) {
        if (conf_.partial.count(name) == 0) {
            Error("Class redefinition", {ErrorDetail("original", iter->second)}, reader);
        }
        ASSERT(iter->second->GetItemType() == panda_file::ItemTypes::CLASS_ITEM);
        return static_cast<panda_file::ClassItem *>(iter->second);
    }
    return cont_.GetOrCreateClassItem(name);
}

void Context::AddRegularClasses()
{
    for (auto &reader : readers_) {
        auto *ic = reader.GetContainerPtr();
        auto &classes = *ic->GetClassMap();

        for (const auto &[name, i] : classes) {
            if (i->IsForeign()) {
                continue;
            }
            ASSERT(name == GetStr(i->GetNameItem()));

            auto *clz = GetOrCreateRegularClass(name, &reader);
            knownItems_[i] = clz;
            cameFrom_.emplace_back(clz, &reader);
        }
    }
}

void Context::FillRegularClasses()
{
    for (auto &reader : readers_) {
        auto *ic = reader.GetContainerPtr();
        auto &classes = *ic->GetClassMap();

        for (const auto &[name, i] : classes) {
            if (i->IsForeign()) {
                continue;
            }
            auto found = knownItems_.find(i);
            ASSERT(found != knownItems_.end());
            auto ni = static_cast<panda_file::ClassItem *>(found->second);
            auto oi = static_cast<panda_file::ClassItem *>(i);
            MergeClass(&reader, ni, oi);
        }
    }
}

void Context::MergeClass(const panda_file::FileReader *reader, panda_file::ClassItem *ni, panda_file::ClassItem *oi)
{
    ni->SetAccessFlags(oi->GetAccessFlags());
    ni->SetPGORank(oi->GetPGORank());
    ni->SetSourceLang(oi->GetSourceLang());
    ni->SetSourceFile(StringFromOld(oi->GetSourceFile()));

    ni->SetSuperClass(ClassFromOld(oi->GetSuperClass()));

    for (auto iface : oi->GetInterfaces()) {
        ni->AddInterface(ClassFromOld(iface));
    }

    TransferAnnotations(reader, ni, oi);

    oi->VisitFields([this, ni, reader](panda_file::BaseItem *mi) -> bool {
        ASSERT(mi->GetItemType() == panda_file::ItemTypes::FIELD_ITEM);
        MergeField(reader, ni, reinterpret_cast<panda_file::FieldItem *>(mi));
        return true;
    });

    oi->VisitMethods([this, ni, reader](panda_file::BaseItem *mi) -> bool {
        ASSERT(mi->GetItemType() == panda_file::ItemTypes::METHOD_ITEM);
        MergeMethod(reader, ni, reinterpret_cast<panda_file::MethodItem *>(mi));
        return true;
    });
}

void Context::MergeField(const panda_file::FileReader *reader, panda_file::ClassItem *clz, panda_file::FieldItem *oi)
{
    auto ni = clz->AddField(StringFromOld(oi->GetNameItem()), TypeFromOld(oi->GetTypeItem()), oi->GetAccessFlags());
    knownItems_[oi] = ni;
    cameFrom_.emplace_back(ni, reader);

    if (oi->GetValue() != nullptr) {
        auto newVal = ValueFromOld(oi->GetValue());
        if (std::holds_alternative<ErrorDetail>(newVal)) {
            Error("can not parse field value",
                  {std::get<ErrorDetail>(newVal), ErrorDetail {"field", oi}, {"value", oi->GetValue()}});
        } else {
            ni->SetValue(std::get<panda_file::ValueItem *>(newVal));
        }
    }

    TransferAnnotations(reader, ni, oi);
}

void Context::MergeMethod(const panda_file::FileReader *reader, panda_file::ClassItem *clz, panda_file::MethodItem *oi)
{
    const auto &protoData = GetProto(oi->GetProto());
    auto &oldParams = oi->GetParams();
    auto *ni =
        clz->AddMethod(StringFromOld(oi->GetNameItem()), protoData.proto, oi->GetAccessFlags(), protoData.params);
    knownItems_[oi] = ni;
    cameFrom_.emplace_back(ni, reader);
    ni->SetProfileSize(oi->GetProfileSize());

    for (size_t i = 0; i < oldParams.size(); i++) {
        TransferAnnotations(reader, &ni->GetParams()[i], &oldParams[i]);
    }

    TransferAnnotations(reader, ni, oi);

    if (ni->HasRuntimeParamAnnotations()) {
        cont_.CreateItem<panda_file::ParamAnnotationsItem>(ni, true);
    }

    if (ni->HasParamAnnotations()) {
        cont_.CreateItem<panda_file::ParamAnnotationsItem>(ni, false);
    }

    std::vector<uint8_t> *instructions = nullptr;

    auto [shouldSave, patchLnp] = UpdateDebugInfo(ni, oi);

    if (auto oci = oi->GetCode(); oci != nullptr) {
        shouldSave = true;
        result_.stats.codeCount++;

        auto nci = cont_.CreateItem<panda_file::CodeItem>();
        ni->SetCode(nci);
        nci->SetPGORank(oci->GetPGORank());
        nci->SetNumArgs(oci->GetNumArgs());
        nci->SetNumVregs(oci->GetNumVregs());
        *nci->GetInstructions() = *oci->GetInstructions();
        instructions = nci->GetInstructions();
        nci->SetNumInstructions(oci->GetNumInstructions());

        CreateTryBlocks(ni, nci, oi, oci);
    }

    if (shouldSave) {
        codeDatas_.push_back(CodeData {instructions, oi, ni, reader, patchLnp});
    }
}

std::pair<bool, bool> Context::UpdateDebugInfo(panda_file::MethodItem *ni, panda_file::MethodItem *oi)
{
    auto shouldSave = false;
    auto patchLnp = false;

    if (auto odbg = oi->GetDebugInfo(); !conf_.stripDebugInfo && odbg != nullptr) {
        panda_file::LineNumberProgramItem *lnpItem {};

        auto oldProgram = odbg->GetLineNumberProgram();
        if (auto old = knownItems_.find(oldProgram); old != knownItems_.end()) {
            lnpItem = static_cast<panda_file::LineNumberProgramItem *>(old->second);
            cont_.IncRefLineNumberProgramItem(lnpItem);
        } else {
            lnpItem = cont_.CreateLineNumberProgramItem();
            knownItems_.emplace(oldProgram, lnpItem);

            shouldSave = true;
            result_.stats.debugCount++;
            patchLnp = true;
        }

        auto *ndbg = cont_.CreateItem<panda_file::DebugInfoItem>(lnpItem);
        ndbg->SetLineNumber(odbg->GetLineNumber());
        ni->SetDebugInfo(ndbg);
        auto *parameters = odbg->GetParameters();
        ndbg->ReserveParameters(parameters->size());
        for (auto *s : *parameters) {
            ndbg->AddParameter(StringFromOld(s));
        }
    }

    return {shouldSave, patchLnp};
}

void Context::CreateTryBlocks(panda_file::MethodItem *ni, panda_file::CodeItem *nci,
                              [[maybe_unused]] panda_file::MethodItem *oi, panda_file::CodeItem *oci)
{
    for (const auto &otb : oci->GetTryBlocks()) {
        auto ncbs = std::vector<panda_file::CodeItem::CatchBlock>();
        const auto &ocbs = otb.GetCatchBlocks();
        ncbs.reserve(ocbs.size());
        for (const auto &ocb : ocbs) {
            ASSERT(ocb.GetMethod() == oi);
            auto excKlass = ClassFromOld(ocb.GetType());
            if (excKlass != nullptr) {
                ni->AddIndexDependency(excKlass);
            }
            ncbs.emplace_back(ni, excKlass, ocb.GetHandlerPc(), ocb.GetCodeSize());
        }
        auto ntb = panda_file::CodeItem::TryBlock(otb.GetStartPc(), otb.GetLength(), ncbs);
        nci->AddTryBlock(ntb);
    }
}

bool Context::IsSameType(ark::panda_file::TypeItem *nevv, ark::panda_file::TypeItem *old)
{
    if (nevv->GetType().IsPrimitive()) {
        if (!old->GetType().IsPrimitive()) {
            return false;
        }
        return nevv->GetType() == old->GetType();
    }

    auto iter = knownItems_.find(old);
    return iter != knownItems_.end() && iter->second == nevv;
}

Context::MethodSearchResult Context::TryFindMethod(panda_file::BaseClassItem *klass, panda_file::StringItem *name,
                                                   panda_file::ProtoItem *proto, std::vector<ErrorDetail> *relatedItems)
{
    if (klass == nullptr) {
        return false;
    }
    if (klass->GetItemType() == panda_file::ItemTypes::FOREIGN_CLASS_ITEM) {
        return true;
    }
    ASSERT(klass->GetItemType() == panda_file::ItemTypes::CLASS_ITEM);
    auto kls = static_cast<panda_file::ClassItem *>(klass);

    auto [new_meth_beg, new_meth_end] = kls->FindMethod(name);

    for (auto beg = new_meth_beg; beg != new_meth_end; ++beg) {
        auto nm = beg->get();

        relatedItems->emplace_back("candidate", nm);

        if (nm->GetProto() == proto) {
            return nm;
        }
    }

    panda_file::BaseClassItem *searchIn = kls->GetSuperClass();
    if (klass == searchIn) {
        LOG(FATAL, STATIC_LINKER) << "TryFindMethod Error: Current class same as SuperClass";
    }
    auto hasSearchResult = [](const MethodSearchResult &res) {
        if (std::holds_alternative<bool>(res)) {
            return std::get<bool>(res);
        }
        return std::holds_alternative<panda_file::MethodItem *>(res);
    };

    auto res = TryFindMethod(searchIn, name, proto, relatedItems);
    if (hasSearchResult(res)) {
        return res;
    }
    for (auto *iface : kls->GetInterfaces()) {
        res = TryFindMethod(iface, name, proto, relatedItems);
        if (hasSearchResult(res)) {
            return res;
        }
    }
    return false;
}

bool Context::IsSameProto(panda_file::ProtoItem *op1, panda_file::ProtoItem *op2)
{
    if (op1->GetRefTypes().size() != op2->GetRefTypes().size()) {
        return false;
    }

    if (op1->GetShorty() != op2->GetShorty()) {
        return false;
    }

    for (size_t i = 0; i < op1->GetRefTypes().size(); i++) {
        if (!IsSameType(op2->GetRefTypes()[i], op1->GetRefTypes()[i])) {
            return false;
        }
    }

    return true;
}

void Context::MergeForeignMethod(const panda_file::FileReader *reader, panda_file::ForeignMethodItem *fm)
{
    ASSERT(knownItems_.find(fm) == knownItems_.end());
    auto clzIt = knownItems_.find(fm->GetClassItem());
    ASSERT(clzIt != knownItems_.end());
    auto *clz = static_cast<panda_file::BaseClassItem *>(clzIt->second);
    auto *name = StringFromOld(fm->GetNameItem());
    auto *proto = GetProto(fm->GetProto()).proto;
    auto resolutionKey = MethodResolutionKey {clz, name, proto};
    if (auto cached = resolvedMethods_.find(resolutionKey); cached != resolvedMethods_.end()) {
        knownItems_[fm] = cached->second;
        return;
    }

    std::vector<ark::static_linker::Context::ErrorDetail> details = {{"method", fm}};
    auto res = TryFindMethod(clz, name, proto, &details);
    if (std::holds_alternative<panda_file::MethodItem *>(res)) {
        ASSERT(std::holds_alternative<panda_file::MethodItem *>(res));
        auto *meth = std::get<panda_file::MethodItem *>(res);
        if (meth->GetClassItem() != clz) {
            LOG(DEBUG, STATIC_LINKER) << "Resolved method\n"
                                      << ErrorToString({"old method", fm}, 1) << '\n'
                                      << ErrorToString({"new method", meth}, 1);
        }
        resolvedMethods_.emplace(resolutionKey, meth);
        knownItems_[fm] = meth;
        return;
    }

    ASSERT(std::holds_alternative<bool>(res));
    if (std::get<bool>(res) || conf_.remainsPartial.count(GetStr(clz->GetNameItem())) != 0) {
        MergeForeignMethodCreate(reader, clz, fm, name, proto);
    } else {
        Error("Unresolved method", details, reader);
    }
}

void Context::MergeForeignMethodCreate(const panda_file::FileReader *reader, panda_file::BaseClassItem *clz,
                                       panda_file::ForeignMethodItem *fm, panda_file::StringItem *name,
                                       panda_file::ProtoItem *proto)
{
    auto *fc = static_cast<panda_file::BaseClassItem *>(clz);
    auto access = fm->GetAccessFlags();
    auto [iter, was_inserted] = foreignMethods_.emplace(ForeignMethodKey {fc, name, proto, access}, nullptr);
    if (was_inserted) {
        iter->second = cont_.CreateItem<panda_file::ForeignMethodItem>(fc, name, proto, access);
    } else {
        result_.stats.deduplicatedForeigners++;
    }
    auto *nfm = iter->second;
    cameFrom_.emplace_back(nfm, reader);
    knownItems_[fm] = nfm;
}

Context::FieldSearchResult Context::TryFindField(panda_file::BaseClassItem *klass, const std::string &name,
                                                 panda_file::TypeItem *expectedType,
                                                 std::vector<panda_file::FieldItem *> *badCandidates)
{
    if (klass == nullptr) {
        return std::monostate {};
    }
    if (klass->GetItemType() == panda_file::ItemTypes::FOREIGN_CLASS_ITEM) {
        return static_cast<panda_file::ForeignClassItem *>(klass);
    }
    auto *kls = static_cast<panda_file::ClassItem *>(klass);
    panda_file::FieldItem *newFld = nullptr;
    kls->VisitFields([&name, expectedType, badCandidates, &newFld](panda_file::BaseItem *bi) {
        ASSERT(bi->GetItemType() == panda_file::ItemTypes::FIELD_ITEM);
        auto *fld = static_cast<panda_file::FieldItem *>(bi);
        if (fld->GetNameItem()->GetData() != name) {
            return true;
        }
        if (fld->GetTypeItem() != expectedType) {
            if (badCandidates != nullptr) {
                badCandidates->push_back(fld);
            }
            return true;
        }
        newFld = fld;
        return false;
    });
    if (newFld != nullptr) {
        return newFld;
    }

    if (klass == kls->GetSuperClass()) {
        LOG(FATAL, STATIC_LINKER) << "TryFindField Error: Current class same as SuperClass";
    }
    return TryFindField(kls->GetSuperClass(), name, expectedType, badCandidates);
}

void Context::HandleCandidates(const panda_file::FileReader *reader,
                               const std::vector<panda_file::FieldItem *> &candidates, panda_file::ForeignFieldItem *ff)
{
    auto details = std::vector<ErrorDetail> {{"field", ff}};
    for (const auto &c : candidates) {
        details.emplace_back("candidate", c);
    }
    Error("Unresolved field", details, reader);
}

void Context::MergeForeignField(const panda_file::FileReader *reader, panda_file::ForeignFieldItem *ff)
{
    ASSERT(knownItems_.find(ff) == knownItems_.end());
    auto clzIt = knownItems_.find(ff->GetClassItem());
    ASSERT(clzIt != knownItems_.end());

    auto *clz = static_cast<panda_file::BaseClassItem *>(clzIt->second);
    auto *name = StringFromOld(ff->GetNameItem());
    auto *type = TypeFromOld(ff->GetTypeItem());
    if (clz->GetItemType() == panda_file::ItemTypes::FOREIGN_CLASS_ITEM) {
        MergeForeignFieldCreate(reader, clz, ff, name, type);
        return;
    }

    ASSERT(clz->GetItemType() == panda_file::ItemTypes::CLASS_ITEM);
    auto *rclz = static_cast<panda_file::ClassItem *>(clz);
    auto resolutionKey = ForeignFieldKey {rclz, name, type};
    auto resolutionData = FieldResolutionData {reader, ff, resolutionKey, name, type};
    if (auto cached = resolvedFields_.find(resolutionKey); cached != resolvedFields_.end()) {
        ApplyResolvedField(resolutionData, cached->second, false);
        return;
    }

    std::vector<panda_file::FieldItem *> candidates;
    auto res = TryFindField(rclz, name->GetData(), type, &candidates);
    if (std::holds_alternative<std::monostate>(res)) {
        if (conf_.remainsPartial.count(GetStr(clz->GetNameItem())) != 0) {
            MergeForeignFieldCreate(reader, clz, ff, name, type);
            return;
        }
        HandleCandidates(reader, candidates, ff);
        return;
    }
    if (auto field = std::get_if<panda_file::FieldItem *>(&res); field != nullptr) {
        ApplyResolvedField(resolutionData, ResolvedField {*field}, true);
        return;
    }

    auto foreignClass = std::get<panda_file::ForeignClassItem *>(res);
    LOG(DEBUG, STATIC_LINKER) << "Propagating foreign field to base\n"
                              << ErrorToString({"field", ff}, 1) << '\n'
                              << ErrorToString({"new base", foreignClass}, 1);
    ApplyResolvedField(resolutionData, ResolvedField {foreignClass}, true);
}

void Context::ApplyResolvedField(const FieldResolutionData &data, ResolvedField resolvedField, bool cacheResult)
{
    if (cacheResult) {
        resolvedFields_.emplace(data.key, resolvedField);
    }
    if (auto field = std::get_if<panda_file::FieldItem *>(&resolvedField); field != nullptr) {
        knownItems_[data.oldField] = *field;
        return;
    }
    auto foreignClass = std::get<panda_file::ForeignClassItem *>(resolvedField);
    MergeForeignFieldCreate(data.reader, foreignClass, data.oldField, data.name, data.type);
}

void Context::MergeForeignFieldCreate(const panda_file::FileReader *reader, panda_file::BaseClassItem *clz,
                                      panda_file::ForeignFieldItem *ff, panda_file::StringItem *name,
                                      panda_file::TypeItem *typ)
{
    auto *fc = static_cast<panda_file::ForeignClassItem *>(clz);
    auto [iter, was_inserted] = foreignFields_.emplace(ForeignFieldKey {fc, name, typ}, nullptr);
    if (was_inserted) {
        iter->second = cont_.CreateItem<panda_file::ForeignFieldItem>(fc, name, typ, ff->GetAccessFlags());
    } else {
        result_.stats.deduplicatedForeigners++;
    }

    auto nff = iter->second;
    cameFrom_.emplace_back(nff, reader);
    knownItems_[ff] = nff;
}

std::vector<panda_file::Type> Helpers::BreakProto(panda_file::ProtoItem *p)
{
    auto &shorty = p->GetShorty();

    auto ret = std::vector<panda_file::Type>();
    ret.reserve(panda_file::SHORTY_ELEM_PER16 * shorty.size());

    // SHORTY
    size_t numElem = 0;
    [[maybe_unused]] size_t numRefs = 0;
    auto fetch = [idx = size_t(0), &shorty]() mutable {
        ASSERT(idx < shorty.size());
        return shorty[idx++];
    };

    auto v = fetch();

    while (v != 0) {
        auto shift = (numElem % panda_file::SHORTY_ELEM_PER16) * panda_file::SHORTY_ELEM_WIDTH;

        auto elem = (static_cast<decltype(shift)>(v) >> shift) & panda_file::SHORTY_ELEM_MASK;

        if (elem == 0) {
            break;
        }
        auto asId = panda_file::Type::TypeId(elem);
        ASSERT(asId != panda_file::Type::TypeId::INVALID);

        auto t = panda_file::Type(asId);
        if (t.IsReference()) {
            numRefs++;
        }
        static_assert(std::is_trivially_copyable_v<decltype(t)>);
        ret.emplace_back(t);

        numElem++;

        if (numElem % panda_file::SHORTY_ELEM_PER16 == 0) {
            v = fetch();
        }
    }

    ASSERT(numRefs == p->GetRefTypes().size());
    ASSERT(!ret.empty());

    return ret;
}

const Context::ProtoData &Context::GetProto(panda_file::ProtoItem *p)
{
    if (auto cached = protoCache_.find(p); cached != protoCache_.end()) {
        return cached->second;
    }

    auto &refs = p->GetRefTypes();

    auto typs = Helpers::BreakProto(p);

    panda_file::TypeItem *ret = nullptr;
    auto params = std::vector<panda_file::MethodParamItem>();
    params.reserve(typs.size() - 1);

    size_t numRefs = 0;

    for (auto const &t : typs) {
        panda_file::TypeItem *ti;

        if (t.IsReference()) {
            ASSERT(numRefs < refs.size());
            ti = TypeFromOld(refs[numRefs++]);
        } else {
            ti = cont_.GetOrCreatePrimitiveTypeItem(t);
        }

        if (ret == nullptr) {
            ret = ti;
        } else {
            params.emplace_back(ti);
        }
    }

    ASSERT(numRefs == refs.size());
    ASSERT(ret != nullptr);

    auto proto = cont_.GetOrCreateProtoItem(ret, params);

    auto [it, inserted] = protoCache_.emplace(p, ProtoData {proto, std::move(params)});
    ASSERT(inserted);
    return it->second;
}

template <typename T, typename Getter, typename Adder>
void Context::AddAnnotationImpl(AddAnnotationImplData<T> ad, Getter getter, Adder adder)
{
    const auto &oldAnnotList = DerefPtrRef(getter(ad.oi));
    for (size_t ind = ad.from; ind < oldAnnotList.size(); ind++) {
        auto oldAnnot = oldAnnotList[ind];
        auto mbNewAnnot = AnnotFromOld(oldAnnot);
        if (std::holds_alternative<panda_file::AnnotationItem *>(mbNewAnnot)) {
            adder(ad.ni, std::get<panda_file::AnnotationItem *>(mbNewAnnot));
            continue;
        }
        const auto &ed = std::get<ErrorDetail>(mbNewAnnot);
        if (ad.retriesLeft-- == 0) {
            std::vector<ErrorDetail> details {ErrorDetail {"annotation", oldAnnot}, ed};
            if constexpr (std::is_base_of_v<panda_file::BaseItem, std::decay_t<T>>) {
                details.emplace_back("old item", ad.oi);
                details.emplace_back("new item", ad.ni);
            }
            this->Error("can't transfer annotation", details, ad.reader);
            return;
        }

        LOG(DEBUG, STATIC_LINKER) << "defer annotation transferring due to" << ErrorToString(ed, 1);
        ad.from = ind;
        deferredFailedAnnotations_.emplace_back(
            [this, ad, getter, adder]() { AddAnnotationImpl<T>(ad, getter, adder); });
        return;
    }
}

template <typename T>
void Context::TransferAnnotations(const panda_file::FileReader *reader, T *ni, T *oi)
{
    AddAnnotationImplData<T> data {reader, ni, oi, 0, 1};
    // pointers to members break clang-12 on CI
    AddAnnotationImpl(
        data, [](T *self) -> decltype(auto) { return DerefPtrRef(self->GetRuntimeAnnotations()); },
        [](T *self, panda_file::AnnotationItem *an) { self->AddRuntimeAnnotation(an); });
    AddAnnotationImpl(
        data, [](T *self) -> decltype(auto) { return DerefPtrRef(self->GetAnnotations()); },
        [](T *self, panda_file::AnnotationItem *an) { self->AddAnnotation(an); });
    AddAnnotationImpl(
        data, [](T *self) -> decltype(auto) { return DerefPtrRef(self->GetRuntimeTypeAnnotations()); },
        [](T *self, panda_file::AnnotationItem *an) { self->AddRuntimeTypeAnnotation(an); });
    AddAnnotationImpl(
        data, [](T *self) -> decltype(auto) { return DerefPtrRef(self->GetTypeAnnotations()); },
        [](T *self, panda_file::AnnotationItem *an) { self->AddTypeAnnotation(an); });
}

std::variant<panda_file::ValueItem *, Context::ErrorDetail> Context::ArrayValueFromOld(panda_file::ValueItem *oi)
{
    auto old = oi->GetAsArray();
    auto its = old->GetItems();
    for (auto &i : its) {
        if (i.HasValue<panda_file::BaseItem *>()) {
            auto vl = ScalarValueIdFromOld(i.GetIdItem());
            if (std::holds_alternative<ErrorDetail>(vl)) {
                return std::move(std::get<ErrorDetail>(vl));
            }
            i = panda_file::ScalarValueItem(std::get<panda_file::BaseItem *>(vl));
        }
    }
    return cont_.CreateItem<panda_file::ArrayValueItem>(old->GetComponentType(), std::move(its));
}

std::variant<panda_file::ValueItem *, Context::ErrorDetail> Context::ValueFromOld(panda_file::ValueItem *oi)
{
    using ValueType = panda_file::ValueItem::Type;
    switch (oi->GetType()) {
        case ValueType::INTEGER:
            return cont_.GetOrCreateIntegerValueItem(oi->GetAsScalar()->GetValue<uint32_t>());
        case ValueType::LONG:
            return cont_.GetOrCreateLongValueItem(oi->GetAsScalar()->GetValue<uint64_t>());
        case ValueType::FLOAT:
            return cont_.GetOrCreateFloatValueItem(oi->GetAsScalar()->GetValue<float>());
        case ValueType::DOUBLE:
            return cont_.GetOrCreateDoubleValueItem(oi->GetAsScalar()->GetValue<double>());
        case ValueType::ID: {
            auto oldItem = oi->GetAsScalar()->GetIdItem();
            ASSERT(oldItem != nullptr);
            auto newItem = ScalarValueIdFromOld(oldItem);
            if (std::holds_alternative<ErrorDetail>(newItem)) {
                return std::move(std::get<ErrorDetail>(newItem));
            }
            return cont_.GetOrCreateIdValueItem(std::get<panda_file::BaseItem *>(newItem));
        }
        case ValueType::ARRAY: {
            return ArrayValueFromOld(oi);
        }
        default:
            UNREACHABLE();
    }
}

std::variant<panda_file::AnnotationItem *, Context::ErrorDetail> Context::AnnotFromOld(panda_file::AnnotationItem *oa)
{
    if (auto iter = knownItems_.find(oa); iter != knownItems_.end()) {
        return static_cast<panda_file::AnnotationItem *>(iter->second);
    }

    using Elem = panda_file::AnnotationItem::Elem;

    const auto &oldElems = *oa->GetElements();
    auto newElems = std::vector<Elem>();
    newElems.reserve(oldElems.size());
    for (const auto &oe : oldElems) {
        auto name = StringFromOld(oe.GetName());
        auto newVal = ValueFromOld(oe.GetValue());
        if (std::holds_alternative<ErrorDetail>(newVal)) {
            return std::move(std::get<ErrorDetail>(newVal));
        }

        newElems.emplace_back(name, std::get<panda_file::ValueItem *>(newVal));
    }

    auto clzIt = knownItems_.find(oa->GetClassItem());
    ASSERT(clzIt != knownItems_.end());
    ASSERT(clzIt->second->GetItemType() == panda_file::ItemTypes::CLASS_ITEM ||
           clzIt->second->GetItemType() == panda_file::ItemTypes::FOREIGN_CLASS_ITEM);
    auto clz = static_cast<panda_file::BaseClassItem *>(clzIt->second);

    auto na = cont_.CreateItem<panda_file::AnnotationItem>(clz, std::move(newElems), oa->GetTags());
    knownItems_.emplace(oa, na);
    return na;
}

std::variant<panda_file::BaseItem *, Context::ErrorDetail> Context::ScalarValueIdFromOld(panda_file::BaseItem *oi)
{
    if (auto newItemIt = knownItems_.find(static_cast<panda_file::IndexedItem *>(oi)); newItemIt != knownItems_.end()) {
        return newItemIt->second;
    }
    if (oi->GetItemType() == panda_file::ItemTypes::STRING_ITEM) {
        return StringFromOld(static_cast<panda_file::StringItem *>(oi));
    }
    if (oi->GetItemType() == panda_file::ItemTypes::ANNOTATION_ITEM) {
        auto oa = static_cast<panda_file::AnnotationItem *>(oi);
        using ReturnType = decltype(ScalarValueIdFromOld(oi));
        return std::visit([](auto &&a) -> ReturnType { return std::forward<decltype(a)>(a); }, AnnotFromOld(oa));
    }
    return ErrorDetail("id", oi);
}

void Context::Parse()
{
    const auto hardwareThreads = std::max(1U, std::thread::hardware_concurrency());
    const auto threadCount = std::min(codeDatas_.size(), static_cast<size_t>(hardwareThreads));
    if (threadCount <= 1) {
        patcher_.ReserveChanges(EstimatePatchChanges(0, codeDatas_.size()));
        ProcessCodeDataRange(&patcher_, 0, codeDatas_.size());
        ApplyPatchDependencies();
        return;
    }

    ParseConcurrently();
}

void Context::ApplyPatchDependencies()
{
    patcher_.ApplyDeps(this);
    ClearAndReleaseStorage(protoCache_);
    ClearAndReleaseStorage(stringCache_);
    ClearAndReleaseStorage(literalArrayCache_);
}

size_t Context::EstimatePatchChanges(size_t start, size_t end) const
{
    size_t patchChanges = end - start;
    for (auto idx = start; idx < end; idx++) {
        auto *code = codeDatas_[idx].nmi->GetCode();
        if (code == nullptr) {
            continue;
        }
        patchChanges += std::min(code->GetNumInstructions(), code->GetCodeSize());
    }
    return patchChanges;
}

void Context::ProcessCodeDataRange(CodePatcher *patcher, size_t start, size_t end)
{
    for (auto idx = start; idx < end; idx++) {
        auto changeStart = patcher->GetSize();
        ProcessCodeData(*patcher, &codeDatas_[idx]);
        // Keep one patch range per method.  Bytecode id updates inside different methods are independent and can be
        // patched in parallel; debug info changes collected in the same range are skipped here and handled serially.
        patcher->AddBytecodePatchRange({changeStart, patcher->GetSize()});
    }
}

void Context::ParseConcurrently()
{
    const auto hardwareThreads = std::max(1U, std::thread::hardware_concurrency());
    const auto threadCount = std::min(codeDatas_.size(), static_cast<size_t>(hardwareThreads));
    ASSERT(threadCount > 1);

    std::vector<CodePatcher> patchers(threadCount);
    std::vector<std::thread> threads;
    threads.reserve(threadCount);
    const auto chunkSize = (codeDatas_.size() + threadCount - 1U) / threadCount;
    for (size_t i = 0; i < threadCount; i++) {
        auto start = i * chunkSize;
        auto end = std::min(codeDatas_.size(), start + chunkSize);
        if (start >= end) {
            break;
        }
        patchers[i].ReserveChanges(EstimatePatchChanges(start, end));
        // Keep chunk order deterministic: local patchers are merged by increasing start index.
        threads.emplace_back(
            [this, start, end, &patcher = patchers[i]]() { ProcessCodeDataRange(&patcher, start, end); });
    }
    for (auto &thread : threads) {
        thread.join();
    }

    size_t totalChanges = 0;
    for (const auto &patcher : patchers) {
        totalChanges += patcher.GetSize();
    }
    patcher_.ReserveChanges(totalChanges);
    for (auto &patcher : patchers) {
        patcher_.Devour(std::move(patcher));
    }
    ApplyPatchDependencies();
}

void Context::ComputeLayout()
{
    cont_.ComputeLayout();
}

bool Context::MethodFind(const std::string &className, const std::string &methodName,
                         std::map<std::string, panda_file::BaseClassItem *> &classesMap)
{
    auto it = classesMap.find(className);
    if (it != classesMap.end() && !it->second->IsForeign()) {
        auto classItem = static_cast<panda_file::ClassItem *>(it->second);
        auto methodNameItem = std::make_unique<panda_file::StringItem>(methodName);
        auto range = classItem->FindMethod(methodNameItem.get());
        if (range.first != range.second) {
            classItem->SetDependencyMark();
            return true;
        }
    }
    return false;
}

bool Context::FileFind(const std::string &fileName, std::map<std::string, panda_file::BaseClassItem *> &classesMap)
{
    bool found = false;
    auto dotPos = fileName.rfind('.');
    if (dotPos == std::string::npos) {
        return found;
    }
    auto suffix = fileName.substr(dotPos + 1);
    if (suffix != "ets" && suffix != "sts") {
        return found;
    }

    auto fileNameFromEntry = fileName.substr(0, dotPos);
    std::replace(fileNameFromEntry.begin(), fileNameFromEntry.end(), '.', '/');
    fileNameFromEntry.insert(fileNameFromEntry.begin(), 'L');

    for (auto &entry : classesMap) {
        auto classItemName = entry.first;
        auto lastSlash = classItemName.rfind('/');
        if (lastSlash != std::string::npos) {
            auto fileNameFromClass = classItemName.substr(0, lastSlash);
            if (fileNameFromEntry == fileNameFromClass) {
                entry.second->SetDependencyMark();
                found = true;
            }
        }
    }
    return found;
}

bool Context::HandleEntryDependencies()
{
    auto &cm = *cont_.GetClassMap();
    for (const auto &name : conf_.entryNames) {
        auto dotPos = name.rfind('.');
        auto firstSlash = name.find('/');
        auto lastSlash = name.rfind('/');
        if (dotPos != std::string::npos && dotPos != 0 && dotPos < name.size() - 1) {
            // if entry is filename
            if (FileFind(name, cm)) {
                continue;
            }
        } else if (firstSlash != std::string::npos && lastSlash != std::string::npos && lastSlash > firstSlash) {
            // if entry is method
            auto classNameFromEntry = name.substr(0, lastSlash);
            classNameFromEntry.insert(classNameFromEntry.begin(), 'L');
            classNameFromEntry += ";";
            auto methodNameFromEntry = name.substr(lastSlash + 1);
            if (MethodFind(classNameFromEntry, methodNameFromEntry, cm)) {
                continue;
            }
        }
        // try look up as classname for last try
        auto className = name + ";";
        className.insert(className.begin(), 'L');
        auto it = cm.find(className);
        if (it != cm.end()) {
            it->second->SetDependencyMark();
        } else {
            Error("Entry not found", {ErrorDetail("Entry", name)});
            return false;
        }
    }
    return true;
}

void Context::TryDelete()
{
    if (!conf_.allFileIsEntry && conf_.entryNames.empty()) {
        return;
    }
    for (auto &name : conf_.entryNames) {
        if (name == "*") {
            conf_.allFileIsEntry = true;
        }
    }
    auto &cm = *cont_.GetClassMap();
    if (conf_.allFileIsEntry) {
        for (auto &entry : cm) {
            if (entry.second->GetItemType() == panda_file::ItemTypes::CLASS_ITEM) {
                entry.second->SetDependencyMark();
            }
        }
    } else {
        if (!HandleEntryDependencies()) {
            return;
        }
    }
    // All LiteralarrayMap should be reserved
    cont_.MarkLiteralarrayMap();
    patcher_.AddStringDependency();
    // patches may not be available after items delete, try delete patches before delete items.
    patcher_.TryDeletePatch();
    // delete class index map.
    for (auto it = cm.begin(); it != cm.end();) {
        if (it->second->GetDependencyMark()) {
            ++it;
        } else {
            it = cm.erase(it);
        }
    }
    cont_.DeleteForeignItems();
    cont_.DeleteItems();
}

void Context::Patch()
{
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr size_t BYTECODE_PATCH_CHUNK_SIZE = 256;

    auto patchSize = patcher_.GetSize();
    auto rangeCount = patcher_.GetBytecodePatchRangeCount();
    auto chunkCount = (rangeCount + BYTECODE_PATCH_CHUNK_SIZE - 1U) / BYTECODE_PATCH_CHUNK_SIZE;
    auto hardwareThreads = std::max(1U, std::thread::hardware_concurrency());
    auto threadCount = std::min(chunkCount, static_cast<size_t>(hardwareThreads));
    if (threadCount <= 1) {
        patcher_.Patch({0, patchSize});
        return;
    }

    std::vector<std::thread> threads;
    threads.reserve(threadCount);
    std::atomic_size_t nextRange {0};
    for (size_t i = 0; i < threadCount; i++) {
        threads.emplace_back([this, rangeCount, &nextRange]() {
            // Atomic with relaxed order reason: counter only gives each thread a separate bytecode range
            for (auto start = nextRange.fetch_add(BYTECODE_PATCH_CHUNK_SIZE, std::memory_order_relaxed);
                 start < rangeCount;
                 // Atomic with relaxed order reason: counter only gives each thread a separate bytecode range
                 start = nextRange.fetch_add(BYTECODE_PATCH_CHUNK_SIZE, std::memory_order_relaxed)) {
                auto end = std::min(rangeCount, start + BYTECODE_PATCH_CHUNK_SIZE);
                patcher_.PatchBytecodeRanges({start, end});
            }
        });
    }
    for (auto &thread : threads) {
        thread.join();
    }
    // Debug info patching mutates shared debug structures, so it stays serial after parallel bytecode patching.
    patcher_.PatchDebug({0, patchSize});
}

panda_file::BaseClassItem *Context::ClassFromOld(panda_file::BaseClassItem *old)
{
    if (old == nullptr) {
        return old;
    }
    if (auto known = knownItems_.find(old); known != knownItems_.end()) {
        ASSERT(known->second->GetItemType() == panda_file::ItemTypes::CLASS_ITEM ||
               known->second->GetItemType() == panda_file::ItemTypes::FOREIGN_CLASS_ITEM);
        return static_cast<panda_file::BaseClassItem *>(known->second);
    }
    auto &cm = *cont_.GetClassMap();
    if (auto iter = cm.find(GetStr(old->GetNameItem())); iter != cm.end()) {
        return iter->second;
    }
    UNREACHABLE();
}

panda_file::TypeItem *Context::TypeFromOld(panda_file::TypeItem *old)
{
    const auto ty = old->GetType();
    if (ty.IsPrimitive()) {
        return cont_.GetOrCreatePrimitiveTypeItem(ty.GetId());
    }
    ASSERT(old->GetItemType() == panda_file::ItemTypes::CLASS_ITEM ||
           old->GetItemType() == panda_file::ItemTypes::FOREIGN_CLASS_ITEM);
    return ClassFromOld(static_cast<panda_file::BaseClassItem *>(old));
}

std::string Context::GetStr(const panda_file::StringItem *si)
{
    return utf::Mutf8AsCString(reinterpret_cast<const uint8_t *>(si->GetData().data()));
}

panda_file::StringItem *Context::StringFromOld(const panda_file::StringItem *s)
{
    if (s == nullptr) {
        return nullptr;
    }
    if (auto known = stringCache_.find(s); known != stringCache_.end()) {
        return known->second;
    }
    auto *item = cont_.GetOrCreateStringItem(GetStr(s));
    stringCache_.emplace(s, item);
    return item;
}
}  // namespace ark::static_linker
