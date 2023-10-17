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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TS2ETS_ETS_TYPE_VISITOR_H_
#define PANDA_PLUGINS_ETS_RUNTIME_TS2ETS_ETS_TYPE_VISITOR_H_

#include "libpandabase/macros.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/heap_manager.h"

namespace panda::ets::interop::js {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TYPEVIS_PRIM_TYPES_LIST(V) \
    V(U1)                          \
    V(I8)                          \
    V(U16)                         \
    V(I16)                         \
    V(I32)                         \
    V(I64)                         \
    V(F32)                         \
    V(F64)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TYPEVIS_RAISE_ERROR(val) \
    do {                         \
        Error() = (val);         \
        return;                  \
    } while (0)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TYPEVIS_ABRUPT_ON_ERROR() \
    do {                          \
        if (UNLIKELY(!!Error()))  \
            return;               \
    } while (0)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TYPEVIS_CHECK_ERROR(expr, val) \
    do {                               \
        if (UNLIKELY(!(expr)))         \
            TYPEVIS_RAISE_ERROR(val);  \
    } while (0)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TYPEVIS_CHECK_FORWARD_ERROR(err)         \
    do {                                         \
        auto &_e = (err);                        \
        TYPEVIS_CHECK_ERROR(!_e, std::move(_e)); \
    } while (0)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TYPEVIS_NAPI_CHECK(expr) TYPEVIS_CHECK_ERROR((expr) == napi_ok, #expr)

class EtsTypeVisitor {
public:
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_VIS(name) virtual void Visit##name() = 0;
    TYPEVIS_PRIM_TYPES_LIST(DEF_VIS)
#undef DEF_VIS

    virtual void VisitPrimitive(const panda::panda_file::Type type)
    {
        switch (type.GetId()) {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DELEGATE(name)                            \
    case panda::panda_file::Type::TypeId::name: { \
        return Visit##name();                     \
    }
            TYPEVIS_PRIM_TYPES_LIST(DELEGATE)
#undef DELEGATE

            default:
                TYPEVIS_RAISE_ERROR("bad primitive type");
        }
    }

    virtual void VisitString(panda::Class *klass) = 0;
    virtual void VisitArray(panda::Class *klass) = 0;

    virtual void VisitFieldPrimitive(panda::Field const *field, panda::panda_file::Type type) = 0;
    virtual void VisitFieldReference(panda::Field const *field, panda::Class *klass) = 0;

    virtual void VisitField(panda::Field const *field)
    {
        auto type = field->GetType();
        if (type.IsPrimitive()) {
            return VisitFieldPrimitive(field, type);
        }
        return VisitFieldReference(field, field->ResolveTypeClass());
    }

    virtual void VisitObject(panda::Class *klass)
    {
        auto fields = klass->GetInstanceFields();
        for (auto const &f : fields) {
            VisitField(&f);
        }
        InStatic() = true;
        auto sfields = klass->GetStaticFields();
        for (auto const &f : sfields) {
            VisitField(&f);
        }
        InStatic() = false;
        TYPEVIS_ABRUPT_ON_ERROR();
    }

    virtual void VisitReference(panda::Class *klass)
    {
        if (klass->IsStringClass()) {
            return VisitString(klass);
        }
        if (klass->IsArrayClass()) {
            return VisitArray(klass);
        }
        return VisitObject(klass);
    }

    virtual void VisitClass(panda::Class *klass)
    {
        if (klass->IsPrimitive()) {
            return VisitPrimitive(klass->GetType());
        }
        return VisitReference(klass);
    }

    std::optional<std::string> &Error()
    {
        return error_;
    }

    bool &InStatic()
    {
        return in_static_;
    }

private:
    std::optional<std::string> error_;
    bool in_static_ = false;
};

class EtsMethodVisitor {
public:
    explicit EtsMethodVisitor(panda::Method *method) : method_(method) {}

    virtual void VisitMethod()
    {
        ref_arg_idx_ = 0;
        auto exclude_this = static_cast<uint32_t>(!method_->IsStatic());
        auto num_args = method_->GetNumArgs() - exclude_this;
        VisitReturn();
        TYPEVIS_ABRUPT_ON_ERROR();
        for (uint32_t i = 0; i < num_args; ++i) {
            VisitArgument(i);
            TYPEVIS_ABRUPT_ON_ERROR();
        }
    }

    virtual void VisitArgs()
    {
        ref_arg_idx_ = 0;
        auto exclude_this = static_cast<uint32_t>(!method_->IsStatic());
        auto num_args = method_->GetNumArgs() - exclude_this;
        if (!method_->GetReturnType().IsPrimitive()) {
            ref_arg_idx_++;
        }
        for (uint32_t i = 0; i < num_args; ++i) {
            VisitArgument(i);
            TYPEVIS_ABRUPT_ON_ERROR();
        }
    }

    virtual void VisitReturn()
    {
        ref_arg_idx_ = 0;
        VisitReturnImpl();
    }

    std::optional<std::string> &Error()
    {
        return error_;
    }

    auto GetMethod() const
    {
        return method_;
    }

protected:
    virtual void VisitReturn(panda::panda_file::Type type) = 0;
    virtual void VisitReturn(panda::Class *klass) = 0;
    virtual void VisitArgument(uint32_t idx, panda::panda_file::Type type) = 0;
    virtual void VisitArgument(uint32_t idx, panda::Class *klass) = 0;

private:
    void VisitReturnImpl()
    {
        panda_file::Type type = method_->GetReturnType();
        if (type.IsPrimitive()) {
            return VisitReturn(type);
        }
        return VisitReturn(ResolveRefClass());
    }

    void VisitArgument(uint32_t arg_idx)
    {
        auto exclude_this = static_cast<uint32_t>(!method_->IsStatic());
        panda_file::Type type = method_->GetArgType(arg_idx + exclude_this);
        if (type.IsPrimitive()) {
            return VisitArgument(arg_idx, type);
        }
        return VisitArgument(arg_idx, ResolveRefClass());
    }

    panda::Class *ResolveRefClass()
    {
        auto pf = method_->GetPandaFile();
        panda_file::MethodDataAccessor mda(*pf, method_->GetFileId());
        panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
        auto class_linker = panda::Runtime::GetCurrent()->GetClassLinker();
        auto ctx = method_->GetClass()->GetLoadContext();

        auto klass_id = pda.GetReferenceType(ref_arg_idx_++);
        auto klass = class_linker->GetClass(*pf, klass_id, ctx);
        return klass;
    }

    panda::Method *method_ {};
    uint32_t ref_arg_idx_ {};
    std::optional<std::string> error_;
};

class EtsConvertorRef {
public:
    using ObjRoot = panda::ObjectHeader **;
    using ValVariant = std::variant<panda::Value, ObjRoot>;

    EtsConvertorRef() = default;
    explicit EtsConvertorRef(ValVariant *data_ptr)
    {
        u_.data_ptr = data_ptr;  // NOLINT(cppcoreguidelines-pro-type-union-access)
    }
    EtsConvertorRef(ObjRoot obj, size_t offs) : is_field_(true)
    {
        u_.field.obj = obj;    // NOLINT(cppcoreguidelines-pro-type-union-access)
        u_.field.offs = offs;  // NOLINT(cppcoreguidelines-pro-type-union-access)
    }

    template <typename T>
    T LoadPrimitive() const
    {
        if (is_field_) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            return (*u_.field.obj)->GetFieldPrimitive<T>(u_.field.offs);
        }
        return std::get<panda::Value>(*u_.data_ptr).GetAs<T>();  // NOLINT(cppcoreguidelines-pro-type-union-access)
    }

    panda::ObjectHeader *LoadReference() const
    {
        if (is_field_) {
            return (*u_.field.obj)->GetFieldObject(u_.field.offs);  // NOLINT(cppcoreguidelines-pro-type-union-access)
        }
        return *std::get<ObjRoot>(*u_.data_ptr);  // NOLINT(cppcoreguidelines-pro-type-union-access)
    }

    template <typename T>
    void StorePrimitive(T val)
    {
        if (is_field_) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            (*u_.field.obj)->SetFieldPrimitive<T>(u_.field.offs, val);
        } else {
            *u_.data_ptr = panda::Value(val);  // NOLINT(cppcoreguidelines-pro-type-union-access)
        }
    }

    void StoreReference(ObjRoot val)
    {
        if (is_field_) {
            (*u_.field.obj)->SetFieldObject(u_.field.offs, *val);  // NOLINT(cppcoreguidelines-pro-type-union-access)
        } else {
            *u_.data_ptr = val;  // NOLINT(cppcoreguidelines-pro-type-union-access)
        }
    }

private:
    union USlot {
        struct FieldSlot {  // field slot
            ObjRoot obj = nullptr;
            size_t offs = 0;
        };

        FieldSlot field;
        ValVariant *data_ptr = nullptr;  // handle or primitive slot

        USlot() {}  // NOLINT(modernize-use-equals-default)
    };

    USlot u_ {};
    bool is_field_ = false;
};

}  // namespace panda::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_TS2ETS_ETS_TYPE_VISITOR_H_
