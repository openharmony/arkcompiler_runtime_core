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
#ifndef PANDA_RUNTIME_CLASS_H_
#define PANDA_RUNTIME_CLASS_H_

#include <securec.h>
#include <atomic>
#include <cstdint>
#include <iostream>
#include <memory>

#include "libpandafile/file.h"
#include "libpandafile/file_items.h"
#include "runtime/include/field.h"
#include "runtime/include/itable.h"
#include "runtime/include/method.h"
#include "libpandabase/macros.h"

namespace ark {

class ClassLinkerContext;
class ManagedThread;
class ObjectHeader;

// NOTE (Artem Udovichenko): move BaseClass to another file but still have Class.h
class BaseClass {
public:
    static constexpr uint32_t DYNAMIC_CLASS = 1U;

public:
    explicit BaseClass(panda_file::SourceLang lang) : lang_(lang) {}

    ~BaseClass() = default;

    DEFAULT_COPY_SEMANTIC(BaseClass);
    DEFAULT_MOVE_SEMANTIC(BaseClass);

    uint32_t GetFlags() const
    {
        return flags_;
    }

    bool IsDynamicClass() const
    {
        return (flags_ & DYNAMIC_CLASS) != 0;
    }

    uint32_t GetObjectSize() const
    {
        return objectSize_;
    }

    void SetObjectSize(uint32_t size)
    {
        objectSize_ = size;
    }

    void SetManagedObject(ObjectHeader *obj)
    {
        managedObject_ = obj;
    }

    ObjectHeader *GetManagedObject() const
    {
        return managedObject_;
    }

    panda_file::SourceLang GetSourceLang() const
    {
        return lang_;
    }

    void SetSourceLang(panda_file::SourceLang lang)
    {
        lang_ = lang;
    }

    static constexpr uint32_t GetFlagsOffset()
    {
        return MEMBER_OFFSET(BaseClass, flags_);
    }
    static constexpr size_t GetManagedObjectOffset()
    {
        return MEMBER_OFFSET(BaseClass, managedObject_);
    }
    static constexpr size_t GetObjectSizeOffset()
    {
        return MEMBER_OFFSET(BaseClass, objectSize_);
    }

protected:
    void SetFlags(uint32_t flags)
    {
        flags_ = flags;
    }

private:
    uint32_t flags_ {0};
    // Size of the object of this class. In case of static classes it is 0
    // for abstract classes, interfaces and classes whose objects
    // have variable size (for example strings).
    uint32_t objectSize_ {0};
    ObjectHeader *managedObject_ {nullptr};
    panda_file::SourceLang lang_;
};

class Class : public BaseClass {
public:
    using UniqId = uint64_t;
    static constexpr uint32_t STRING_CLASS = DYNAMIC_CLASS << 1U;
    static constexpr uint32_t IS_CLONEABLE = STRING_CLASS << 1U;
    static constexpr size_t IMTABLE_SIZE = 32;

    enum {
        DUMPCLASSFULLDETAILS = 1,
        DUMPCLASSCLASSLODER = 2,
        DUMPCLASSINITIALIZED = 4,
    };

    enum class State : uint8_t { INITIAL = 0, LOADED, VERIFIED, INITIALIZING, ERRONEOUS, INITIALIZED };

    Class(const uint8_t *descriptor, panda_file::SourceLang lang, uint32_t vtableSize, uint32_t imtSize, uint32_t size);

    Class *GetBase() const
    {
        return base_;
    }

    void SetBase(Class *base)
    {
        base_ = base;
    }

    panda_file::File::EntityId GetFileId() const
    {
        return fileId_;
    }

    void SetFileId(panda_file::File::EntityId fileId)
    {
        fileId_ = fileId;
    }

    const panda_file::File *GetPandaFile() const
    {
        return pandaFile_;
    }

    void SetPandaFile(const panda_file::File *pf)
    {
        pandaFile_ = pf;
    }

    const uint8_t *GetDescriptor() const
    {
        return descriptor_;
    }

