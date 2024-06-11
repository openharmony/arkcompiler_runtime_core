/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_vtable_builder.h"

#include "ets_class_linker_extension.h"
#include "runtime/include/class_linker.h"

namespace ark::ets {

bool EtsVTableOverridePred::IsInSamePackage(const MethodInfo &info1, const MethodInfo &info2) const
{
    if (info1.GetLoadContext() != info2.GetLoadContext()) {
        return false;
    }

    auto *desc1 = info1.GetClassName();
    auto *desc2 = info2.GetClassName();

    while (ClassHelper::IsArrayDescriptor(desc1)) {
        desc1 = ClassHelper::GetComponentDescriptor(desc1);
    }

    while (ClassHelper::IsArrayDescriptor(desc2)) {
        desc2 = ClassHelper::GetComponentDescriptor(desc2);
    }

    Span sp1(desc1, 1);
    Span sp2(desc2, 1);
    while (sp1[0] == sp2[0] && sp1[0] != 0 && sp2[0] != 0) {
        sp1 = Span(sp1.cend(), 1);
        sp2 = Span(sp2.cend(), 1);
    }

    bool isSamePackage = true;
    while (sp1[0] != 0 && isSamePackage) {
        isSamePackage = sp1[0] != '/';
        sp1 = Span(sp1.cend(), 1);
    }

    while (sp2[0] != 0 && isSamePackage) {
        isSamePackage = sp2[0] != '/';
        sp2 = Span(sp2.cend(), 1);
    }

    return isSamePackage;
}

class RefTypeLink {
public:
    explicit RefTypeLink(uint8_t const *descr) : descriptor_(descr) {}
    RefTypeLink(panda_file::File const *pf, panda_file::File::EntityId idx)
        : pf_(pf), id_(idx), descriptor_(pf->GetStringData(idx).data)
    {
    }

    static RefTypeLink InPDA(panda_file::ProtoDataAccessor &pda, uint32_t idx)
    {
        return RefTypeLink(&pda.GetPandaFile(), pda.GetReferenceType(idx));
    }

    uint8_t const *GetDescriptor()
    {
        return descriptor_;
    }

    ALWAYS_INLINE static bool AreEqual(RefTypeLink const &a, RefTypeLink const &b)
    {
        if (LIKELY(a.pf_ == b.pf_ && a.pf_ != nullptr)) {
            return a.id_ == b.id_;
        }
        return utf::IsEqual(a.descriptor_, b.descriptor_);
    }

    ALWAYS_INLINE std::optional<panda_file::ClassDataAccessor> CreateCDA()
    {
        if (UNLIKELY(!Resolve())) {
            return std::nullopt;
        }
        return panda_file::ClassDataAccessor(*pf_, id_);
    }

private:
    bool Resolve()
    {
        if (pf_ != nullptr && !pf_->IsExternal(id_)) {
            return true;
        }

        auto linker = Runtime::GetCurrent()->GetClassLinker();

        // NOTE(vpukhov): speedup lookup with tls cache
        linker->EnumeratePandaFiles([this](panda_file::File const &itpf) {
            auto itClassId = itpf.GetClassId(descriptor_);
            if (itClassId.IsValid() && !itpf.IsExternal(itClassId)) {
                pf_ = &itpf;
                id_ = itClassId;
                return false;
            }
            return true;
        });
        return pf_ != nullptr;
    }

    panda_file::File const *pf_ {};
    panda_file::File::EntityId id_ {};
    uint8_t const *descriptor_ {};
};

static inline bool IsPrimitveDescriptor(uint8_t const *descr)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return descr[0] != 0 && descr[1] == 0;
}

static bool RefIsAssignableToImpl(RefTypeLink sub, RefTypeLink super, uint32_t depth);

