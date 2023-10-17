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

namespace panda::ets {

enum class AccessLevel { PUBLIC, PROTECTED, DEFAULT, PRIVATE };

class EtsMethod;
class EtsObject;
class EtsVoid;
class EtsString;
class EtsArray;
class EtsPromise;
class EtsTypeAPIField;
class EtsTypeAPIMethod;
class EtsTypeAPIParameter;

enum class EtsType;

class EtsClass {
public:
    // We shouldn't init header_ here - because it has been memset(0) in object allocation,
    // otherwise it may cause data race while visiting object's class concurrently in gc.
    void InitClass(const uint8_t *descriptor, uint32_t vtable_size, uint32_t imt_size, uint32_t klass_size)
    {
        new (&klass_) panda::Class(descriptor, panda_file::SourceLang::ETS, vtable_size, imt_size, klass_size);
    }

    const char *GetDescriptor() const
    {
        return reinterpret_cast<const char *>(GetRuntimeClass()->GetDescriptor());
    }

    EtsClass *GetBase()
    {
        if (IsInterface()) {
            return nullptr;
        }
        auto *base = GetRuntimeClass()->GetBase();
        if (base == nullptr) {
            return nullptr;
        }
        return FromRuntimeClass(base);
    }

    uint32_t GetFieldsNumber();

    uint32_t GetStaticFieldsNumber()
    {
        return GetRuntimeClass()->GetStaticFields().size();
    }

    uint32_t GetInstanceFieldsNumber()
    {
        return GetRuntimeClass()->GetInstanceFields().size();
    }

    PandaVector<EtsField *> GetFields();

    EtsField *GetFieldByIndex(uint32_t i);

    EtsField *GetFieldIDByName(const char *name, const char *sig = nullptr)
    {
        auto u8name = reinterpret_cast<const uint8_t *>(name);
        auto *field = reinterpret_cast<EtsField *>(GetRuntimeClass()->GetInstanceFieldByName(u8name));

        if (sig != nullptr && field != nullptr) {
            if (strcmp(field->GetTypeDescriptor(), sig) != 0) {
                return nullptr;
            }
        }

        return field;
    }

    uint32_t GetFieldIndexByName(const char *name)
    {
        auto u8name = reinterpret_cast<const uint8_t *>(name);
        auto fields = GetRuntimeClass()->GetFields();
        panda_file::File::StringData sd = {static_cast<uint32_t>(panda::utf::MUtf8ToUtf16Size(u8name)), u8name};
        for (uint32_t i = 0; i < GetFieldsNumber(); i++) {
            if (fields[i].GetName() == sd) {
                return i;
            }
        }
        return -1;
    }

    EtsField *GetStaticFieldIDByName(const char *name, const char *sig = nullptr)
    {
        auto u8name = reinterpret_cast<const uint8_t *>(name);
        auto *field = reinterpret_cast<EtsField *>(GetRuntimeClass()->GetStaticFieldByName(u8name));

        if (sig != nullptr && field != nullptr) {
            if (strcmp(field->GetTypeDescriptor(), sig) != 0) {
                return nullptr;
            }
        }

        return field;
    }

    EtsField *GetDeclaredFieldIDByName(const char *name)
    {
        return reinterpret_cast<EtsField *>(
            GetRuntimeClass()->FindDeclaredField([name](const panda::Field &field) -> bool {
                auto *jfield = EtsField::FromRuntimeField(&field);
                return ::strcmp(jfield->GetName(), name) == 0;
            }));
    }

    EtsField *GetFieldIDByOffset(uint32_t field_offset)
    {
        auto pred = [field_offset](const panda::Field &f) { return f.GetOffset() == field_offset; };
        return reinterpret_cast<EtsField *>(GetRuntimeClass()->FindInstanceField(pred));
    }

    EtsField *GetStaticFieldIDByOffset(uint32_t field_offset)
    {
        auto pred = [field_offset](const panda::Field &f) { return f.GetOffset() == field_offset; };
        return reinterpret_cast<EtsField *>(GetRuntimeClass()->FindStaticField(pred));
    }

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

    PandaVector<EtsMethod *> GetConstructors();

    template <class T>
    T GetStaticFieldPrimitive(EtsField *field)
    {
        return GetRuntimeClass()->GetFieldPrimitive<T>(*field->GetRuntimeField());
    }

    template <class T>
    T GetStaticFieldPrimitive(int32_t field_offset, bool is_volatile)
    {
        if (is_volatile) {
            return GetRuntimeClass()->GetFieldPrimitive<T, true>(field_offset);
        }
        return GetRuntimeClass()->GetFieldPrimitive<T, false>(field_offset);
    }

    template <class T>
    void SetStaticFieldPrimitive(EtsField *field, T value)
    {
        GetRuntimeClass()->SetFieldPrimitive<T>(*field->GetRuntimeField(), value);
    }

    template <class T>
    void SetStaticFieldPrimitive(int32_t field_offset, bool is_volatile, T value)
    {
        if (is_volatile) {
            GetRuntimeClass()->SetFieldPrimitive<T, true>(field_offset, value);
        }
        GetRuntimeClass()->SetFieldPrimitive<T, false>(field_offset, value);
    }

