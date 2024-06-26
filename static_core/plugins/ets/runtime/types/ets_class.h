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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_CLASS_H_
#define PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_CLASS_H_

#include <cstdint>
#include "include/mem/panda_containers.h"
#include "include/object_header.h"
#include "include/runtime.h"
#include "libpandabase/mem/object_pointer.h"
#include "runtime/class_linker_context.h"
#include "runtime/include/class-inl.h"
#include "runtime/include/thread.h"
#include "runtime/include/class_linker.h"
#include "plugins/ets/runtime/types/ets_field.h"
#include "plugins/ets/runtime/types/ets_type.h"
#include "plugins/ets/runtime/ets_panda_file_items.h"

namespace ark::ets {

enum class AccessLevel { PUBLIC, PROTECTED, DEFAULT, PRIVATE };

class EtsMethod;
class EtsObject;
class EtsString;
class EtsArray;
class EtsPromise;
class EtsErrorOptions;
class EtsTypeAPIField;
class EtsTypeAPIMethod;
class EtsTypeAPIParameter;

enum class EtsType;

class EtsClass {
public:
    // We shouldn't init header_ here - because it has been memset(0) in object allocation,
    // otherwise it may cause data race while visiting object's class concurrently in gc.
    void InitClass(const uint8_t *descriptor, uint32_t vtableSize, uint32_t imtSize, uint32_t klassSize)
    {
        new (&klass_) ark::Class(descriptor, panda_file::SourceLang::ETS, vtableSize, imtSize, klassSize);
    }

    const char *GetDescriptor() const
    {
        return reinterpret_cast<const char *>(GetRuntimeClass()->GetDescriptor());
    }

    PANDA_PUBLIC_API EtsClass *GetBase();

    uint32_t GetFieldsNumber();

    uint32_t GetOwnFieldsNumber();  // Without inherited fields

    uint32_t GetStaticFieldsNumber()
    {
        return GetRuntimeClass()->GetStaticFields().size();
    }

    uint32_t GetInstanceFieldsNumber()
    {
        return GetRuntimeClass()->GetInstanceFields().size();
    }

    uint32_t GetFieldIndexByName(const char *name);

    PANDA_PUBLIC_API PandaVector<EtsField *> GetFields();

    EtsField *GetFieldByIndex(uint32_t i);

    EtsField *GetFieldIDByOffset(uint32_t fieldOffset);
    PANDA_PUBLIC_API EtsField *GetFieldIDByName(const char *name, const char *sig = nullptr);

    EtsField *GetOwnFieldByIndex(uint32_t i);
    EtsField *GetDeclaredFieldIDByName(const char *name);

    PANDA_PUBLIC_API EtsField *GetStaticFieldIDByName(const char *name, const char *sig = nullptr);
    EtsField *GetStaticFieldIDByOffset(uint32_t fieldOffset);

    PANDA_PUBLIC_API EtsMethod *GetDirectMethod(const char *name);
    PANDA_PUBLIC_API EtsMethod *GetDirectMethod(const uint8_t *name, const char *signature);
    PANDA_PUBLIC_API EtsMethod *GetDirectMethod(const char *name, const char *signature);

    PANDA_PUBLIC_API EtsMethod *GetMethod(const char *name);
    PANDA_PUBLIC_API EtsMethod *GetMethod(const char *name, const char *signature);

    PANDA_PUBLIC_API EtsMethod *GetDirectMethod(const PandaString &name, const PandaString &signature)
    {
        return GetDirectMethod(name.c_str(), signature.c_str());
    }

    PANDA_PUBLIC_API EtsMethod *GetDirectMethod(const char *name, const Method::Proto &proto) const;

    EtsMethod *GetMethod(const PandaString &name, const PandaString &signature)
    {
        return GetMethod(name.c_str(), signature.c_str());
    }

    EtsMethod *GetMethodByIndex(uint32_t i);

    uint32_t GetMethodsNum();

    PandaVector<EtsMethod *> GetMethods();

    PANDA_PUBLIC_API PandaVector<EtsMethod *> GetConstructors();

    template <class T>
    T GetStaticFieldPrimitive(EtsField *field)
    {
        return GetRuntimeClass()->GetFieldPrimitive<T>(*field->GetRuntimeField());
    }

    template <class T>
    T GetStaticFieldPrimitive(int32_t fieldOffset, bool isVolatile)
    {
        if (isVolatile) {
            return GetRuntimeClass()->GetFieldPrimitive<T, true>(fieldOffset);
        }
        return GetRuntimeClass()->GetFieldPrimitive<T, false>(fieldOffset);
    }

    template <class T>
    void SetStaticFieldPrimitive(EtsField *field, T value)
    {
        GetRuntimeClass()->SetFieldPrimitive<T>(*field->GetRuntimeField(), value);
    }