    void SetMethods(Span<Method> methods, uint32_t numVmethods, uint32_t numSmethods)
    {
        methods_ = methods.data();
        numMethods_ = numVmethods + numSmethods;
        numVmethods_ = numVmethods;
        numCopiedMethods_ = methods.size() - numMethods_;
    }

    Method *GetRawFirstMethodAddr() const
    {
        return methods_;
    }

    Span<Method> GetMethods() const
    {
        return {methods_, numMethods_};
    }

    Span<Method> GetMethodsWithCopied() const
    {
        return {methods_, numMethods_ + numCopiedMethods_};
    }

    Span<Method> GetStaticMethods() const
    {
        return GetMethods().SubSpan(numVmethods_);
    }

    Span<Method> GetVirtualMethods() const
    {
        return {methods_, numVmethods_};
    }

    Span<Method> GetCopiedMethods() const
    {
        Span<Method> res {methods_, numMethods_ + numCopiedMethods_};
        return res.SubSpan(numMethods_);
    }

    Span<Field> GetFields() const
    {
        return {fields_, numFields_};
    }

    Span<Field> GetStaticFields() const
    {
        return {fields_, numSfields_};
    }

    Span<Field> GetInstanceFields() const
    {
        return GetFields().SubSpan(numSfields_);
    }

    void SetFields(Span<Field> fields, uint32_t numSfields)
    {
        fields_ = fields.data();
        numFields_ = fields.size();
        numSfields_ = numSfields;
    }

    Span<Method *> GetVTable();

    Span<Method *const> GetVTable() const;

    Span<Class *> GetInterfaces() const
    {
        return {ifaces_, numIfaces_};
    }

    void SetInterfaces(Span<Class *> ifaces)
    {
        ifaces_ = ifaces.data();
        numIfaces_ = ifaces.size();
    }

    Span<Method *> GetIMT()
    {
        return GetClassSpan().SubSpan<Method *>(GetIMTOffset(), imtSize_);
    }

    Span<Method *const> GetIMT() const
    {
        return GetClassSpan().SubSpan<Method *const>(GetIMTOffset(), imtSize_);
    }

    uint32_t GetIMTableIndex(uint32_t methodOffset) const
    {
        ASSERT(imtSize_ != 0);
        return methodOffset % imtSize_;
    }

    uint32_t GetAccessFlags() const
    {
        return accessFlags_;
    }

    void SetAccessFlags(uint32_t accessFlags)
    {
        accessFlags_ = accessFlags;
    }

    bool IsPublic() const
    {
        return (accessFlags_ & ACC_PUBLIC) != 0;
    }

    bool IsProtected() const
    {
        return (accessFlags_ & ACC_PROTECTED) != 0;
    }

    bool IsPrivate() const
    {
        return (accessFlags_ & ACC_PRIVATE) != 0;
    }

    bool IsFinal() const
    {
        return (accessFlags_ & ACC_FINAL) != 0;
    }

    bool IsAnnotation() const
    {
        return (accessFlags_ & ACC_ANNOTATION) != 0;
    }

    bool IsEnum() const
    {
        return (accessFlags_ & ACC_ENUM) != 0;
    }

    uint32_t GetVTableSize() const
    {
        return vtableSize_;
    }

    uint32_t GetIMTSize() const
    {
        return imtSize_;
    }

    uint32_t GetClassSize() const
    {
        return classSize_;
    }

    uint32_t GetObjectSize() const
    {
        ASSERT(!IsVariableSize());
        return BaseClass::GetObjectSize();
    }

    void SetObjectSize(uint32_t size)
    {
        ASSERT(!IsVariableSize());
        BaseClass::SetObjectSize(size);
    }

    static uint32_t GetTypeSize(panda_file::Type type);
    uint32_t GetComponentSize() const;

    Class *GetComponentType() const
    {
        return componentType_;
    }

    void SetComponentType(Class *type)
    {
        componentType_ = type;
    }

    bool IsArrayClass() const
    {
        return componentType_ != nullptr;
    }

