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

#ifndef _PANDA_VERIFIER_PANDA_TYPES_HPP
#define _PANDA_VERIFIER_PANDA_TYPES_HPP

#include "runtime/include/method.h"
#include "runtime/include/class.h"

#include "verification/type/type_systems.h"
#include "verification/type/type_system.h"
#include "verification/type/type_sort.h"
#include "verification/type/type_tags.h"
#include "verification/type/type_type_inl.h"

#include "verification/util/synchronized.h"
#include "verification/util/callable.h"
#include "verification/jobs/cache.h"

#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"

#include "libpandabase/os/mutex.h"
#include "plugins.h"

namespace panda::verifier {
class PandaTypes {
public:
    // TODO(vdyadov): change Id to hash from filename_id and entity id
    using Id = LibCache::Id;
    using TypeId = panda_file::Type::TypeId;

    // TODO(vdyadov): solve problem with cycles
    //       (todo: mutual recursive types)

    const PandaString &ClassNameOfId(Id id)
    {
        return ClassNameOfId_[id];
    }

    const PandaString &MethodNameOfId(Id id)
    {
        return MethodNameOfId_[id];
    }

    Type NormalizedTypeOf(Type type);
    TypeParams NormalizeMethodSignature(const TypeParams &sig);

    const TypeParams &MethodSignature(const CachedMethod &method);
    const TypeParams &NormalizedMethodSignature(const CachedMethod &method);

    TypeId TypeIdOf(const Type &type) const;

    Type TypeOf(const CachedMethod &method);
    Type TypeOf(const CachedClass &klass);
    Type TypeOf(TypeId id) const;

    Type TypeOf(const TypeParamIdx &idx) const
    {
        return {TypeSystemKind::JAVA, threadnum_, idx};
    }

    void CloseAccumulatedSubtypingRelation()
    {
        TypeSystem_.CloseAccumulatedSubtypingRelation();
    };

    SortIdx GetSort(const PandaString &name) const
    {
        return TypeSystems::GetSort(TypeSystemKind::JAVA, threadnum_, name);
    }

    TypeSystemKind GetKind() const
    {
        return TypeSystemKind::JAVA;
    }

    ThreadNum GetThreadNum() const
    {
        return threadnum_;
    }

    explicit PandaTypes(ThreadNum threadnum);
    NO_COPY_SEMANTIC(PandaTypes);

    ~PandaTypes() = default;
    Type Bot() const
    {
        return TypeSystem_.Bot();
    }
    Type Top() const
    {
        return TypeSystem_.Top();
    }
    const ParametricType &Array()
    {
        return Array_;
    }
    const ParametricType &Method()
    {
        return Method_;
    }
    const ParametricType &NormalizedMethod()
    {
        return NormalizedMethod_;
    }
    const ParametricType &Normalize()
    {
        return Normalize_;
    }
    const ParametricType &Abstract()
    {
        return Abstract_;
    }
    const ParametricType &Interface()
    {
        return Interface_;
    }
    const ParametricType &TypeClass()
    {
        return TypeClass_;
    }

    const Type &U1() const
    {
        return U1_;
    }
    const Type &I8() const
    {
        return I8_;
    }
    const Type &U8() const
    {
        return U8_;
    }
    const Type &I16() const
    {
        return I16_;
    }
    const Type &U16() const
    {
        return U16_;
    }
    const Type &I32() const
    {
        return I32_;
    }
    const Type &U32() const
    {
        return U32_;
    }
    const Type &I64() const
    {
        return I64_;
    }
    const Type &U64() const
    {
        return U64_;
    }
    const Type &F32() const
    {
        return F32_;
    }
    const Type &F64() const
    {
        return F64_;
    }

    const Type &RefType() const
    {
        return RefType_;
    }
    const Type &ObjectType() const
    {
        return ObjectType_;
    }
    const Type &StringType() const
    {
        return StringType_;
    }
    const Type &PrimitiveType() const
    {
        return PrimitiveType_;
    }
    const Type &AbstractType() const
    {
        return AbstractType_;
    }
    const Type &InterfaceType() const
    {
        return InterfaceType_;
    }
    const Type &TypeClassType() const
    {
        return TypeClassType_;
    }
    const Type &InstantiableType() const
    {
        return InstantiableType_;
    }
    const Type &ArrayType() const
    {
        return ArrayType_;
    }
    const Type &ObjectArrayType() const
    {
        return ObjectArrayType_;
    }
    const Type &MethodType() const
    {
        return MethodType_;
    }
    const Type &StaticMethodType() const
    {
        return StaticMethodType_;
    }
    const Type &NonStaticMethodType() const
    {
        return NonStaticMethodType_;
    }
    const Type &VirtualMethodType() const
    {
        return VirtualMethodType_;
    }
    const Type &NullRefType() const
    {
        return NullRefType_;
    }
    const Type &Bits32Type() const
    {
        return Bits32Type_;
    }
    const Type &Bits64Type() const
    {
        return Bits64Type_;
    }
    const Type &Integral8Type() const
    {
        return Integral8Type_;
    }
    const Type &Integral16Type() const
    {
        return Integral16Type_;
    }
    const Type &Integral32Type() const
    {
        return Integral32Type_;
    }
    const Type &Integral64Type() const
    {
        return Integral64Type_;
    }
    const Type &Float32Type() const
    {
        return Float32Type_;
    }
    const Type &Float64Type() const
    {
        return Float64Type_;
    }
    const Type &Object(panda::panda_file::SourceLang lang) const
    {
        return LangContextTypesObjects_.at(lang);
    }
    const Type &Class(panda::panda_file::SourceLang lang) const
    {
        return LangContextTypesClass_.at(lang);
    }
    const Type &Throwable(panda::panda_file::SourceLang lang) const
    {
        return LangContextTypesThrowables_.at(lang);
    }