    template <class T>
    void SetStaticFieldPrimitive(int32_t fieldOffset, bool isVolatile, T value)
    {
        if (isVolatile) {
            GetRuntimeClass()->SetFieldPrimitive<T, true>(fieldOffset, value);
        }
        GetRuntimeClass()->SetFieldPrimitive<T, false>(fieldOffset, value);
    }

    PANDA_PUBLIC_API EtsObject *GetStaticFieldObject(EtsField *field);
    EtsObject *GetStaticFieldObject(int32_t fieldOffset, bool isVolatile);

    void SetStaticFieldObject(EtsField *field, EtsObject *value);
    void SetStaticFieldObject(int32_t fieldOffset, bool isVolatile, EtsObject *value);

    bool IsEtsObject()
    {
        return GetRuntimeClass()->IsObjectClass();
    }

    bool IsPrimitive() const
    {
        return GetRuntimeClass()->IsPrimitive();
    }

    bool IsAbstract()
    {
        return GetRuntimeClass()->IsAbstract();
    }

    bool IsPublic() const
    {
        return GetRuntimeClass()->IsPublic();
    }

    bool IsFinal() const
    {
        return GetRuntimeClass()->IsFinal();
    }

    bool IsAnnotation() const;
    bool IsEnum() const;
    bool IsStringClass() const;
    bool IsFunctionalClass() const;
    bool IsUnionClass() const;
    bool IsUndefined() const;
    bool IsClassClass() const;
    bool IsInterface() const;
    bool IsArrayClass() const;
    bool IsTupleClass() const;
    bool IsBoxedClass() const;
    bool IsJSValueClass() const;

    static bool IsInSamePackage(std::string_view className1, std::string_view className2);

    bool IsInSamePackage(EtsClass *that);

    PandaVector<EtsClass *> GetInterfaces() const;

    uint32_t GetComponentSize() const
    {
        return GetRuntimeClass()->GetComponentSize();
    }

    panda_file::Type GetType() const
    {
        return GetRuntimeClass()->GetType();
    }

    bool IsSubClass(EtsClass *cls)
    {
        return GetRuntimeClass()->IsSubClassOf(cls->GetRuntimeClass());
    }

    bool IsAssignableFrom(EtsClass *clazz) const
    {
        return GetRuntimeClass()->IsAssignableFrom(clazz->GetRuntimeClass());
    }

    bool IsGenerated() const
    {
        return IsArrayClass() || IsPrimitive();
    }

    bool CanAccess(EtsClass *that)
    {
        return that->IsPublic() || IsInSamePackage(that);
    }

    EtsMethod *ResolveVirtualMethod(const EtsMethod *method) const;

    template <class Callback>
    void EnumerateMethods(const Callback &callback)
    {
        for (auto &method : GetRuntimeClass()->GetMethods()) {
            bool finished = callback(reinterpret_cast<EtsMethod *>(&method));
            if (finished) {
                break;
            }
        }
    }

    template <class Callback>
    void EnumerateDirectInterfaces(const Callback &callback)
    {
        for (Class *runtimeInterface : GetRuntimeClass()->GetInterfaces()) {
            EtsClass *interface = EtsClass::FromRuntimeClass(runtimeInterface);
            bool finished = callback(interface);
            if (finished) {
                break;
            }
        }
    }

    void GetInterfaces(PandaUnorderedSet<EtsClass *> &ifaces, EtsClass *iface);

    template <class Callback>
    void EnumerateInterfaces(const Callback &callback)
    {
        PandaUnorderedSet<EtsClass *> ifaces;
        GetInterfaces(ifaces, this);
        for (auto iface : ifaces) {
            bool finished = callback(iface);
            if (finished) {
                break;
            }
        }
    }

    template <class Callback>
    void EnumerateBaseClasses(const Callback &callback)
    {
        PandaVector<EtsClass *> inherChain;
        auto curClass = this;
        while (curClass != nullptr) {
            inherChain.push_back(curClass);
            curClass = curClass->GetBase();
        }
        for (auto i = inherChain.rbegin(); i != inherChain.rend(); i++) {
            bool finished = callback(*i);
            if (finished) {
                break;
            }
        }
    }

    ClassLinkerContext *GetClassLoader() const
    {
        return GetRuntimeClass()->GetLoadContext();
    }

    Class *GetRuntimeClass()
    {
        return &klass_;
    }

    const Class *GetRuntimeClass() const
    {
        return &klass_;
    }

    EtsObject *AsObject()
    {
        return reinterpret_cast<EtsObject *>(this);
    }

    const EtsObject *AsObject() const
    {
        return reinterpret_cast<const EtsObject *>(this);
    }

    bool IsInitialized() const
    {
        return GetRuntimeClass()->IsInitialized();
    }

    EtsType GetEtsType();

    static size_t GetSize(uint32_t klassSize)
    {
        return GetRuntimeClassOffset() + klassSize;
    }