    bool IsObjectArrayClass() const
    {
        return IsArrayClass() && !componentType_->IsPrimitive();
    }

    bool IsStringClass() const
    {
        return (GetFlags() & STRING_CLASS) != 0;
    }

    void SetStringClass()
    {
        SetFlags(GetFlags() | STRING_CLASS);
    }

    void SetCloneable()
    {
        SetFlags(GetFlags() | IS_CLONEABLE);
    }

    bool IsVariableSize() const
    {
        return IsArrayClass() || IsStringClass();
    }

    size_t GetStaticFieldsOffset() const;

    panda_file::Type GetType() const
    {
        return type_;
    }

    void SetType(panda_file::Type type)
    {
        type_ = type;
    }

    bool IsPrimitive() const
    {
        return type_.IsPrimitive();
    }

    bool IsAbstract() const
    {
        return (accessFlags_ & ACC_ABSTRACT) != 0;
    }

    bool IsInterface() const
    {
        return (accessFlags_ & ACC_INTERFACE) != 0;
    }

    bool IsInstantiable() const
    {
        return (!IsPrimitive() && !IsAbstract() && !IsInterface()) || IsArrayClass();
    }

    bool IsObjectClass() const
    {
        return !IsPrimitive() && GetBase() == nullptr;
    }

    /**
     * Check if the object is Class instance
     * @return true if the object is Class instance
     */
    bool IsClassClass() const;

    bool IsSubClassOf(const Class *klass) const;

    /**
     * Check whether an instance of this class can be assigned from an instance of class "klass".
     * Object of type O is instance of type T if O is the same as T or is subtype of T. For arrays T should be a root
     * type in type hierarchy or T is such array that O array elements are the same or subtype of T array elements.
     */
    bool IsAssignableFrom(const Class *klass) const;

    bool IsProxy() const
    {
        return (GetAccessFlags() & ACC_PROXY) != 0;
    }

    bool Implements(const Class *klass) const;

    void SetITable(ITable itable)
    {
        itable_ = itable;
    }

    ITable GetITable() const
    {
        return itable_;
    }

    State GetState() const
    {
        return state_;
    }

    PANDA_PUBLIC_API void SetState(State state);

    bool IsVerified() const
    {
        return state_ >= State::VERIFIED;
    }

    bool IsInitializing() const
    {
        return state_ == State::INITIALIZING;
    }

    bool IsInitialized() const
    {
        return state_ == State::INITIALIZED;
    }

    bool IsLoaded() const
    {
        return state_ >= State::LOADED;
    }

    bool IsErroneous() const
    {
        return state_ == State::ERRONEOUS;
    }

    static constexpr uint32_t GetBaseOffset()
    {
        return MEMBER_OFFSET(Class, base_);
    }
    static constexpr uint32_t GetComponentTypeOffset()
    {
        return MEMBER_OFFSET(Class, componentType_);
    }
    static constexpr uint32_t GetTypeOffset()
    {
        return MEMBER_OFFSET(Class, type_);
    }
    static constexpr uint32_t GetStateOffset()
    {
        return MEMBER_OFFSET(Class, state_);
    }
    static constexpr uint32_t GetITableOffset()
    {
        return MEMBER_OFFSET(Class, itable_);
    }

    uint8_t GetInitializedValue()
    {
        return static_cast<uint8_t>(State::INITIALIZED);
    }

    bool IsVerifiedSuccess() const
    {
        return (IsVerified() && (!IsErroneous()));
    }
    void SetInitTid(uint32_t id)
    {
        initTid_ = id;
    }

    uint32_t GetInitTid() const
    {
        return initTid_;
    }

    static constexpr size_t GetVTableOffset();

    uint32_t GetNumVirtualMethods() const
    {
        return numVmethods_;
    }

    void SetNumVirtualMethods(uint32_t n)
    {
        numVmethods_ = n;
    }

    uint32_t GetNumCopiedMethods() const
    {
        return numCopiedMethods_;
    }