    EtsObject *GetStaticFieldObject(EtsField *field)
    {
        return reinterpret_cast<EtsObject *>(GetRuntimeClass()->GetFieldObject(*field->GetRuntimeField()));
    }

    EtsObject *GetStaticFieldObject(int32_t field_offset, bool is_volatile)
    {
        if (is_volatile) {
            return reinterpret_cast<EtsObject *>(GetRuntimeClass()->GetFieldObject<true>(field_offset));
        }
        return reinterpret_cast<EtsObject *>(GetRuntimeClass()->GetFieldObject<false>(field_offset));
    }

    void SetStaticFieldObject(EtsField *field, EtsObject *value)
    {
        GetRuntimeClass()->SetFieldObject(*field->GetRuntimeField(), reinterpret_cast<ObjectHeader *>(value));
    }

    void SetStaticFieldObject(int32_t field_offset, bool is_volatile, EtsObject *value)
    {
        if (is_volatile) {
            GetRuntimeClass()->SetFieldObject<true>(field_offset, reinterpret_cast<ObjectHeader *>(value));
        }
        GetRuntimeClass()->SetFieldObject<false>(field_offset, reinterpret_cast<ObjectHeader *>(value));
    }

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

    bool IsAnnotation() const
    {
        return GetRuntimeClass()->IsAnnotation();
    }

    bool IsEnum() const
    {
        return GetRuntimeClass()->IsEnum();
    }

    static bool IsInSamePackage(std::string_view class_name1, std::string_view class_name2);

    bool IsInSamePackage(EtsClass *that);

    bool IsStringClass() const
    {
        return GetRuntimeClass()->IsStringClass();
    }

    bool IsFunctionClass() const
    {
        // TODO(petr-shumilov): Make more clear
        return !GetRuntimeClass()->IsPrimitive() && GetRuntimeClass()->GetName().rfind(LAMBDA_PREFIX, 0) == 0;
    }

    bool IsUnionClass() const
    {
        // TODO(petr-shumilov): Not implemented
        return false;
    }

    bool IsUndefined() const
    {
        // TODO(petr-shumilov): Not implemented
        return false;
    }

    bool IsClassClass();

    bool IsInterface() const
    {
        return GetRuntimeClass()->IsInterface();
    }

    bool IsArrayClass() const
    {
        return GetRuntimeClass()->IsArrayClass();
    }

    bool IsBoxedClass() const
    {
        auto type_desc = GetDescriptor();
        return (type_desc == panda::ets::panda_file_items::class_descriptors::BOX_BOOLEAN ||
                type_desc == panda::ets::panda_file_items::class_descriptors::BOX_BYTE ||
                type_desc == panda::ets::panda_file_items::class_descriptors::BOX_CHAR ||
                type_desc == panda::ets::panda_file_items::class_descriptors::BOX_SHORT ||
                type_desc == panda::ets::panda_file_items::class_descriptors::BOX_INT ||
                type_desc == panda::ets::panda_file_items::class_descriptors::BOX_LONG ||
                type_desc == panda::ets::panda_file_items::class_descriptors::BOX_FLOAT ||
                type_desc == panda::ets::panda_file_items::class_descriptors::BOX_DOUBLE);
    }

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
    void EnumerateInterfaces(const Callback &callback)
    {
        for (Class *runtime_interface : GetRuntimeClass()->GetInterfaces()) {
            EtsClass *interface = EtsClass::FromRuntimeClass(runtime_interface);
            bool finished = callback(interface);
            if (finished) {
                break;
            }
        }
    }