static bool RefExtendsOrImplements(RefTypeLink sub, RefTypeLink super, uint32_t depth)
{
    auto superCDAOpt = super.CreateCDA();
    if (!superCDAOpt.has_value()) {
        return false;
    }
    panda_file::ClassDataAccessor const &superCDA = superCDAOpt.value();
    if (superCDA.IsFinal()) {
        return false;
    }
    auto subCDAOpt = sub.CreateCDA();
    if (!subCDAOpt.has_value()) {
        return false;
    }
    panda_file::ClassDataAccessor const &subCDA = subCDAOpt.value();

    if (superCDA.IsInterface()) {
        for (size_t i = 0; i < subCDA.GetIfacesNumber(); ++i) {
            auto iface = RefTypeLink(&subCDA.GetPandaFile(), subCDA.GetInterfaceId(i));
            if (RefIsAssignableToImpl(iface, super, depth)) {
                return true;
            }
        }
    }
    // subtype is interface (thus has no base class) or subtype is Object
    if (subCDA.IsInterface() || subCDA.GetSuperClassId().GetOffset() == 0) {
        return false;
    }
    return RefIsAssignableToImpl(RefTypeLink(&subCDA.GetPandaFile(), subCDA.GetSuperClassId()), super, depth);
}

static bool RefIsAssignableToImpl(RefTypeLink sub, RefTypeLink super, uint32_t depth)
{
    if (UNLIKELY(depth-- == 0)) {
        LOG(ERROR, CLASS_LINKER) << "Max class assignability test depth reached";
        return false;
    }
    if (RefTypeLink::AreEqual(sub, super)) {
        return true;
    }
    if (IsPrimitveDescriptor(sub.GetDescriptor()) || IsPrimitveDescriptor(super.GetDescriptor())) {
        return false;
    }
    if (utf::IsEqual(super.GetDescriptor(), utf::CStringAsMutf8(panda_file_items::class_descriptors::OBJECT.data()))) {
        return true;
    }
    if (ClassHelper::IsArrayDescriptor(super.GetDescriptor())) {
        if (!ClassHelper::IsArrayDescriptor(sub.GetDescriptor())) {
            return false;
        }
        RefTypeLink subComp(ClassHelper::GetComponentDescriptor(sub.GetDescriptor()));
        RefTypeLink superComp(ClassHelper::GetComponentDescriptor(super.GetDescriptor()));
        return RefIsAssignableToImpl(subComp, superComp, depth);
    }
    // Assume array does not implement interfaces
    if (ClassHelper::IsArrayDescriptor(sub.GetDescriptor())) {
        return false;
    }

    return RefExtendsOrImplements(sub, super, depth);
}

static inline bool RefIsAssignableTo(RefTypeLink sub, RefTypeLink super)
{
    static constexpr uint32_t ASSIGNABILITY_MAX_DEPTH = 256U;
    return RefIsAssignableToImpl(sub, super, ASSIGNABILITY_MAX_DEPTH);
}

bool ETSProtoIsOverriddenBy(Method::ProtoId const &base, Method::ProtoId const &derv)
{
    auto basePDA = panda_file::ProtoDataAccessor(base.GetPandaFile(), base.GetEntityId());
    auto dervPDA = panda_file::ProtoDataAccessor(derv.GetPandaFile(), derv.GetEntityId());
    if (dervPDA.GetNumElements() != basePDA.GetNumElements()) {
        return false;
    }
    uint32_t numElems = basePDA.GetNumElements();

    for (uint32_t i = 0, refIdx = 0; i < numElems; ++i) {
        if (dervPDA.GetType(i) != basePDA.GetType(i)) {
            return false;
        }
        if (dervPDA.GetType(i).IsReference()) {
            auto dervRef = RefTypeLink::InPDA(dervPDA, refIdx);
            auto baseRef = RefTypeLink::InPDA(basePDA, refIdx);
            auto res = i == 0 ? RefIsAssignableTo(dervRef, baseRef) : RefIsAssignableTo(baseRef, dervRef);
            if (!res) {
                return false;
            }
            refIdx++;
        }
    }
    return true;
}

}  // namespace ark::ets