    void SetNumCopiedMethods(uint32_t n)
    {
        numCopiedMethods_ = n;
    }

    uint32_t GetNumStaticFields() const
    {
        return numSfields_;
    }

    void SetNumStaticFields(uint32_t n)
    {
        numSfields_ = n;
    }

    void SetHasDefaultMethods()
    {
        accessFlags_ |= ACC_HAS_DEFAULT_METHODS;
    }

    bool HasDefaultMethods() const
    {
        return (accessFlags_ & ACC_HAS_DEFAULT_METHODS) != 0;
    }

    size_t GetIMTOffset() const;

    PANDA_PUBLIC_API std::string GetName() const;

    ClassLinkerContext *GetLoadContext() const
    {
        ASSERT(loadContext_ != nullptr);
        return loadContext_;
    }

    void SetLoadContext(ClassLinkerContext *context)
    {
        ASSERT(context != nullptr);
        loadContext_ = context;
    }

    template <class Pred>
    Field *FindInstanceField(Pred pred) const;

    Field *FindInstanceFieldById(panda_file::File::EntityId id) const;

    template <class Pred>
    Field *FindStaticField(Pred pred) const;

    Field *FindStaticFieldById(panda_file::File::EntityId id) const;

    template <class Pred>
    Field *FindField(Pred pred) const;

    template <class Pred>
    Field *FindDeclaredField(Pred pred) const;

    Field *GetInstanceFieldByName(const uint8_t *mutf8Name) const;

    Field *GetStaticFieldByName(const uint8_t *mutf8Name) const;

    Field *GetDeclaredFieldByName(const uint8_t *mutf8Name) const;

    Method *GetVirtualInterfaceMethod(panda_file::File::EntityId id) const;

    Method *GetStaticInterfaceMethod(panda_file::File::EntityId id) const;

    Method *GetStaticClassMethod(panda_file::File::EntityId id) const;

    Method *GetVirtualClassMethod(panda_file::File::EntityId id) const;

    Method *GetDirectMethod(const uint8_t *mutf8Name, const Method::Proto &proto) const;

    Method *GetClassMethod(const uint8_t *mutf8Name, const Method::Proto &proto) const;

    Method *GetClassMethod(const panda_file::File::StringData &sd, const Method::Proto &proto) const;

    Method *GetStaticClassMethodByName(const panda_file::File::StringData &sd, const Method::Proto &proto) const;

    Method *GetVirtualClassMethodByName(const panda_file::File::StringData &sd, const Method::Proto &proto) const;

    Method *GetInterfaceMethod(const uint8_t *mutf8Name, const Method::Proto &proto) const;

    Method *GetInterfaceMethod(const panda_file::File::StringData &sd, const Method::Proto &proto) const;

    Method *GetStaticInterfaceMethodByName(const panda_file::File::StringData &sd, const Method::Proto &proto) const;

    Method *GetVirtualInterfaceMethodByName(const panda_file::File::StringData &sd, const Method::Proto &proto) const;

    Method *GetDirectMethod(const uint8_t *mutf8Name) const;

    Method *GetClassMethod(const uint8_t *mutf8Name) const;

    Method *GetInterfaceMethod(const uint8_t *mutf8Name) const;

    Method *ResolveVirtualMethod(const Method *method) const;

    template <class T, bool IS_VOLATILE = false>
    T GetFieldPrimitive(size_t offset) const;

    template <class T, bool IS_VOLATILE = false>
    void SetFieldPrimitive(size_t offset, T value);

    template <bool IS_VOLATILE = false, bool NEED_READ_BARRIER = true>
    ObjectHeader *GetFieldObject(size_t offset) const;

    template <bool IS_VOLATILE = false, bool NEED_WRITE_BARRIER = true>
    void SetFieldObject(size_t offset, ObjectHeader *value);

    template <class T>
    T GetFieldPrimitive(const Field &field) const;

    template <class T>
    void SetFieldPrimitive(const Field &field, T value);