    template <class Callback>
    void EnumerateBaseClasses(const Callback &callback)
    {
        PandaVector<EtsClass *> inher_chain;
        auto cur_class = this;
        while (cur_class != nullptr) {
            inher_chain.push_back(cur_class);
            cur_class = cur_class->GetBase();
        }
        for (auto i = inher_chain.rbegin(); i != inher_chain.rend(); i++) {
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

    static size_t GetSize(uint32_t klass_size)
    {
        return GetRuntimeClassOffset() + klass_size;
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

    static EtsClass *GetPrimitiveClass(EtsString *primitive_name);

    void Initialize(EtsArray *if_table, EtsClass *super_class, uint16_t access_flags, bool is_primitive_type)
    {
        ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();

        SetIfTable(if_table);
        SetName(nullptr);
        SetSuperClass(super_class);

        uint32_t flags = access_flags;
        if (is_primitive_type) {
            flags |= ETS_ACC_PRIMITIVE;
        }

        if (super_class != nullptr) {
            if (super_class->IsSoftReference()) {
                flags |= IS_SOFT_REFERENCE;
            } else if (super_class->IsWeakReference()) {
                flags |= IS_WEAK_REFERENCE;
            } else if (super_class->IsPhantomReference()) {
                flags |= IS_PHANTOM_REFERENCE;
            }
            if (super_class->IsFinalizerReference()) {
                flags |= IS_FINALIZE_REFERENCE;
            }
            if (super_class->IsFinalizable()) {
                flags |= IS_CLASS_FINALIZABLE;
            }
        }
        if ((flags & IS_CLASS_FINALIZABLE) == 0) {
            if (IsClassFinalizable(this)) {
                flags |= IS_CLASS_FINALIZABLE;
            }
        }
        SetFlags(flags);
    }

    void SetComponentType(EtsClass *component_type)
    {
        if (component_type == nullptr) {
            GetRuntimeClass()->SetComponentType(nullptr);
            return;
        }
        GetRuntimeClass()->SetComponentType(component_type->GetRuntimeClass());
    }

    EtsClass *GetComponentType() const
    {
        panda::Class *component_type = GetRuntimeClass()->GetComponentType();
        if (component_type == nullptr) {
            return nullptr;
        }
        return FromRuntimeClass(component_type);
    }

    void SetIfTable(EtsArray *array)
    {
        GetObjectHeader()->SetFieldObject(GetIfTableOffset(), reinterpret_cast<ObjectHeader *>(array));
    }

    void SetName(EtsString *name)
    {
        GetObjectHeader()->SetFieldObject(GetNameOffset(), reinterpret_cast<ObjectHeader *>(name));
    }

    bool CompareAndSetName(EtsString *old_name, EtsString *new_name)
    {
        return GetObjectHeader()->CompareAndSetFieldObject(GetNameOffset(), reinterpret_cast<ObjectHeader *>(old_name),
                                                           reinterpret_cast<ObjectHeader *>(new_name),
                                                           std::memory_order::memory_order_seq_cst, true);
    }

    EtsString *GetName();
    static EtsString *CreateEtsClassName([[maybe_unused]] const char *descriptor);

    void SetSuperClass(EtsClass *super_class)
    {
        ObjectHeader *obj =
            super_class != nullptr ? reinterpret_cast<ObjectHeader *>(super_class->AsObject()) : nullptr;
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

    /// Return true if given class has .finalize() method, false otherwise
    static bool IsClassFinalizable(EtsClass *klass);

    void SetSoftReference();
    void SetWeakReference();
    void SetFinalizeReference();
    void SetPhantomReference();
    void SetFinalizable();

    [[nodiscard]] bool IsSoftReference() const;
    [[nodiscard]] bool IsWeakReference() const;
    [[nodiscard]] bool IsFinalizerReference() const;
    [[nodiscard]] bool IsPhantomReference() const;
    [[nodiscard]] bool IsFinalizable() const;

    /// True if class inherited from Reference, false otherwise
    [[nodiscard]] bool IsReference() const;

    EtsClass() = delete;
    ~EtsClass() = delete;
    NO_COPY_SEMANTIC(EtsClass);
    NO_MOVE_SEMANTIC(EtsClass);

    static constexpr size_t GetIfTableOffset()
    {
        return MEMBER_OFFSET(EtsClass, if_table_);
    }

    static constexpr size_t GetNameOffset()
    {
        return MEMBER_OFFSET(EtsClass, name_);
    }

    static constexpr size_t GetSuperClassOffset()
    {
        return MEMBER_OFFSET(EtsClass, super_class_);
    }

    static constexpr size_t GetFlagsOffset()
    {
        return MEMBER_OFFSET(EtsClass, flags_);
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
    /// Class is a SoftReference or successor of this class
    constexpr static uint32_t IS_SOFT_REFERENCE = 1U << 18U;
    /// Class is a WeakReference or successor of this class
    constexpr static uint32_t IS_WEAK_REFERENCE = 1U << 19U;
    /// Class is a FinalizerReference or successor of this class
    constexpr static uint32_t IS_FINALIZE_REFERENCE = 1U << 20U;
    /// Class is a PhantomReference or successor of this class
    constexpr static uint32_t IS_PHANTOM_REFERENCE = 1U << 21U;
    /// Class is a Reference or successor of this class
    constexpr static uint32_t IS_REFERENCE =
        IS_SOFT_REFERENCE | IS_WEAK_REFERENCE | IS_FINALIZE_REFERENCE | IS_PHANTOM_REFERENCE;
    /// Class override Object.finalize() or any of his ancestors, and implementation is not trivial.
    constexpr static uint32_t IS_CLASS_FINALIZABLE = 1U << 22U;

    panda::ObjectHeader header_;  // EtsObject

    // ets.Class fields BEGIN
    FIELD_UNUSED ObjectPointer<EtsArray> if_table_;     // Class[]
    FIELD_UNUSED ObjectPointer<EtsString> name_;        // String
    FIELD_UNUSED ObjectPointer<EtsClass> super_class_;  // Class<? super T>

    FIELD_UNUSED uint32_t flags_;
    // ets.Class fields END

    panda::Class klass_;
};

// Object header field must be first
static_assert(EtsClass::GetHeaderOffset() == 0);

// Klass field has variable size so it must be last
static_assert(EtsClass::GetRuntimeClassOffset() + sizeof(panda::Class) == sizeof(EtsClass));

}  // namespace panda::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_CLASS_H_
