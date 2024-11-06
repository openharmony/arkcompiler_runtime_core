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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TS2ETS_ETS_TYPE_VISITOR_H_
#define PANDA_PLUGINS_ETS_RUNTIME_TS2ETS_ETS_TYPE_VISITOR_H_

#include "libpandabase/macros.h"
#include "libpandafile/proto_data_accessor-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/heap_manager.h"

namespace ark::ets::interop::js {

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
#define TYPEVIS_RAISE_ERROR(val)                       \
    do {                                               \
        Error() = (val);                               \
        return; /* CC-OFF(G.PRE.05) code generation */ \
    } while (0)

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TYPEVIS_ABRUPT_ON_ERROR()                          \
    do {                                                   \
        if (UNLIKELY(!!Error()))                           \
            return; /* CC-OFF(G.PRE.05) code generation */ \
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
// CC-OFFNXT(G.PRE.09) code generation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_VIS(name) virtual void Visit##name() = 0;
    TYPEVIS_PRIM_TYPES_LIST(DEF_VIS)
#undef DEF_VIS

    virtual void VisitPrimitive(const ark::panda_file::Type type)
    {
        switch (type.GetId()) {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DELEGATE(name)                                               \
    /* CC-OFFNXT(G.PRE.02) namespace member */                       \
    case ark::panda_file::Type::TypeId::name: {                      \
        return Visit##name(); /* CC-OFF(G.PRE.05) code generation */ \
    }
            TYPEVIS_PRIM_TYPES_LIST(DELEGATE)
#undef DELEGATE

            default:
                TYPEVIS_RAISE_ERROR("bad primitive type");
        }
    }

    virtual void VisitString(ark::Class *klass) = 0;
    virtual void VisitArray(ark::Class *klass) = 0;

    virtual void VisitFieldPrimitive(ark::Field const *field, ark::panda_file::Type type) = 0;
    virtual void VisitFieldReference(ark::Field const *field, ark::Class *klass) = 0;

    virtual void VisitField(ark::Field const *field)
    {
        auto type = field->GetType();
        if (type.IsPrimitive()) {
            return VisitFieldPrimitive(field, type);
        }
        return VisitFieldReference(field, field->ResolveTypeClass());
    }

    virtual void VisitObject(ark::Class *klass)
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

    virtual void VisitReference(ark::Class *klass)
    {
        if (klass->IsStringClass()) {
            return VisitString(klass);
        }
        if (klass->IsArrayClass()) {
            return VisitArray(klass);
        }
        return VisitObject(klass);
    }

    virtual void VisitClass(ark::Class *klass)
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
        return inStatic_;
    }

private:
    std::optional<std::string> error_;
    bool inStatic_ = false;
};

class EtsMethodVisitor {
public:
    explicit EtsMethodVisitor(ark::Method *method) : method_(method) {}

    virtual void VisitMethod()
    {
        refArgIdx_ = 0;
        auto excludeThis = static_cast<uint32_t>(!method_->IsStatic());
        auto numArgs = method_->GetNumArgs() - excludeThis;
        VisitReturn();
        TYPEVIS_ABRUPT_ON_ERROR();
        for (uint32_t i = 0; i < numArgs; ++i) {
            VisitArgument(i);
            TYPEVIS_ABRUPT_ON_ERROR();
        }
    }

    virtual void VisitArgs()
    {
        refArgIdx_ = 0;
        auto excludeThis = static_cast<uint32_t>(!method_->IsStatic());
        auto numArgs = method_->GetNumArgs() - excludeThis;
        if (!method_->GetReturnType().IsPrimitive()) {
            refArgIdx_++;
        }
        for (uint32_t i = 0; i < numArgs; ++i) {
            VisitArgument(i);
            TYPEVIS_ABRUPT_ON_ERROR();
        }
    }