    template <bool NEED_READ_BARRIER = true>
    ObjectHeader *GetFieldObject(const Field &field) const;

    template <bool NEED_WRITE_BARRIER = true>
    void SetFieldObject(const Field &field, ObjectHeader *value);

    // Pass thread parameter to speed up interpreter
    template <bool NEED_READ_BARRIER = true>
    ObjectHeader *GetFieldObject(ManagedThread *thread, const Field &field) const;

    template <bool NEED_WRITE_BARRIER = true>
    void SetFieldObject(ManagedThread *thread, const Field &field, ObjectHeader *value);

    template <class T>
    T GetFieldPrimitive(size_t offset, std::memory_order memoryOrder) const;

    template <class T>
    void SetFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <bool NEED_READ_BARRIER = true>
    ObjectHeader *GetFieldObject(size_t offset, std::memory_order memoryOrder) const;

    template <bool NEED_WRITE_BARRIER = true>
    void SetFieldObject(size_t offset, ObjectHeader *value, std::memory_order memoryOrder);

    template <typename T>
    bool CompareAndSetFieldPrimitive(size_t offset, T oldValue, T newValue, std::memory_order memoryOrder, bool strong);

    template <bool NEED_WRITE_BARRIER = true>
    bool CompareAndSetFieldObject(size_t offset, ObjectHeader *oldValue, ObjectHeader *newValue,
                                  std::memory_order memoryOrder, bool strong);

    template <typename T>
    T CompareAndExchangeFieldPrimitive(size_t offset, T oldValue, T newValue, std::memory_order memoryOrder,
                                       bool strong);

    template <bool NEED_WRITE_BARRIER = true>
    ObjectHeader *CompareAndExchangeFieldObject(size_t offset, ObjectHeader *oldValue, ObjectHeader *newValue,
                                                std::memory_order memoryOrder, bool strong);

    template <typename T>
    T GetAndSetFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <bool NEED_WRITE_BARRIER = true>
    ObjectHeader *GetAndSetFieldObject(size_t offset, ObjectHeader *value, std::memory_order memoryOrder);

    template <typename T>
    T GetAndAddFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <typename T>
    T GetAndBitwiseOrFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <typename T>
    T GetAndBitwiseAndFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    template <typename T>
    T GetAndBitwiseXorFieldPrimitive(size_t offset, T value, std::memory_order memoryOrder);

    void DumpClass(std::ostream &os, size_t flags);

    static UniqId CalcUniqId(const panda_file::File *file, panda_file::File::EntityId fileId);

    // for synthetic classes, like arrays
    static UniqId CalcUniqId(const uint8_t *descriptor);

    UniqId GetUniqId() const
    {
        // Atomic with acquire order reason: data race with uniq_id_ with dependecies on reads after the load which
        // should become visible
        auto id = uniqId_.load(std::memory_order_acquire);
        if (id == 0) {
            id = CalcUniqId();
            // Atomic with release order reason: data race with uniq_id_ with dependecies on writes before the store
            // which should become visible acquire
            uniqId_.store(id, std::memory_order_release);
        }
        return id;
    }

    void SetRefFieldsNum(uint32_t num, bool isStatic)
    {
        if (isStatic) {
            numRefsfields_ = num;
        } else {
            numReffields_ = num;
        }
    }

    void SetRefFieldsOffset(uint32_t offset, bool isStatic)
    {
        if (isStatic) {
            offsetRefsfields_ = offset;
        } else {
            offsetReffields_ = offset;
        }
    }

    void SetVolatileRefFieldsNum(uint32_t num, bool isStatic)
    {
        if (isStatic) {
            volatileRefsfieldsNum_ = num;
        } else {
            volatileReffieldsNum_ = num;
        }
    }

    template <bool IS_STATIC>
    uint32_t GetRefFieldsNum() const
    {
        return IS_STATIC ? numRefsfields_ : numReffields_;
    }

    template <bool IS_STATIC>
    uint32_t GetRefFieldsOffset() const
    {
        return IS_STATIC ? offsetRefsfields_ : offsetReffields_;
    }

