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
// All common ObjectHeader methods can be found here:
// - Get/Set Mark or Class word
// - Get size of the object header and an object itself
// - Get/Generate an object hash
// Methods, specific for Class word:
// - Get different object fields
// - Return object type
// - Verify object
// - Is it a subclass of not
// - Get field addr
// Methods, specific for Mark word:
// - Object locked/unlocked
// - Marked for GC or not
// - Monitor functions (get monitor, notify, notify all, wait)
// - Forwarded or not
#ifndef PANDA_RUNTIME_OBJECT_HEADER_H_
#define PANDA_RUNTIME_OBJECT_HEADER_H_

#include <atomic>
#include <ctime>

#include "runtime/mem/lock_config_helper.h"
#include "runtime/include/class_helper.h"
#include "runtime/mark_word.h"

namespace panda {

namespace object_header_traits {

constexpr const uint32_t LINEAR_X = 1103515245U;
constexpr const uint32_t LINEAR_Y = 12345U;
constexpr const uint32_t LINEAR_SEED = 987654321U;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static auto HASH_SEED = std::atomic<uint32_t>(LINEAR_SEED + std::time(nullptr));

}  // namespace object_header_traits

class BaseClass;
class Class;
class Field;
class ManagedThread;

class PANDA_PUBLIC_API ObjectHeader {
public:
    // Simple getters and setters for Class and Mark words.
    // Use it only in single thread
    inline MarkWord GetMark() const
    {
        return *(const_cast<MarkWord *>(reinterpret_cast<const MarkWord *>(&mark_word_)));
    }
    inline void SetMark(volatile MarkWord mark_word)
    {
        mark_word_ = mark_word.Value();
    }

    inline MarkWord AtomicGetMark() const
    {
        auto ptr = const_cast<MarkWord *>(reinterpret_cast<const MarkWord *>(&mark_word_));
        auto atomic_ptr = reinterpret_cast<std::atomic<MarkWord> *>(ptr);
        // Atomic with seq_cst order reason: data race with markWord_ with requirement for sequentially consistent order
        // where threads observe all modifications in the same order
        return atomic_ptr->load(std::memory_order_seq_cst);
    }

    inline void SetClass(BaseClass *klass)
    {
        static_assert(sizeof(ClassHelper::ClassWordSize) == sizeof(ObjectPointerType));
        // Atomic with release order reason: data race with classWord_ with dependecies on writes before the store which
        // should become visible acquire
        reinterpret_cast<std::atomic<ClassHelper::ClassWordSize> *>(&class_word_)
            ->store(static_cast<ClassHelper::ClassWordSize>(ToObjPtrType(klass)), std::memory_order_release);
        ASSERT(ClassAddr<BaseClass>() == klass);
    }

    template <typename T>
    inline T *ClassAddr() const
    {
        auto ptr = const_cast<ClassHelper::ClassWordSize *>(&class_word_);
        // Atomic with acquire order reason: data race with classWord_ with dependecies on reads after the load which
        // should become visible
        return reinterpret_cast<T *>(
            reinterpret_cast<std::atomic<ClassHelper::ClassWordSize> *>(ptr)->load(std::memory_order_acquire));
    }

    template <typename T>
    inline T *NotAtomicClassAddr() const
    {
        return reinterpret_cast<T *>(*const_cast<ClassHelper::ClassWordSize *>(&class_word_));
    }

    // Generate hash value for an object.
    static inline uint32_t GenerateHashCode()
    {
        uint32_t ex_val;
        uint32_t n_val;
        do {
            // Atomic with relaxed order reason: data race with hash_seed with no synchronization or ordering
            // constraints imposed on other reads or writes
            ex_val = object_header_traits::HASH_SEED.load(std::memory_order_relaxed);
            n_val = ex_val * object_header_traits::LINEAR_X + object_header_traits::LINEAR_Y;
        } while (!object_header_traits::HASH_SEED.compare_exchange_weak(ex_val, n_val, std::memory_order_relaxed) ||
                 (ex_val & MarkWord::HASH_MASK) == 0);
        return ex_val & MarkWord::HASH_MASK;
    }