    static constexpr size_t GetRuntimeClassOffset()
    {
        return MEMBER_OFFSET(EtsClass, klass_);
    }

    static constexpr size_t GetHeaderOffset()
    {
        return MEMBER_OFFSET(EtsClass, header_);
    }

    static EtsClass *FromRuntimeClass(const Class *cls)
    {
        ASSERT(cls != nullptr);
        return reinterpret_cast<EtsClass *>(reinterpret_cast<uintptr_t>(cls) - GetRuntimeClassOffset());
    }

    static EtsClass *FromClassObject(ObjectHeader *obj)
    {
        ASSERT(obj->ClassAddr<Class>()->IsClassClass());
        return reinterpret_cast<EtsClass *>(ToUintPtr(obj) - GetHeaderOffset());
    }

    static EtsClass *GetPrimitiveClass(EtsString *primitiveName);

    void Initialize(EtsClass *superClass, uint16_t accessFlags, bool isPrimitiveType);

    void SetComponentType(EtsClass *componentType);

    EtsClass *GetComponentType() const;

    void SetName(EtsString *name);

    bool CompareAndSetName(EtsString *oldName, EtsString *newName);

    EtsString *GetName();
    static EtsString *CreateEtsClassName([[maybe_unused]] const char *descriptor);

    void SetSuperClass(EtsClass *superClass)
    {
        ObjectHeader *obj = superClass != nullptr ? reinterpret_cast<ObjectHeader *>(superClass->AsObject()) : nullptr;
        GetObjectHeader()->SetFieldObject(GetSuperClassOffset(), obj);
    }

    EtsClass *GetSuperClass() const
    {
        return reinterpret_cast<EtsClass *>(GetObjectHeader()->GetFieldObject(GetSuperClassOffset()));
    }

    void SetFlags(uint32_t flags)
    {
        GetObjectHeader()->SetFieldPrimitive(GetFlagsOffset(), flags);
    }

    uint32_t GetFlags() const
    {
        return GetObjectHeader()->GetFieldPrimitive<uint32_t>(GetFlagsOffset());
    }

    void SetWeakReference();
    void SetFinalizeReference();
    void SetValueTyped();

    [[nodiscard]] bool IsWeakReference() const;
    [[nodiscard]] bool IsFinalizerReference() const;

    [[nodiscard]] bool IsValueTyped() const
    {
        return (GetFlags() & IS_VALUE_TYPED) != 0;
    }

    /// True if class inherited from Reference, false otherwise
    [[nodiscard]] bool IsReference() const;

    EtsClass() = delete;
    ~EtsClass() = delete;
    NO_COPY_SEMANTIC(EtsClass);
    NO_MOVE_SEMANTIC(EtsClass);

    static constexpr size_t GetNameOffset()
    {
        return MEMBER_OFFSET(EtsClass, name_);
    }

    static constexpr size_t GetSuperClassOffset()
    {
        return MEMBER_OFFSET(EtsClass, superClass_);
    }

    static constexpr size_t GetFlagsOffset()
    {
        return MEMBER_OFFSET(EtsClass, flags_);
    }

    static constexpr size_t GCRefFieldsOffset()
    {
        return GetHeaderOffset() + sizeof(header_);
    }

    static constexpr size_t GCRefFieldsNum()
    {
        return (GetFlagsOffset() - GCRefFieldsOffset()) / sizeof(ObjectPointerType);
    }

private:
    ObjectHeader *GetObjectHeader()
    {
        return &header_;
    }

    const ObjectHeader *GetObjectHeader() const
    {
        return &header_;
    }

    constexpr static uint32_t ETS_ACC_PRIMITIVE = 1U << 16U;
    /// Class is a WeakReference or successor of this class
    constexpr static uint32_t IS_WEAK_REFERENCE = 1U << 17U;
    /// Class is a FinalizerReference or successor of this class
    constexpr static uint32_t IS_FINALIZE_REFERENCE = 1U << 18U;
    constexpr static uint32_t IS_REFERENCE = IS_WEAK_REFERENCE | IS_FINALIZE_REFERENCE;

    constexpr static uint32_t IS_VALUE_TYPED = 1U << 19U;

    ark::ObjectHeader header_;  // EtsObject

    // ets.Class fields BEGIN
    // Reference fields START
    FIELD_UNUSED ObjectPointer<EtsString> name_;       // String
    FIELD_UNUSED ObjectPointer<EtsClass> superClass_;  // Class<? super T>
    // Reference fields END
    FIELD_UNUSED uint32_t flags_;
    // ets.Class fields END

    ark::Class klass_;
};

// Object header field must be first
static_assert(EtsClass::GetHeaderOffset() == 0);

// Klass field has variable size so it must be last
static_assert(EtsClass::GetRuntimeClassOffset() + sizeof(ark::Class) == sizeof(EtsClass));

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_CLASS_H_