    virtual void VisitReturn()
    {
        refArgIdx_ = 0;
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
    virtual void VisitReturn(ark::panda_file::Type type) = 0;
    virtual void VisitReturn(ark::Class *klass) = 0;
    virtual void VisitArgument(uint32_t idx, ark::panda_file::Type type) = 0;
    virtual void VisitArgument(uint32_t idx, ark::Class *klass) = 0;

private:
    void VisitReturnImpl()
    {
        panda_file::Type type = method_->GetReturnType();
        if (type.IsPrimitive()) {
            return VisitReturn(type);
        }
        return VisitReturn(ResolveRefClass());
    }

    void VisitArgument(uint32_t argIdx)
    {
        auto excludeThis = static_cast<uint32_t>(!method_->IsStatic());
        panda_file::Type type = method_->GetArgType(argIdx + excludeThis);
        if (type.IsPrimitive()) {
            return VisitArgument(argIdx, type);
        }
        return VisitArgument(argIdx, ResolveRefClass());
    }

    ark::Class *ResolveRefClass()
    {
        auto pf = method_->GetPandaFile();
        panda_file::MethodDataAccessor mda(*pf, method_->GetFileId());
        panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
        auto classLinker = ark::Runtime::GetCurrent()->GetClassLinker();
        auto ctx = method_->GetClass()->GetLoadContext();

        auto klassId = pda.GetReferenceType(refArgIdx_++);
        auto klass = classLinker->GetClass(*pf, klassId, ctx);
        return klass;
    }

    ark::Method *method_ {};
    uint32_t refArgIdx_ {};
    std::optional<std::string> error_;
};

class EtsConvertorRef {
public:
    using ObjRoot = ark::ObjectHeader **;
    using ValVariant = std::variant<ark::Value, ObjRoot>;

    EtsConvertorRef() = default;
    explicit EtsConvertorRef(ValVariant *dataPtr)
    {
        u_.dataPtr = dataPtr;  // NOLINT(cppcoreguidelines-pro-type-union-access)
    }
    EtsConvertorRef(ObjRoot obj, size_t offs) : isField_(true)
    {
        u_.field.obj = obj;    // NOLINT(cppcoreguidelines-pro-type-union-access)
        u_.field.offs = offs;  // NOLINT(cppcoreguidelines-pro-type-union-access)
    }

    template <typename T>
    T LoadPrimitive() const
    {
        if (isField_) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            return (*u_.field.obj)->GetFieldPrimitive<T>(u_.field.offs);
        }
        return std::get<ark::Value>(*u_.dataPtr).GetAs<T>();  // NOLINT(cppcoreguidelines-pro-type-union-access)
    }

    ark::ObjectHeader *LoadReference() const
    {
        if (isField_) {
            return (*u_.field.obj)->GetFieldObject(u_.field.offs);  // NOLINT(cppcoreguidelines-pro-type-union-access)
        }
        return *std::get<ObjRoot>(*u_.dataPtr);  // NOLINT(cppcoreguidelines-pro-type-union-access)
    }

    template <typename T>
    void StorePrimitive(T val)
    {
        if (isField_) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            (*u_.field.obj)->SetFieldPrimitive<T>(u_.field.offs, val);
        } else {
            *u_.dataPtr = ark::Value(val);  // NOLINT(cppcoreguidelines-pro-type-union-access)
        }
    }

    void StoreReference(ObjRoot val)
    {
        if (isField_) {
            (*u_.field.obj)->SetFieldObject(u_.field.offs, *val);  // NOLINT(cppcoreguidelines-pro-type-union-access)
        } else {
            *u_.dataPtr = val;  // NOLINT(cppcoreguidelines-pro-type-union-access)
        }
    }

private:
    union USlot {
        struct FieldSlot {  // field slot
            ObjRoot obj = nullptr;
            size_t offs = 0;
        };

        FieldSlot field;
        ValVariant *dataPtr = nullptr;  // handle or primitive slot

        USlot() {}  // NOLINT(modernize-use-equals-default)
    };

    USlot u_ {};
    bool isField_ = false;
};

}  // namespace ark::ets::interop::js

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_TS2ETS_ETS_TYPE_VISITOR_H_