    // Get Hash value for an object.
    template <MTModeT MT_MODE>
    uint32_t GetHashCode();
    uint32_t GetHashCodeFromMonitor(Monitor *monitor_p);

    // Size of object header
    static constexpr size_t ObjectHeaderSize()
    {
        return sizeof(ObjectHeader);
    }

    static constexpr size_t GetClassOffset()
    {
        return MEMBER_OFFSET(ObjectHeader, class_word_);
    }

    static constexpr size_t GetMarkWordOffset()
    {
        return MEMBER_OFFSET(ObjectHeader, mark_word_);
    }

    // Garbage collection method
    template <bool ATOMIC_FLAG = true>
    inline bool IsMarkedForGC() const
    {
        if constexpr (!ATOMIC_FLAG) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            return GetMark().IsMarkedForGC();
        }
        return AtomicGetMark().IsMarkedForGC();
    }
    template <bool ATOMIC_FLAG = true>
    inline void SetMarkedForGC()
    {
        if constexpr (!ATOMIC_FLAG) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            SetMark(GetMark().SetMarkedForGC());
            return;
        }
        bool res;
        MarkWord word = AtomicGetMark();
        do {
            res = AtomicSetMark<false>(word, word.SetMarkedForGC());
        } while (!res);
    }
    template <bool ATOMIC_FLAG = true>
    inline void SetUnMarkedForGC()
    {
        if constexpr (!ATOMIC_FLAG) {  // NOLINTNEXTLINE(readability-braces-around-statements)
            SetMark(GetMark().SetUnMarkedForGC());
            return;
        }
        bool res;
        MarkWord word = AtomicGetMark();
        do {
            res = AtomicSetMark<false>(word, word.SetUnMarkedForGC());
        } while (!res);
    }
    inline bool IsForwarded() const
    {
        return AtomicGetMark().GetState() == MarkWord::ObjectState::STATE_GC;
    }

    // Type test methods
    inline bool IsInstance() const;

    // Get field address in Class
    inline void *FieldAddr(int offset) const;

    template <bool STRONG = true>
    bool AtomicSetMark(MarkWord &old_mark_word, MarkWord new_mark_word)
    {
        // This is the way to operate with casting MarkWordSize <-> MarkWord and atomics
        auto ptr = reinterpret_cast<MarkWord *>(&mark_word_);
        auto atomic_ptr = reinterpret_cast<std::atomic<MarkWord> *>(ptr);
        // NOLINTNEXTLINE(readability-braces-around-statements, hicpp-braces-around-statements)
        if constexpr (STRONG) {  // NOLINT(bugprone-suspicious-semicolon)
            return atomic_ptr->compare_exchange_strong(old_mark_word, new_mark_word);
        }
        // CAS weak may return false results, but is more efficient, use it only in loops
        return atomic_ptr->compare_exchange_weak(old_mark_word, new_mark_word);
    }

    // Accessors to typical Class types

    template <class T, bool IS_VOLATILE = false>
    T GetFieldPrimitive(size_t offset) const;

    template <class T, bool IS_VOLATILE = false>
    void SetFieldPrimitive(size_t offset, T value);

    template <bool IS_VOLATILE = false, bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetFieldObject(int offset) const;

    template <bool IS_VOLATILE = false, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void SetFieldObject(size_t offset, ObjectHeader *value);

    template <class T>
    T GetFieldPrimitive(const Field &field) const;

    template <class T>
    void SetFieldPrimitive(const Field &field, T value);

    template <bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetFieldObject(const Field &field) const;

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void SetFieldObject(const Field &field, ObjectHeader *value);

    // Pass thread parameter to speed up interpreter
    template <bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetFieldObject(const ManagedThread *thread, const Field &field);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void SetFieldObject(const ManagedThread *thread, const Field &field, ObjectHeader *value);

    template <bool IS_VOLATILE = false, bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void SetFieldObject(const ManagedThread *thread, size_t offset, ObjectHeader *value);

    template <class T>
    T GetFieldPrimitive(size_t offset, std::memory_order memory_order) const;

    template <class T>
    void SetFieldPrimitive(size_t offset, T value, std::memory_order memory_order);

    template <bool NEED_READ_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetFieldObject(size_t offset, std::memory_order memory_order) const;

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    void SetFieldObject(size_t offset, ObjectHeader *value, std::memory_order memory_order);

    template <typename T>
    bool CompareAndSetFieldPrimitive(size_t offset, T old_value, T new_value, std::memory_order memory_order,
                                     bool strong);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    bool CompareAndSetFieldObject(size_t offset, ObjectHeader *old_value, ObjectHeader *new_value,
                                  std::memory_order memory_order, bool strong);

    template <typename T>
    T CompareAndExchangeFieldPrimitive(size_t offset, T old_value, T new_value, std::memory_order memory_order,
                                       bool strong);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *CompareAndExchangeFieldObject(size_t offset, ObjectHeader *old_value, ObjectHeader *new_value,
                                                std::memory_order memory_order, bool strong);

    template <typename T>
    T GetAndSetFieldPrimitive(size_t offset, T value, std::memory_order memory_order);

    template <bool NEED_WRITE_BARRIER = true, bool IS_DYN = false>
    ObjectHeader *GetAndSetFieldObject(size_t offset, ObjectHeader *value, std::memory_order memory_order);

    template <typename T>
    T GetAndAddFieldPrimitive(size_t offset, T value, std::memory_order memory_order);

    template <typename T>
    T GetAndBitwiseOrFieldPrimitive(size_t offset, T value, std::memory_order memory_order);

    template <typename T>
    T GetAndBitwiseAndFieldPrimitive(size_t offset, T value, std::memory_order memory_order);

    template <typename T>
    T GetAndBitwiseXorFieldPrimitive(size_t offset, T value, std::memory_order memory_order);

    /*
     * Is the object is an instance of specified class.
     * Object of type O is instance of type T if O is the same as T or is subtype of T. For arrays T should be a root
     * type in type hierarchy or T is such array that O array elements are the same or subtype of T array elements.
     */
    inline bool IsInstanceOf(const Class *klass) const;

    // Verification methods
    static void Verify(ObjectHeader *object_header);

    static ObjectHeader *Create(BaseClass *klass);
    static ObjectHeader *Create(ManagedThread *thread, BaseClass *klass);

    static ObjectHeader *CreateNonMovable(BaseClass *klass);

    static ObjectHeader *Clone(ObjectHeader *src);

    static ObjectHeader *ShallowCopy(ObjectHeader *src);

    size_t ObjectSize() const;

    template <LangTypeT LANG>
    size_t ObjectSize(BaseClass *base_klass) const
    {
        if constexpr (LANG == LangTypeT::LANG_TYPE_DYNAMIC) {
            return ObjectSizeDyn(base_klass);
        } else {
            static_assert(LANG == LangTypeT::LANG_TYPE_STATIC);
            return ObjectSizeStatic(base_klass);
        }
    }

private:
    uint32_t GetHashCodeMTSingle();
    uint32_t GetHashCodeMTMulti();
    size_t ObjectSizeDyn(BaseClass *base_klass) const;
    size_t ObjectSizeStatic(BaseClass *base_klass) const;
    MarkWord::MarkWordSize mark_word_;
    ClassHelper::ClassWordSize class_word_;

    /**
     * Allocates memory for the Object. No ctor is called.
     * @param klass - class of Object
     * @param non_movable - if true, object will be allocated in non-movable space
     * @return pointer to the created Object
     */
    static ObjectHeader *CreateObject(BaseClass *klass, bool non_movable);
    static ObjectHeader *CreateObject(ManagedThread *thread, BaseClass *klass, bool non_movable);
};

constexpr uint32_t OBJECT_HEADER_CLASS_OFFSET = 4U;
static_assert(OBJECT_HEADER_CLASS_OFFSET == panda::ObjectHeader::GetClassOffset());

}  // namespace panda

#endif  // PANDA_RUNTIME_OBJECT_HEADER_H_
