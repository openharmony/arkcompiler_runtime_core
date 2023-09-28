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

#include <tuple>
#include <type_traits>
#include <variant>

#include "libpandafile/file_items.h"
#include "libpandafile/file_reader.h"
#include "libpandafile/proto_data_accessor-inl.h"
#include "libpandabase/utils/utf.h"

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

namespace panda::static_linker {

void Context::Merge()
{
    // types can reference types only
    // methods can reference types
    // code items can reference everything

    // parse regular classes as forward declarations
    // parse all foreign classes
    // parse classes with fields and methods
    // iterate all known indexed entities and create
    //   appropriate mappings for them
    //   oldEntity -> newEntity

    auto &cm = *cont_.GetClassMap();

    // set offsets of all entities
    for (auto &reader : readers_) {
        for (auto [o, i] : *reader.GetItems()) {
            i->SetOffset(o.GetOffset());
        }
    }

    // add regular classes
    for (auto &reader : readers_) {
        auto *ic = reader.GetContainerPtr();
        auto &classes = *ic->GetClassMap();

        known_items_.reserve(reader.GetItems()->size());

        for (auto [name, i] : classes) {
            if (i->IsForeign()) {
                continue;
            }
            ASSERT(name == GetStr(i->GetNameItem()));

            if (conf_.partial.count(name) == 0) {
                if (auto iter = cm.find(name); iter != cm.end()) {
                    Error("Class redefinition", {ErrorDetail("original", iter->second)}, &reader);
                }
            }
            auto clz = cont_.GetOrCreateClassItem(name);
            known_items_[i] = clz;
            came_from_.emplace(clz, &reader);
        }
    }

    // find all referenced foreign classes
    for (auto &reader : readers_) {
        auto *items = reader.GetItems();
        for (auto &it_pair : *items) {
            auto *item = it_pair.second;

            if (item->GetItemType() == panda_file::ItemTypes::FOREIGN_CLASS_ITEM) {
                auto *fc = static_cast<panda_file::ForeignClassItem *>(item);
                auto name = GetStr(fc->GetNameItem());
                if (auto iter = cm.find(name); iter != cm.end()) {
                    known_items_[fc] = iter->second;
                } else {
                    auto clz = cont_.GetOrCreateForeignClassItem(name);
                    came_from_.emplace(clz, &reader);
                    known_items_[fc] = clz;
                }
            }
        }
    }

    // fill regular classes
    for (auto &reader : readers_) {
        auto *ic = reader.GetContainerPtr();
        auto &classes = *ic->GetClassMap();

        for (auto [name, i] : classes) {
            if (i->IsForeign()) {
                continue;
            }
            ASSERT(cm.find(name) != cm.end());
            auto ni = static_cast<panda_file::ClassItem *>(cm[name]);
            auto oi = static_cast<panda_file::ClassItem *>(i);
            MergeClass(&reader, ni, oi);
        }
    }

    // scrap all indexable items
    for (auto &reader : readers_) {
        auto *items = reader.GetItems();
        for (auto [offset, item] : *items) {
            item->SetOffset(offset.GetOffset());

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
    }

    for (const auto &f : deferred_failed_annotations_) {
        f();
    }
    deferred_failed_annotations_.clear();
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
    known_items_[oi] = ni;
    came_from_.emplace(ni, reader);

    TransferAnnotations(reader, ni, oi);
}

void Context::MergeMethod(const panda_file::FileReader *reader, panda_file::ClassItem *clz, panda_file::MethodItem *oi)
{
    auto [proto, params] = GetProto(oi->GetProto());
    auto &old_params = oi->GetParams();
    auto *ni = clz->AddMethod(StringFromOld(oi->GetNameItem()), proto, oi->GetAccessFlags(), params);
    known_items_[oi] = ni;
    came_from_.emplace(ni, reader);
    ni->SetProfileSize(oi->GetProfileSize());

    for (size_t i = 0; i < old_params.size(); i++) {
        TransferAnnotations(reader, &ni->GetParams()[i], &old_params[i]);
    }

    TransferAnnotations(reader, ni, oi);

    if (ni->HasRuntimeParamAnnotations()) {
        cont_.CreateItem<panda_file::ParamAnnotationsItem>(ni, true);
    }

    if (ni->HasParamAnnotations()) {
        cont_.CreateItem<panda_file::ParamAnnotationsItem>(ni, false);
    }

    auto should_save = false;
    std::vector<uint8_t> *instructions = nullptr;

    auto patch_lnp = false;

    if (auto odbg = oi->GetDebugInfo(); !conf_.strip_debug_info && odbg != nullptr) {
        panda_file::LineNumberProgramItem *lnp_item {};

        auto old_program = odbg->GetLineNumberProgram();
        if (auto old = known_items_.find(old_program); old != known_items_.end()) {
            lnp_item = static_cast<panda_file::LineNumberProgramItem *>(old->second);
            cont_.IncRefLineNumberProgramItem(lnp_item);
        } else {
            lnp_item = cont_.CreateLineNumberProgramItem();
            known_items_.emplace(old_program, lnp_item);

            should_save = true;
            result_.stats.debug_count++;
            patch_lnp = true;
        }

        auto *ndbg = cont_.CreateItem<panda_file::DebugInfoItem>(lnp_item);
        ndbg->SetLineNumber(odbg->GetLineNumber());
        ni->SetDebugInfo(ndbg);
        for (auto *s : *odbg->GetParameters()) {
            ndbg->AddParameter(StringFromOld(s));
        }
    }

    if (auto oci = oi->GetCode(); oci != nullptr) {
        should_save = true;
        result_.stats.code_count++;

        auto nci = cont_.CreateItem<panda_file::CodeItem>();
        ni->SetCode(nci);
        nci->SetPGORank(oci->GetPGORank());
        nci->SetNumArgs(oci->GetNumArgs());
        nci->SetNumVregs(oci->GetNumVregs());
        *nci->GetInstructions() = *oci->GetInstructions();
        instructions = nci->GetInstructions();
        nci->SetNumInstructions(oci->GetNumInstructions());

        for (const auto &otb : oci->GetTryBlocks()) {
            auto ncbs = std::vector<panda_file::CodeItem::CatchBlock>();
            const auto &ocbs = otb.GetCatchBlocks();
            ncbs.reserve(ocbs.size());
            for (const auto &ocb : ocbs) {
                ASSERT(ocb.GetMethod() == oi);
                auto exc_klass = ClassFromOld(ocb.GetType());
                if (exc_klass != nullptr) {
                    ni->AddIndexDependency(exc_klass);
                }
                ncbs.emplace_back(ni, exc_klass, ocb.GetHandlerPc(), ocb.GetCodeSize());
            }
            auto ntb = panda_file::CodeItem::TryBlock(otb.GetStartPc(), otb.GetLength(), ncbs);
            nci->AddTryBlock(ntb);
        }
    }

    if (should_save) {
        code_datas_.push_back(CodeData {instructions, oi, ni, reader, patch_lnp});
    }
}

bool Context::IsSameType(panda::panda_file::TypeItem *nevv, panda::panda_file::TypeItem *old)
{
    if (nevv->GetType().IsPrimitive()) {
        if (!old->GetType().IsPrimitive()) {
            return false;
        }
        return nevv->GetType() == old->GetType();
    }

    auto iter = known_items_.find(old);
    return iter != known_items_.end() && iter->second == nevv;
}

std::variant<bool, panda_file::MethodItem *> Context::TryFindMethod(panda_file::BaseClassItem *klass,
                                                                    panda_file::ForeignMethodItem *fm,
                                                                    std::vector<ErrorDetail> *related_items)
{
    if (klass == nullptr) {
        return false;
    }
    if (klass->GetItemType() == panda_file::ItemTypes::FOREIGN_CLASS_ITEM) {
        return true;
    }
    ASSERT(klass->GetItemType() == panda_file::ItemTypes::CLASS_ITEM);
    auto kls = static_cast<panda_file::ClassItem *>(klass);
    auto op = fm->GetProto();

    auto [new_meth_beg, new_meth_end] = kls->FindMethod(fm->GetNameItem());

    for (auto beg = new_meth_beg; beg != new_meth_end; ++beg) {
        auto nm = beg->get();
        auto np = nm->GetProto();

        related_items->emplace_back("candidate", nm);

        if (op->GetRefTypes().size() != np->GetRefTypes().size()) {
            continue;
        }

        if (op->GetShorty() != np->GetShorty()) {
            continue;
        }

        bool ok = true;
        for (size_t i = 0; i < op->GetRefTypes().size(); i++) {
            if (!IsSameType(np->GetRefTypes()[i], op->GetRefTypes()[i])) {
                ok = false;
                break;
            }
        }

        if (ok) {
            return nm;
        }
    }

    panda_file::BaseClassItem *search_in = kls->GetSuperClass();
    size_t idx = 0;
    while (true) {
        auto res = TryFindMethod(search_in, fm, related_items);
        if (std::holds_alternative<bool>(res)) {
            if (std::get<bool>(res)) {
                return res;
            }
        }
        if (std::holds_alternative<panda_file::MethodItem *>(res)) {
            return res;
        }
        if (idx >= kls->GetInterfaces().size()) {
            return false;
        }
        search_in = kls->GetInterfaces()[idx++];
    }
    return false;
}

void Context::MergeForeignMethod(const panda_file::FileReader *reader, panda_file::ForeignMethodItem *fm)
{
    ASSERT(known_items_.find(fm) == known_items_.end());
    ASSERT(known_items_.find(fm->GetClassItem()) != known_items_.end());
    auto clz = static_cast<panda_file::BaseClassItem *>(known_items_[fm->GetClassItem()]);
    std::vector<panda::static_linker::Context::ErrorDetail> details = {{"method", fm}};
    auto res = TryFindMethod(clz, fm, &details);
    if (std::holds_alternative<bool>(res) || conf_.remains_partial.count(GetStr(clz->GetNameItem())) != 0) {
        if (std::get<bool>(res) || conf_.remains_partial.count(GetStr(clz->GetNameItem())) != 0) {
            MergeForeignMethodCreate(reader, clz, fm);
        } else {
            Error("Unresolved method", details, reader);
        }
    } else {
        ASSERT(std::holds_alternative<panda_file::MethodItem *>(res));
        auto meth = std::get<panda_file::MethodItem *>(res);
        if (meth->GetClassItem() != ClassFromOld(fm->GetClassItem())) {
            LOG(DEBUG, STATIC_LINKER) << "Resolved method\n"
                                      << ErrorToString({"old method", fm}, 1) << '\n'
                                      << ErrorToString({"new method", meth}, 1);
        }
        known_items_[fm] = meth;
    }
}

void Context::MergeForeignMethodCreate(const panda_file::FileReader *reader, panda_file::BaseClassItem *clz,
                                       panda_file::ForeignMethodItem *fm)
{
    auto *fc = static_cast<panda_file::BaseClassItem *>(clz);
    auto name = StringFromOld(fm->GetNameItem());
    auto proto = GetProto(fm->GetProto()).first;
    auto access = fm->GetAccessFlags();
    auto [iter, was_inserted] = foreign_methods_.emplace(
        std::piecewise_construct, std::forward_as_tuple(fc, name, proto, access), std::forward_as_tuple(nullptr));
    if (was_inserted) {
        iter->second = cont_.CreateItem<panda_file::ForeignMethodItem>(fc, name, proto, access);
    } else {
        result_.stats.deduplicated_foreigners++;
    }
    auto nfm = iter->second;
    came_from_.emplace(nfm, reader);
    known_items_[fm] = nfm;
}

std::variant<std::monostate, panda_file::FieldItem *, panda_file::ForeignClassItem *> Context::TryFindField(
    panda_file::BaseClassItem *klass, const std::string &name, panda_file::TypeItem *expected_type,
    std::vector<panda_file::FieldItem *> *bad_candidates)
{
    if (klass == nullptr) {
        return std::monostate {};
    }
    if (klass->GetItemType() == panda_file::ItemTypes::FOREIGN_CLASS_ITEM) {
        return static_cast<panda_file::ForeignClassItem *>(klass);
    }
    auto kls = static_cast<panda_file::ClassItem *>(klass);
    panda_file::FieldItem *new_fld = nullptr;
    kls->VisitFields([&](panda_file::BaseItem *bi) {
        ASSERT(bi->GetItemType() == panda_file::ItemTypes::FIELD_ITEM);
        auto fld = static_cast<panda_file::FieldItem *>(bi);
        if (fld->GetNameItem()->GetData() != name) {
            return true;
        }
        if (fld->GetTypeItem() != expected_type) {
            if (bad_candidates != nullptr) {
                bad_candidates->push_back(fld);
            }
            return true;
        }
        new_fld = fld;
        return false;
    });
    if (new_fld != nullptr) {
        return new_fld;
    }
    return TryFindField(kls->GetSuperClass(), name, expected_type, bad_candidates);
}

void Context::MergeForeignField(const panda_file::FileReader *reader, panda_file::ForeignFieldItem *ff)
{
    ASSERT(known_items_.find(ff) == known_items_.end());
    ASSERT(known_items_.find(ff->GetClassItem()) != known_items_.end());
    auto clz = static_cast<panda_file::BaseClassItem *>(known_items_[ff->GetClassItem()]);
    if (clz->GetItemType() != panda_file::ItemTypes::FOREIGN_CLASS_ITEM) {
        ASSERT(clz->GetItemType() == panda_file::ItemTypes::CLASS_ITEM);
        auto rclz = static_cast<panda_file::ClassItem *>(clz);
        std::vector<panda_file::FieldItem *> candidates;
        auto res = TryFindField(rclz, ff->GetNameItem()->GetData(), TypeFromOld(ff->GetTypeItem()), &candidates);

        std::visit(
            [&](auto el) {
                using T = decltype(el);
                if constexpr (std::is_same_v<T, std::monostate>) {
                    if (conf_.remains_partial.count(GetStr(clz->GetNameItem())) != 0) {
                        MergeForeignFieldCreate(reader, clz, ff);
                    } else {
                        auto details = std::vector<ErrorDetail> {{"field", ff}};
                        for (const auto &c : candidates) {
                            details.emplace_back("candidate", c);
                        }
                        Error("Unresolved field", details, reader);
                    }
                } else if constexpr (std::is_same_v<T, panda_file::FieldItem *>) {
                    known_items_[ff] = el;
                } else {
                    ASSERT((std::is_same_v<T, panda_file::ForeignClassItem *>));
                    // el or klass? propagate field to base or not?
                    auto field_klass = el;
                    LOG(DEBUG, STATIC_LINKER) << "Propagating foreign field to base\n"
                                              << ErrorToString({"field", ff}, 1) << '\n'
                                              << ErrorToString({"new base", field_klass}, 1);
                    MergeForeignFieldCreate(reader, field_klass, ff);
                }
            },
            res);
    } else {
        MergeForeignFieldCreate(reader, clz, ff);
    }
}

void Context::MergeForeignFieldCreate(const panda_file::FileReader *reader, panda_file::BaseClassItem *clz,
                                      panda_file::ForeignFieldItem *ff)
{
    auto *fc = static_cast<panda_file::ForeignClassItem *>(clz);
    auto name = StringFromOld(ff->GetNameItem());
    auto typ = TypeFromOld(ff->GetTypeItem());
    auto [iter, was_inserted] = foreign_fields_.emplace(std::piecewise_construct, std::forward_as_tuple(fc, name, typ),
                                                        std::forward_as_tuple(nullptr));
    if (was_inserted) {
        iter->second = cont_.CreateItem<panda_file::ForeignFieldItem>(fc, name, typ);
    } else {
        result_.stats.deduplicated_foreigners++;
    }

    auto nff = iter->second;
    came_from_.emplace(nff, reader);
    known_items_[ff] = nff;
}

std::vector<panda_file::Type> Helpers::BreakProto(panda_file::ProtoItem *p)
{
    auto &shorty = p->GetShorty();

    auto ret = std::vector<panda_file::Type>();
    ret.reserve(panda_file::SHORTY_ELEM_PER16 * shorty.size());

    // SHORTY
    size_t num_elem = 0;
    size_t num_refs = 0;
    auto fetch = [idx = size_t(0), &shorty]() mutable {
        ASSERT(idx < shorty.size());
        return shorty[idx++];
    };

    auto v = fetch();

    while (v != 0) {
        auto shift = (num_elem % panda_file::SHORTY_ELEM_PER16) * panda_file::SHORTY_ELEM_WIDTH;

        auto elem = (static_cast<decltype(shift)>(v) >> shift) & panda_file::SHORTY_ELEM_MASK;

        if (elem == 0) {
            break;
        }
        auto as_id = panda_file::Type::TypeId(elem);
        ASSERT(as_id != panda_file::Type::TypeId::INVALID);

        auto t = panda_file::Type(as_id);
        if (t.IsReference()) {
            num_refs++;
        }
        static_assert(std::is_trivially_copyable_v<decltype(t)>);
        ret.emplace_back(t);

        num_elem++;

        if (num_elem % panda_file::SHORTY_ELEM_PER16 == 0) {
            v = fetch();
        }
    }

    ASSERT(num_refs == p->GetRefTypes().size());
    ASSERT(!ret.empty());

    return ret;
}

std::pair<panda_file::ProtoItem *, std::vector<panda_file::MethodParamItem>> Context::GetProto(panda_file::ProtoItem *p)
{
    auto &refs = p->GetRefTypes();

    auto typs = Helpers::BreakProto(p);

    panda_file::TypeItem *ret = nullptr;
    auto params = std::vector<panda_file::MethodParamItem>();
    params.reserve(typs.size() - 1);

    size_t num_refs = 0;

    for (auto const &t : typs) {
        panda_file::TypeItem *ti;

        if (t.IsReference()) {
            ASSERT(num_refs < refs.size());
            ti = TypeFromOld(refs[num_refs++]);
        } else {
            ti = cont_.GetOrCreatePrimitiveTypeItem(t);
        }

        if (ret == nullptr) {
            ret = ti;
        } else {
            params.emplace_back(ti);
        }
    }

    ASSERT(num_refs == refs.size());
    ASSERT(ret != nullptr);

    auto proto = cont_.GetOrCreateProtoItem(ret, params);

    return std::make_pair(proto, std::move(params));
}

template <typename T, typename Getter, typename Adder>
void Context::AddAnnotationImpl(AddAnnotationImplData<T> ad, Getter getter, Adder adder)
{
    const auto &old_annot_list = DerefPtrRef(getter(ad.oi));
    for (size_t ind = ad.from; ind < old_annot_list.size(); ind++) {
        auto old_annot = old_annot_list[ind];
        auto mb_new_annot = AnnotFromOld(old_annot);
        if (std::holds_alternative<panda_file::AnnotationItem *>(mb_new_annot)) {
            adder(ad.ni, std::get<panda_file::AnnotationItem *>(mb_new_annot));
            return;
        }
        const auto &ed = std::get<ErrorDetail>(mb_new_annot);
        if (ad.retries_left-- == 0) {
            std::vector<ErrorDetail> details {ErrorDetail {"annotation", old_annot}, ed};
            if constexpr (std::is_base_of_v<panda_file::BaseItem, std::decay_t<T>>) {
                details.emplace_back("old item", ad.oi);
                details.emplace_back("new item", ad.ni);
            }
            this->Error("can't transfer annotation", details, ad.reader);
            return;
        }

        LOG(DEBUG, STATIC_LINKER) << "defer annotation transferring due to" << ErrorToString(ed, 1);
        ad.from = ind;
        deferred_failed_annotations_.emplace_back([=]() { AddAnnotationImpl<T>(ad, getter, adder); });
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

std::variant<panda_file::AnnotationItem *, Context::ErrorDetail> Context::AnnotFromOld(panda_file::AnnotationItem *oa)
{
    if (auto iter = known_items_.find(oa); iter != known_items_.end()) {
        return static_cast<panda_file::AnnotationItem *>(iter->second);
    }

    using Elem = panda_file::AnnotationItem::Elem;

    const auto &old_elems = *oa->GetElements();
    auto new_elems = std::vector<Elem>();
    new_elems.reserve(old_elems.size());
    for (const auto &oe : old_elems) {
        auto name = StringFromOld(oe.GetName());
        auto *old_val = oe.GetValue();
        panda_file::ValueItem *new_val {};

        using ValueType = panda_file::ValueItem::Type;
        switch (old_val->GetType()) {
            case ValueType::INTEGER:
                new_val = cont_.GetOrCreateIntegerValueItem(old_val->GetAsScalar()->GetValue<uint32_t>());
                break;
            case ValueType::LONG:
                new_val = cont_.GetOrCreateLongValueItem(old_val->GetAsScalar()->GetValue<uint64_t>());
                break;
            case ValueType::FLOAT:
                new_val = cont_.GetOrCreateFloatValueItem(old_val->GetAsScalar()->GetValue<float>());
                break;
            case ValueType::DOUBLE:
                new_val = cont_.GetOrCreateDoubleValueItem(old_val->GetAsScalar()->GetValue<double>());
                break;
            case ValueType::ID: {
                auto old_item = old_val->GetAsScalar()->GetIdItem();
                ASSERT(old_item != nullptr);
                auto new_item = ScalarValueIdFromOld(old_item);
                if (std::holds_alternative<ErrorDetail>(new_item)) {
                    return std::move(std::get<ErrorDetail>(new_item));
                }
                new_val = cont_.GetOrCreateIdValueItem(std::get<panda_file::BaseItem *>(new_item));
                break;
            }
            case ValueType::ARRAY: {
                auto old = old_val->GetAsArray();
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
                new_val = cont_.CreateItem<panda_file::ArrayValueItem>(old->GetComponentType(), std::move(its));
                break;
            }
            default:
                UNREACHABLE();
        }

        ASSERT(new_val != nullptr);
        new_elems.emplace_back(name, new_val);
    }

    auto clz_it = known_items_.find(oa->GetClassItem());
    ASSERT(clz_it != known_items_.end());
    ASSERT(clz_it->second->GetItemType() == panda_file::ItemTypes::CLASS_ITEM ||
           clz_it->second->GetItemType() == panda_file::ItemTypes::FOREIGN_CLASS_ITEM);
    auto clz = static_cast<panda_file::BaseClassItem *>(clz_it->second);

    auto na = cont_.CreateItem<panda_file::AnnotationItem>(clz, std::move(new_elems), oa->GetTags());
    known_items_.emplace(oa, na);
    return na;
}

std::variant<panda_file::BaseItem *, Context::ErrorDetail> Context::ScalarValueIdFromOld(panda_file::BaseItem *oi)
{
    if (auto new_item_it = known_items_.find(static_cast<panda_file::IndexedItem *>(oi));
        new_item_it != known_items_.end()) {
        return new_item_it->second;
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
    for (auto &code_data : code_datas_) {
        ProcessCodeData(patcher_, &code_data);
    }
    patcher_.ApplyDeps(this);
}

void Context::ComputeLayout()
{
    cont_.ComputeLayout();
}

void Context::Patch()
{
    patcher_.Patch({0, patcher_.GetSize()});
}

panda_file::BaseClassItem *Context::ClassFromOld(panda_file::BaseClassItem *old)
{
    if (old == nullptr) {
        return old;
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
    return cont_.GetOrCreateStringItem(GetStr(s));
}
}  // namespace panda::static_linker