    template <bool IS_STATIC>
    uint32_t GetVolatileRefFieldsNum() const
    {
        return IS_STATIC ? volatileRefsfieldsNum_ : volatileReffieldsNum_;
    }

    panda_file::File::EntityId ResolveClassIndex(panda_file::File::Index idx) const
    {
        return classIdx_[idx];
    }

    panda_file::File::EntityId ResolveMethodIndex(panda_file::File::Index idx) const
    {
        return methodIdx_[idx];
    }

    panda_file::File::EntityId ResolveFieldIndex(panda_file::File::Index idx) const
    {
        return fieldIdx_[idx];
    }

    Span<const panda_file::File::EntityId> GetClassIndex() const
    {
        return classIdx_;
    }

    void SetClassIndex(Span<const panda_file::File::EntityId> index)
    {
        classIdx_ = index;
    }

    Span<const panda_file::File::EntityId> GetMethodIndex() const
    {
        return methodIdx_;
    }

    void SetMethodIndex(Span<const panda_file::File::EntityId> index)
    {
        methodIdx_ = index;
    }

    Span<const panda_file::File::EntityId> GetFieldIndex() const
    {
        return fieldIdx_;
    }

    void SetFieldIndex(Span<const panda_file::File::EntityId> index)
    {
        fieldIdx_ = index;
    }

    static Class *FromClassObject(const ObjectHeader *obj);

    static size_t GetClassObjectSizeFromClass(Class *cls, panda_file::SourceLang lang);

    static inline constexpr size_t GetMethodsOffset()
    {
        return MEMBER_OFFSET(Class, methods_);
    }

    ~Class() = default;

    NO_COPY_SEMANTIC(Class);
    NO_MOVE_SEMANTIC(Class);

    static constexpr size_t ComputeClassSize(size_t vtableSize, size_t imtSize, size_t num8bitSfields,
                                             size_t num16bitSfields, size_t num32bitSfields, size_t num64bitSfields,
                                             size_t numRefSfields, size_t numTaggedSfields);

    Field *LookupFieldByName(panda_file::File::StringData name) const
    {
        for (auto &f : GetFields()) {
            if (name == f.GetName()) {
                return &f;
            }
        }
        return nullptr;
    }

    template <panda_file::Type::TypeId FIELD_TYPE>
    Method *LookupGetterByName(panda_file::File::StringData name) const
    {
        for (auto &m : GetMethods()) {
            if (name != m.GetName()) {
                continue;
            }
            auto retType = m.GetReturnType();
            if (retType.IsVoid()) {
                continue;
            }
            if (m.GetNumArgs() != 1) {
                continue;
            }
            if (!m.GetArgType(0).IsReference()) {
                continue;
            }
            if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
                if (retType.IsPrimitive()) {
                    continue;
                }
            } else {
                if (retType.IsReference()) {
                    continue;
                }
                if constexpr (panda_file::Type(FIELD_TYPE).GetBitWidth() == coretypes::INT64_BITS) {
                    if (retType.GetBitWidth() != coretypes::INT64_BITS) {
                        continue;
                    }
                } else {
                    if (retType.GetBitWidth() > coretypes::INT32_BITS) {
                        continue;
                    }
                }
            }
            return &m;
        }
        return nullptr;
    }

    template <panda_file::Type::TypeId FIELD_TYPE>
    Method *LookupSetterByName(panda_file::File::StringData name) const
    {
        for (auto &m : GetMethods()) {
            if (name != m.GetName()) {
                continue;
            }
            if (!m.GetReturnType().IsVoid()) {
                continue;
            }
            if (m.GetNumArgs() != 2U) {
                continue;
            }
            if (!m.GetArgType(0).IsReference()) {
                continue;
            }
            if constexpr (FIELD_TYPE == panda_file::Type::TypeId::REFERENCE) {
                if (m.GetArgType(1).IsPrimitive()) {
                    continue;
                }
            } else {
                auto arg1 = m.GetArgType(1);
                if (arg1.IsReference()) {
                    continue;
                }
                if constexpr (panda_file::Type(FIELD_TYPE).GetBitWidth() == coretypes::INT64_BITS) {
                    if (arg1.GetBitWidth() != coretypes::INT64_BITS) {
                        continue;
                    }
                } else {
                    if (arg1.GetBitWidth() > coretypes::INT32_BITS) {
                        continue;
                    }
                }
            }
            return &m;
        }
        return nullptr;
    }

private:
    static constexpr void Pad(size_t size, size_t *padding, size_t *n);

