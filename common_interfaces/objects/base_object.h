/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_INTERFACES_OBJECTS_BASE_OBJECT_H
#define COMMON_INTERFACES_OBJECTS_BASE_OBJECT_H

<<<<<<< HEAD
#include <stdint.h>
#include <stdio.h>

#include "objects/base_class.h"
#include "base/common.h"
#include "objects/base_object_operator.h"
#include "objects/base_state_word.h"

namespace common {
=======
#include <cstdint>
#include <cstdio>

#include "base/common.h"
#include "objects/base_class.h"
#include "objects/base_object_operator.h"
#include "objects/base_state_word.h"

namespace panda {
>>>>>>> OpenHarmony_feature_20250328
class BaseObject {
public:
    BaseObject() : state_(0) {}
    static BaseObject *Cast(MAddress address)
    {
        return reinterpret_cast<BaseObject *>(address);
    }
<<<<<<< HEAD
    static void RegisterDynamic(BaseObjectOperatorInterfaces *dynamicObjOp);
    static void RegisterStatic(BaseObjectOperatorInterfaces *staticObjOp);
=======

    static void RegisterDynamic(BaseObjectOperatorInterfaces *dynamicObjOp);
    static PUBLIC_API void RegisterStatic(BaseObjectOperatorInterfaces *staticObjOp);
>>>>>>> OpenHarmony_feature_20250328

    inline size_t GetSize() const
    {
        if (IsForwarded()) {
            return GetSizeForwarded();
<<<<<<< HEAD
        } else {
=======
        } else {  // NOLINT(readability-else-after-return)
>>>>>>> OpenHarmony_feature_20250328
            return GetOperator()->GetSize(this);
        }
    }

    inline bool IsValidObject() const
    {
        return GetOperator()->IsValidObject(this);
    }

<<<<<<< HEAD
=======
    // Iterate object field, and skit the weak referent, ONLY used in interop.
    void ForEachRefFieldSkipReferent(const RefFieldVisitor &visitor)
    {
        GetOperator()->ForEachRefFieldSkipReferent(this, visitor);
    }

>>>>>>> OpenHarmony_feature_20250328
    void ForEachRefField(const RefFieldVisitor &visitor)
    {
        GetOperator()->ForEachRefField(this, visitor);
    }

<<<<<<< HEAD
=======
    void IterateXRef(const RefFieldVisitor &visitor)
    {
        GetOperator()->IterateXRef(this, visitor);
    }

>>>>>>> OpenHarmony_feature_20250328
    inline BaseObject *GetForwardingPointer() const
    {
        if (IsForwarded()) {
            return GetOperator()->GetForwardingPointer(this);
        }
        return nullptr;
    }

    inline void SetForwardingPointerAfterExclusive(BaseObject *fwdPtr)
    {
        CHECK_CC(IsForwarding());
        GetOperator()->SetForwardingPointerAfterExclusive(this, fwdPtr);
        state_.SetForwarded();
    }

    inline void SetSizeForwarded(size_t size)
    {
        // Set size in old obj when forwarded, forwardee obj size may changed.
        size_t objectHeaderSize = sizeof(BaseStateWord);
        DCHECK_CC(size >= objectHeaderSize + sizeof(size_t));
        auto addr = reinterpret_cast<MAddress>(this) + objectHeaderSize;
<<<<<<< HEAD
        *reinterpret_cast<size_t*>(addr) = size;
=======
        *reinterpret_cast<size_t *>(addr) = size;
>>>>>>> OpenHarmony_feature_20250328
    }

    inline size_t GetSizeForwarded() const
    {
        auto addr = reinterpret_cast<MAddress>(this) + sizeof(BaseStateWord);
<<<<<<< HEAD
        return *reinterpret_cast<size_t*>(addr);
=======
        return *reinterpret_cast<size_t *>(addr);
>>>>>>> OpenHarmony_feature_20250328
    }

    inline bool IsForwarding() const
    {
        return state_.IsForwarding();
    }

    inline bool IsForwarded() const
    {
        return state_.IsForwarded();
    }

<<<<<<< HEAD
    ALWAYS_INLINE_CC bool IsDynamic() const
    {
        return state_.IsDynamic();
    }