    const PandaString &ImageOf(const Type &type)
    {
        return TypeSystems::ImageOfType(type);
    }
    PandaString ImageOf(const TypeParams &params)
    {
        return TypeSystems::ImageOfTypeParams(params);
    }
    template <typename Handler>
    void ForSubtypesOf(const Type &type, Handler &&handler) const
    {
        type.ForAllSubtypes(std::forward<Handler>(handler));
    }
    template <typename Handler>
    void ForSupertypesOf(const Type &type, Handler &&handler) const
    {
        type.ForAllSupertypes(std::forward<Handler>(handler));
    }
    PandaVector<Type> SubtypesOf(const Type &type) const
    {
        PandaVector<Type> result;
        type.ForAllSubtypes([&result](const auto &t) {
            result.push_back(t);
            return true;
        });
        return result;
    }
    PandaVector<Type> SupertypesOf(const Type &type) const
    {
        PandaVector<Type> result;
        type.ForAllSupertypes([&result](const auto &t) {
            result.push_back(t);
            return true;
        });
        return result;
    }
    template <typename Handler>
    void DisplayMethods(Handler handler)
    {
        if (DoNotCalculateMethodType_) {
            for (const auto &item : SigOfMethod_) {
                handler(MethodNameOfId(item.first), ImageOf(item.second));
            }
        } else {
            for (const auto &item : TypeOfMethod_) {
                handler(MethodNameOfId(item.first), ImageOf(item.second));
            }
        }
    }
    template <typename Handler>
    void DisplayClasses(Handler handler)
    {
        for (const auto &item : TypeOfClass_) {
            handler(ClassNameOfId(item.first), ImageOf(item.second));
        }
    }
    template <typename Handler>
    void DisplaySubtyping(Handler handler)
    {
        TypeSystem_.ForAllTypes([this, &handler](const Type &type) {
            type.ForAllSupertypes([this, &handler, &type](const Type &supertype) {
                handler(ImageOf(type), ImageOf(supertype));
                return true;
            });
            return true;
        });
    }
    template <typename Handler>
    void DisplayTypeSystem(Handler handler)
    {
        handler(PandaString {"Classes:"});
        DisplayClasses([&handler](const auto &name, const auto &type) { handler(name + " : " + type); });
        handler(PandaString {"Methods:"});
        DisplayMethods([&handler](const auto &name, const auto &type) { handler(name + " : " + type); });
        handler(PandaString {"Subtyping (type <: supertype):"});
        DisplaySubtyping([&handler](const auto &type, const auto &supertype) { handler(type + " <: " + supertype); });
    }

    bool DoNotCalculateMethodType() const
    {
        return DoNotCalculateMethodType_;
    }

    Variables::Var NewVar()
    {
        return Variables_.NewVar();
    }

private:
    ThreadNum threadnum_;
    PandaUnorderedMap<Id, Type> TypeOfClass_;
    PandaUnorderedMap<Id, Type> TypeOfMethod_;
    PandaUnorderedMap<Id, TypeParams> SigOfMethod_;
    PandaUnorderedMap<Id, TypeParams> NormalizedSigOfMethod_;
    PandaUnorderedMap<Id, PandaString> ClassNameOfId_;
    PandaUnorderedMap<Id, PandaString> MethodNameOfId_;
    PandaUnorderedMap<Type, Type> NormalizedTypeOf_;
    TypeSystem &TypeSystem_;
    Variables Variables_;

    // base sorts
    const ParametricType Array_;
    const ParametricType Method_;
    const ParametricType NormalizedMethod_;
    const ParametricType Normalize_;
    const ParametricType Abstract_;
    const ParametricType Interface_;
    const ParametricType TypeClass_;

    const Type U1_;
    const Type I8_;
    const Type U8_;
    const Type I16_;
    const Type U16_;
    const Type I32_;
    const Type U32_;
    const Type I64_;
    const Type U64_;
    const Type F32_;
    const Type F64_;

    const Type RefType_;
    const Type ObjectType_;
    const Type StringType_;
    const Type PrimitiveType_;
    const Type AbstractType_;
    const Type InterfaceType_;
    const Type TypeClassType_;
    const Type InstantiableType_;
    const Type ArrayType_;
    const Type ObjectArrayType_;
    const Type MethodType_;
    const Type StaticMethodType_;
    const Type NonStaticMethodType_;
    const Type VirtualMethodType_;
    const Type NullRefType_;
    const Type Bits32Type_;
    const Type Bits64Type_;
    const Type Integral8Type_;
    const Type Integral16Type_;
    const Type Integral32Type_;
    const Type Integral64Type_;
    const Type Float32Type_;
    const Type Float64Type_;

    std::map<panda::panda_file::SourceLang, const Type> LangContextTypesClass_;
    std::map<panda::panda_file::SourceLang, const Type> LangContextTypesObjects_;
    std::map<panda::panda_file::SourceLang, const Type> LangContextTypesThrowables_;

    bool DoNotCalculateMethodType_ {true};

    void SetArraySubtyping(const Type &t);

    Type TypeOfArray(const CachedClass &klass);

    ParametricType ParametricTypeForName(const PandaString &name)
    {
        return TypeSystem_.Parametric(GetSort(name));
    }

    Type TypeForName(const PandaString &name)
    {
        return ParametricTypeForName(name)();
    }
};
}  // namespace panda::verifier

#endif  // !_PANDA_VERIFIER_PANDA_TYPES_HPP
