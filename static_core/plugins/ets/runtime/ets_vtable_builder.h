/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_VTABLE_BUILDER_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_VTABLE_BUILDER_H_

#include <cstddef>
#include <utility>

#include "runtime/class_linker_context.h"
#include "runtime/include/vtable_builder_variance-inl.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets {

class RefTypeLink;

bool ETSProtoIsOverriddenBy(const ClassLinkerContext *ctx, Method::ProtoId const &base, Method::ProtoId const &derv,
                            bool isStrict = false);
std::optional<RefTypeLink> GetClosestCommonAncestor(ClassLinker *cl, const ClassLinkerContext *ctx, RefTypeLink source,
                                                    RefTypeLink target);
panda_file::ClassDataAccessor GetClassDataAccessor(ClassLinker *cl, ClassLinkerContext *ctx, const uint8_t *desc);

class EtsVTableCompatibleSignatures {
public:
    explicit EtsVTableCompatibleSignatures(const ClassLinkerContext *ctx) : ctx_(ctx) {}

    bool operator()(const Method::ProtoId &base, const Method::ProtoId &derv, bool isStrict = false) const
    {
        return ETSProtoIsOverriddenBy(ctx_, base, derv, isStrict);
    }

private:
    const ClassLinkerContext *ctx_;
};

struct EtsVTableOverridePred {
    bool operator()(const MethodInfo *base, const MethodInfo *derv) const
    {
        if (base->IsPublic() || base->IsProtected()) {
            return true;
        }

        if (base->IsPrivate()) {
            return false;
        }

        return IsInSamePackage(*base, *derv);
    }

    bool IsInSamePackage(const MethodInfo &info1, const MethodInfo &info2) const;
};

using EtsVTableBuilder = VarianceVTableBuilder<EtsVTableCompatibleSignatures, EtsVTableOverridePred>;

class RefTypeLink {
public:
    explicit RefTypeLink(const ClassLinkerContext *ctx, uint8_t const *descr) : ctx_(ctx), descriptor_(descr) {}
    RefTypeLink(const ClassLinkerContext *ctx, panda_file::File const *pf, panda_file::File::EntityId idx)
        : ctx_(ctx), pf_(pf), id_(idx), descriptor_(pf->GetStringData(idx).data)
    {
    }

    static RefTypeLink InPDA(const ClassLinkerContext *ctx, panda_file::ProtoDataAccessor &pda, uint32_t idx)
    {
        uint8_t const *refDescriptor = pda.GetPandaFile().GetStringData(pda.GetReferenceType(idx)).data;
        if (ClassHelper::IsUnion(refDescriptor)) {
            auto langCtx =
                Runtime::GetCurrent()->GetLanguageContext(const_cast<ClassLinkerContext *>(ctx)->GetSourceLang());
            auto *ext = Runtime::GetCurrent()->GetClassLinker()->GetExtension(langCtx);
            auto [pf, id] = ext->ComputeLUBInfo(ctx, refDescriptor);
            return RefTypeLink(ctx, pf, id);
        }
        return RefTypeLink(ctx, &pda.GetPandaFile(), pda.GetReferenceType(idx));
    }

    uint8_t const *GetDescriptor()
    {
        return descriptor_;
    }

    panda_file::File::EntityId GetId() const
    {
        return id_;
    }

    panda_file::File const *GetPandaFile() const
    {
        return pf_;
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
    bool Resolve();

    bool IsResolved() const
    {
        return (pf_ != nullptr) && !pf_->IsExternal(id_);
    }

    const ClassLinkerContext *ctx_ {};
    panda_file::File const *pf_ {};
    panda_file::File::EntityId id_ {};
    uint8_t const *descriptor_ {};
};

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_ITABLE_BUILDER_H_