    enum class FindFilter { STATIC, INSTANCE, ALL, COPIED };

    template <FindFilter FILTER>
    Span<Field> GetFields() const;

    template <FindFilter FILTER, class Pred>
    Field *FindDeclaredField(Pred pred) const;

    template <FindFilter FILTER>
    Field *FindDeclaredField(panda_file::File::EntityId id) const;

    template <FindFilter FILTER, class Pred>
    Field *FindField(Pred pred) const;

    template <FindFilter FILTER>
    Span<Method> GetMethods() const;

    template <FindFilter FILTER, typename KeyComp, typename Key, typename... Pred>
    Method *FindDirectMethod(Key key, const Pred &...preds) const;

    template <FindFilter FILTER, typename KeyComp, typename Key, typename... Pred>
    Method *FindClassMethod(Key key, const Pred &...preds) const;

    template <Class::FindFilter FILTER, typename KeyComp, typename Key, typename... Pred>
    Method *FindInterfaceMethod(Key key, const Pred &...preds) const;

    Span<std::byte> GetClassSpan()
    {
        return Span(reinterpret_cast<std::byte *>(this), classSize_);
    }

    Span<const std::byte> GetClassSpan() const
    {
        return Span(reinterpret_cast<const std::byte *>(this), classSize_);
    }

private:
    Class *base_ {nullptr};
    const panda_file::File *pandaFile_ {nullptr};
    // Decscriptor is a valid MUTF8 string. See docs/file_format.md#typedescriptor for more information.
    const uint8_t *descriptor_;
    Method *methods_ {nullptr};
    Field *fields_ {nullptr};
    Class **ifaces_ {nullptr};

    panda_file::File::EntityId fileId_ {};
    uint32_t vtableSize_;
    uint32_t imtSize_;
    uint32_t classSize_;
    uint32_t accessFlags_ {0};

    uint32_t numMethods_ {0};
    uint32_t numVmethods_ {0};
    uint32_t numCopiedMethods_ {0};
    uint32_t numFields_ {0};
    uint32_t numSfields_ {0};
    uint32_t numIfaces_ {0};
    uint32_t initTid_ {0};

    ITable itable_;

    // For array types this field contains array's element size, for non-array type it should be zero.
    Class *componentType_ {nullptr};

    ClassLinkerContext *loadContext_ {nullptr};

    panda_file::Type type_ {panda_file::Type::TypeId::REFERENCE};
    std::atomic<State> state_;

    UniqId CalcUniqId() const;
    mutable std::atomic<UniqId> uniqId_ {0};

    uint32_t numReffields_ {0};      // instance reference fields num
    uint32_t numRefsfields_ {0};     // static reference fields num
    uint32_t offsetReffields_ {0};   // first instance reference fields offset in object layout
    uint32_t offsetRefsfields_ {0};  // first static reference fields offset in object layout
    uint32_t volatileReffieldsNum_ {0};
    uint32_t volatileRefsfieldsNum_ {0};

    Span<const panda_file::File::EntityId> classIdx_ {nullptr, nullptr};
    Span<const panda_file::File::EntityId> methodIdx_ {nullptr, nullptr};
    Span<const panda_file::File::EntityId> fieldIdx_ {nullptr, nullptr};
};

PANDA_PUBLIC_API std::ostream &operator<<(std::ostream &os, const Class::State &state);

}  // namespace ark

#endif  // PANDA_RUNTIME_CLASS_H_
