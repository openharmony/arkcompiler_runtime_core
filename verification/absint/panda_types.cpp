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

#include "panda_types.h"

#include "runtime/include/class_linker.h"
#include "runtime/include/method.h"
#include "runtime/include/method-inl.h"
#include "runtime/include/class.h"
#include "runtime/include/runtime.h"

#include "utils/span.h"
#include "verification/type/type_params.h"
#include "verification/type/type_system.h"
#include "verification/type/type_sort.h"
#include "verification/type/type_image.h"
#include "verification/jobs/cache.h"

#include "verifier_messages.h"

#include "utils/logger.h"

#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"

#include "libpandabase/os/mutex.h"

namespace panda::verifier {

Type PandaTypes::NormalizedTypeOf(Type type)
{
    ASSERT(type.IsValid());
    if (type.IsBot() || type.IsTop()) {
        return type;
    }
    auto t = NormalizedTypeOf_.find(type);
    if (t != NormalizedTypeOf_.cend()) {
        return t->second;
    }
    Type result = type;
    if (type <= Integral32Type()) {
        result = Normalize()(~Integral32Type());
        NormalizedTypeOf_[type] = result;
    } else if (type <= Integral64Type()) {
        result = Normalize()(~Integral64Type());
        NormalizedTypeOf_[type] = result;
        // NOLINTNEXTLINE(bugprone-branch-clone)
    } else if (type <= F32()) {
        result = Normalize()(~F32());
        NormalizedTypeOf_[type] = result;
    } else if (type <= F64()) {
        result = Normalize()(~F64());
        NormalizedTypeOf_[type] = result;
    } else if (type <= MethodType()) {
        result = NormalizedMethod()(NormalizeMethodSignature(type.Params()));
    }
    NormalizedTypeOf_[type] = result;
    return result;
}

TypeParams PandaTypes::NormalizeMethodSignature(const TypeParams &sig)
{
    TypeParams result {TypeSystemKind::JAVA, threadnum_};
    sig.ForEach([&result, this](const auto &param) {
        const Type &type = param;
        result >> (NormalizedTypeOf(type) * param.Variance());
    });
    return result;
}

const TypeParams &PandaTypes::NormalizedMethodSignature(const CachedMethod &method)
{
    auto &&method_id = method.id;
    auto it = NormalizedSigOfMethod_.find(method_id);
    if (it != NormalizedSigOfMethod_.end()) {
        return it->second;
    }
    auto &&sig = MethodSignature(method);
    auto &&normalized_sig = NormalizeMethodSignature(sig);
    NormalizedSigOfMethod_[method_id] = normalized_sig;
    return NormalizedSigOfMethod_[method_id];
}

const TypeParams &PandaTypes::MethodSignature(const CachedMethod &method)
{
    auto &&method_id = method.id;
    auto it = SigOfMethod_.find(method_id);
    if (it != SigOfMethod_.end()) {
        return it->second;
    }
    TypeParams params {TypeSystemKind::JAVA, threadnum_};
    Type return_type;
    bool return_is_processed = false;
    for (const auto &arg : method.signature) {
        Type t = LibCache::Visit(
            arg,
            [this](const LibCache::CachedClass &cached_class) {
                return cached_class.type_id == TypeId::VOID ? Top() : TypeOf(cached_class);
            },
            [&]([[maybe_unused]] const LibCache::DescriptorString &descriptor) {
                LOG_VERIFIER_JAVA_TYPES_METHOD_ARG_WAS_NOT_RESOLVED(method.GetName());
                return Top();
            });

        if (!t.IsValid()) {
            LOG_VERIFIER_JAVA_TYPES_METHOD_ARG_CANNOT_BE_CONVERTED_TO_TYPE(method.GetName());
        }
        if (return_is_processed) {
            params >> -t;
        } else {
            return_type = t;
            return_is_processed = true;
        }
    }
    params >> +return_type;
    SigOfMethod_[method_id] = params;
    return SigOfMethod_[method_id];
}

PandaTypes::TypeId PandaTypes::TypeIdOf(const Type &type) const
{
    if (type == U1()) {
        return TypeId::U1;
    }
    if (type == U8()) {
        return TypeId::U8;
    }
    if (type == U16()) {
        return TypeId::U16;
    }
    if (type == U32()) {
        return TypeId::U32;
    }
    if (type == U64()) {
        return TypeId::U64;
    }
    if (type == I8()) {
        return TypeId::I8;
    }
    if (type == I16()) {
        return TypeId::I16;
    }
    if (type == I32()) {
        return TypeId::I32;
    }
    if (type == I64()) {
        return TypeId::I64;
    }
    if (type == F32()) {
        return TypeId::F32;
    }
    if (type == F64()) {
        return TypeId::F64;
    }
    if (type.IsTop()) {
        return TypeId::VOID;
    }

    return TypeId::INVALID;
}

Type PandaTypes::TypeOf(const CachedMethod &method)
{
    auto id = method.id;
    auto k = TypeOfMethod_.find(id);
    if (k != TypeOfMethod_.end()) {
        return k->second;
    }
    ASSERT(!DoNotCalculateMethodType_);
    auto &&sig = MethodSignature(method);
    auto &&norm_sig = NormalizedMethodSignature(method);
    Type type;
    type = Method()(sig);
    type << MethodType();
    TypeOfMethod_[id] = type;
    // Normalize(Method) <: NormalizedMethod(NormalizedMethodSig)
    auto norm_type = Normalize()(~type);
    auto norm_method = NormalizedMethod()(norm_sig);
    norm_type << norm_method;
    NormalizedTypeOf_[type] = norm_method;
    MethodNameOfId_[id] = method.GetName();
    return type;
}

void PandaTypes::SetArraySubtyping(const Type &t)
{
    PandaVector<Type> to_process;
    PandaVector<Type> just_subtype;
    t.ForAllSupertypes([&]([[maybe_unused]] const Type &st) {
        if (!Array()[+st]) {
            to_process.emplace_back(st);
        } else {
            just_subtype.emplace_back(st);
        }
        return true;
    });
    auto array_type = Array()(+t);
    for (const auto &st : just_subtype) {
        array_type << Array()(+st);
    }
    for (const auto &st : to_process) {
        array_type << Array()(+st);
        SetArraySubtyping(st);
    }
}

Type PandaTypes::TypeOfArray(const CachedClass &klass)
{
    ASSERT(klass.flags[CachedClass::Flag::ARRAY_CLASS]);

    Type type;
    auto component = klass.GetArrayComponent();
    if (component.HasRef()) {
        auto component_type = TypeOf(component.Get());
        type = Array()(+component_type);
        SetArraySubtyping(component_type);
    } else {
        type = Array()(+Top());
        LOG_VERIFIER_JAVA_TYPES_ARRAY_COMPONENT_TYPE_IS_UNDEFINED();
    }
    type << ArrayType();
    if (klass.flags[CachedClass::Flag::OBJECT_ARRAY_CLASS]) {
        type << ObjectArrayType();
    }

    return type;
}

Type PandaTypes::TypeOf(const CachedClass &klass)
{
    auto id = klass.id;
    auto k = TypeOfClass_.find(id);
    if (k != TypeOfClass_.end()) {
        return k->second;
    }

    PandaVector<Type> supertypes;
    for (const auto &ancestor : klass.ancestors) {
        // ancestor here cannot be unresolved descriptor
        ASSERT(LibCache::IsRef(ancestor));
        supertypes.emplace_back(TypeOf(LibCache::GetRef(ancestor)));
    }

    Type type;
    bool is_primitive = klass.flags[CachedClass::Flag::PRIMITIVE];

    auto class_name = klass.GetName();

    if (klass.flags[CachedClass::Flag::ARRAY_CLASS]) {
        type = TypeOfArray(klass);
    } else if (is_primitive) {
        type = TypeOf(klass.type_id);
    } else {
        type = TypeForName(class_name);
    }

    if (!is_primitive) {
        bool is_string = klass.flags[CachedClass::Flag::STRING_CLASS];
        if (is_string) {
            type << StringType();
        } else {
            type << ObjectType();
        }
        NullRefType() << type << RefType();
        TypeClass()(~type) << TypeClassType() << RefType();
    }
    if (klass.flags[CachedClass::Flag::ABSTRACT]) {
        Abstract()(~type) << AbstractType();
    }
    for (auto &super : supertypes) {
        type << super;
    }
    ClassNameOfId_[id] = class_name;
    TypeOfClass_[id] = type;
    NormalizedTypeOf(type);
    return type;
}

Type PandaTypes::TypeOf(PandaTypes::TypeId id) const
{
    Type type;
    switch (id) {
        case TypeId::VOID:
            type = Top();
            break;
        case TypeId::U1:
            type = U1();
            break;
        case TypeId::I8:
            type = I8();
            break;
        case TypeId::U8:
            type = U8();
            break;
        case TypeId::I16:
            type = I16();
            break;
        case TypeId::U16:
            type = U16();
            break;
        case TypeId::I32:
            type = I32();
            break;
        case TypeId::U32:
            type = U32();
            break;
        case TypeId::I64:
            type = I64();
            break;
        case TypeId::U64:
            type = U64();
            break;
        case TypeId::F32:
            type = F32();
            break;
        case TypeId::F64:
            type = F64();
            break;
        case TypeId::REFERENCE:
            type = RefType();
            break;
        default:
            LOG_VERIFIER_JAVA_TYPES_CANNOT_CONVERT_TYPE_ID_TO_TYPE(id);
            type = Top();
    }
    return type;
}

PandaTypes::PandaTypes(ThreadNum threadnum)
    : threadnum_ {threadnum},
      TypeSystem_ {TypeSystems::Get(TypeSystemKind::JAVA, threadnum_)},
      Array_ {ParametricTypeForName("Array")},
      Method_ {ParametricTypeForName("Method")},
      NormalizedMethod_ {ParametricTypeForName("NormalizedMethod")},
      Normalize_ {ParametricTypeForName("Normalize")},
      Abstract_ {ParametricTypeForName("Abstract")},
      Interface_ {ParametricTypeForName("Interface")},
      TypeClass_ {ParametricTypeForName("TypeClass")},