    ALWAYS_INLINE_CC bool IsStatic() const
    {
        return state_.IsStatic();
    }

=======
>>>>>>> OpenHarmony_feature_20250328
    inline bool IsToVersion() const
    {
        return state_.IsToVersion();
    }

<<<<<<< HEAD
    inline void SetLanguageType(LanguageType language)
    {
        state_.SetLanguageType(language);
=======
    inline void SetLanguage(Language language)
    {
        state_.SetLanguage(language);
>>>>>>> OpenHarmony_feature_20250328
    }

    BaseStateWord GetBaseStateWord() const
    {
        return state_.AtomicGetBaseStateWord();
    }

    void SetForwardState(BaseStateWord::ForwardState state)
    {
        state_.SetForwardState(state);
    }

<<<<<<< HEAD
=======
    ALWAYS_INLINE_CC bool IsDynamic() const
    {
        return state_.IsDynamic();
    }

    ALWAYS_INLINE_CC bool IsStatic() const
    {
        return state_.IsStatic();
    }

>>>>>>> OpenHarmony_feature_20250328
    template <typename T>
    Field<T> &GetField(uint32_t offset) const
    {
        auto addr = reinterpret_cast<MAddress>(this) + offset;
        return *reinterpret_cast<Field<T> *>(addr);
    }

    // Locking means that this object forwardstate is forwarding.
    // Any other gc coping thread or mutator thread will be wait if copy the same object.
    bool TryLockExclusive(const BaseStateWord state)
    {
        return state_.TryLockBaseStateWord(state);
    }

    void UnlockExclusive(const BaseStateWord::ForwardState newState)
    {
        state_.UnlockStateWord(newState);
    }

    static inline intptr_t FieldOffset(BaseObject *object, const void *field)
    {
        return reinterpret_cast<intptr_t>(field) - reinterpret_cast<intptr_t>(object);
    }

    // The interfaces following only use for common code compiler. It will be deleted later.
    TypeInfo *GetComponentTypeInfo() const
    {
        return reinterpret_cast<TypeInfo *>(const_cast<BaseObject *>(this));
    }

    inline bool HasRefField() const
    {
        return true;
    }

    inline TypeInfo *GetTypeInfo() const
    {
        return reinterpret_cast<TypeInfo *>(const_cast<BaseObject *>(this));
    }

    bool IsRawArray() const
    {
        return true;
    }

    void ForEachRefInStruct([[maybe_unused]] const RefFieldVisitor &visitor, [[maybe_unused]] MAddress aggStart,
                            [[maybe_unused]] MAddress aggEnd)
    {
    }
    // The interfaces above only use for common code compiler. It will be deleted later.

    void SetFullBaseClassWithoutBarrier(BaseClass* cls)
    {
        state_ = 0;
<<<<<<< HEAD
        state_.SetFullBaseClassAddress(reinterpret_cast<common::StateWordType>(cls));
=======
        state_.SetFullBaseClassAddress(reinterpret_cast<StateWordType>(cls));
>>>>>>> OpenHarmony_feature_20250328
    }

    BaseClass *GetBaseClass() const
    {
        return reinterpret_cast<BaseClass *>(state_.GetBaseClassAddress());
    }

    // Size of object header
    static constexpr size_t BaseObjectSize()
    {
        return sizeof(BaseObject);
    }
<<<<<<< HEAD
protected:
    static BaseObject *SetClassInfo(MAddress address, TypeInfo *klass)
    {
        auto ref = reinterpret_cast<BaseObject *>(address);
        return ref;
    }

=======

    bool IsString() const
    {
        return GetBaseClass()->IsString();
    }
protected:
>>>>>>> OpenHarmony_feature_20250328
    inline BaseObjectOperatorInterfaces *GetOperator() const
    {
        if (state_.IsStatic()) {
            return operator_.staticObjOp_;
        }
        return operator_.dynamicObjOp_;
    }

<<<<<<< HEAD
    static BaseObjectOperator operator_;
    BaseStateWord state_;
};
}  // namespace common
=======
    static PUBLIC_API BaseObjectOperator operator_;
    BaseStateWord state_;  // NOLINT(misc-non-private-member-variables-in-classes)
};

static_assert(sizeof(BaseObject) == sizeof(BaseClass::HeaderType));
}  // namespace panda
>>>>>>> OpenHarmony_feature_20250328
#endif  // COMMON_INTERFACES_OBJECTS_BASE_OBJECT_H