      U1_ {TypeForName("u1")},
      I8_ {TypeForName("i8")},
      U8_ {TypeForName("u8")},
      I16_ {TypeForName("i16")},
      U16_ {TypeForName("u16")},
      I32_ {TypeForName("i32")},
      U32_ {TypeForName("u32")},
      I64_ {TypeForName("i64")},
      U64_ {TypeForName("u64")},
      F32_ {TypeForName("f32")},
      F64_ {TypeForName("f64")},

      RefType_ {TypeForName("RefType")},
      ObjectType_ {TypeForName("ObjectType")},
      StringType_ {TypeForName("StringType")},
      PrimitiveType_ {TypeForName("PrimitiveType")},
      AbstractType_ {TypeForName("AbstractType")},
      InterfaceType_ {TypeForName("InterfaceType")},
      TypeClassType_ {TypeForName("TypeClassType")},
      InstantiableType_ {TypeForName("InstantiableType")},
      ArrayType_ {TypeForName("ArrayType")},
      ObjectArrayType_ {TypeForName("ObjectArrayType")},
      MethodType_ {TypeForName("MethodType")},
      StaticMethodType_ {TypeForName("StaticMethodType")},
      NonStaticMethodType_ {TypeForName("NonStaticMethodType")},
      VirtualMethodType_ {TypeForName("VirtualMethodType")},
      NullRefType_ {TypeForName("NullRefType")},
      Bits32Type_ {TypeForName("32Bits")},
      Bits64Type_ {TypeForName("64Bits")},
      Integral8Type_ {TypeForName("Integral8Bits")},
      Integral16Type_ {TypeForName("Integral16Bits")},
      Integral32Type_ {TypeForName("Integral32Bits")},
      Integral64Type_ {TypeForName("Integral64Bits")},
      Float32Type_ {TypeForName("Float32Bits")},
      Float64Type_ {TypeForName("Float64Bits")}
{
    for (panda::panda_file::SourceLang lang : panda::panda_file::LANG_ITERATOR) {
        LanguageContextBase *ctx = panda::plugins::GetLanguageContextBase(lang);

        auto emplace = [&](auto &types, const char *name) {
            if (name != nullptr) {
                types.emplace(lang, TypeForName(name));
            }
        };

        emplace(LangContextTypesClass_, ctx->GetVerificationTypeClass());
        emplace(LangContextTypesObjects_, ctx->GetVerificationTypeObject());
        emplace(LangContextTypesThrowables_, ctx->GetVerificationTypeThrowable());
    }

    TypeSystem_.SetIncrementalRelationClosureMode(false);

    // base subtyping of primitive types
    I8() << I16() << I32();
    U1() << U8() << U16() << U32();
    // integral
    TypeSet(U1(), I8(), U8()) << Integral8Type();
    TypeSet(Integral8Type(), I16(), U16()) << Integral16Type();
    TypeSet(Integral16Type(), I32(), U32()) << Integral32Type();
    TypeSet(I64(), U64()) << Integral64Type();
    // sizes
    F32() << Float32Type();
    F64() << Float64Type();
    TypeSet(Integral32Type(), Float32Type()) << Bits32Type();
    TypeSet(Integral64Type(), Float64Type()) << Bits64Type();
    TypeSet(Bits32Type(), Bits64Type()) << PrimitiveType();

    TypeClassType() << RefType();

    TypeSet object_or_ref(ObjectType(), RefType());
    for (const auto &[lang, type] : LangContextTypesClass_) {
        NullRefType() << type << object_or_ref;
        (void)lang;
    }

    for (const auto &[lang, type] : LangContextTypesObjects_) {
        NullRefType() << type << object_or_ref;
        (void)lang;
    }

    for (const auto &[lang, type] : LangContextTypesThrowables_) {
        NullRefType() << type << object_or_ref;
        (void)lang;
    }

    NullRefType() << (ArrayType() | ObjectArrayType()) << RefType();

    for (const auto &[lang, type] : LangContextTypesObjects_) {
        TypeClass()(~Object(lang)) << TypeClassType();
        (void)type;
    }

    TypeSystem_.CloseSubtypingRelation();

    TypeSystem_.SetIncrementalRelationClosureMode(false);
    TypeSystem_.SetDeferIncrementalRelationClosure(false);
}

}  // namespace panda::verifier
