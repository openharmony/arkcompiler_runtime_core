/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINALIZABLE_WEAK_REF_LIST_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINALIZABLE_WEAK_REF_LIST_H

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_finalizable_weak_ref.h"

namespace ark::ets {

class EtsFinalizableWeakRefList : protected EtsFinalizableWeakRef {
public:
    using Node = EtsFinalizableWeakRef;
    using List = EtsFinalizableWeakRefList;

    void Push(EtsExecutionContext *executionCtx, Node *weakRef)
    {
        ASSERT(weakRef != nullptr);
        auto *head = GetHead();
        if (head == nullptr) {
            SetHead(executionCtx, weakRef);
            return;
        }
        head->SetPrev(executionCtx, weakRef);
        weakRef->SetNext(executionCtx, head);
        SetHead(executionCtx, weakRef);
    }

    bool Unlink(EtsExecutionContext *executionCtx, Node *weakRef)
    {
        ASSERT(weakRef != nullptr);
        auto *prev = weakRef->GetPrev();
        auto *next = weakRef->GetNext();
        if (prev == nullptr && next == nullptr && weakRef != GetHead()) {
            return false;
        }
        if (prev != nullptr) {
            prev->SetNext(executionCtx, next);
        }
        if (next != nullptr) {
            next->SetPrev(executionCtx, prev);
        }
        if (weakRef == GetHead()) {
            SetHead(executionCtx, next);
        }
        weakRef->SetPrev(executionCtx, nullptr);
        weakRef->SetNext(executionCtx, nullptr);
        return true;
    }

    void UnlinkClearedReferences(EtsExecutionContext *executionCtx)
    {
        auto *weakRef = GetHead();
        while (weakRef != nullptr) {
            if (weakRef->GetReferent() == nullptr) {
                // Finalizer of the cleared reference must be enqueued
                ASSERT(weakRef->ReleaseFinalizer().IsEmpty());
                Unlink(executionCtx, weakRef);
            }
            weakRef = weakRef->GetNext();
        }
    }

    static List *FromCoreType(ObjectHeader *intrusiveList)
    {
        return reinterpret_cast<List *>(intrusiveList);
    }

    ObjectHeader *GetCoreType()
    {
        return reinterpret_cast<ObjectHeader *>(const_cast<EtsObject *>(AsObject()));
    }

    const ObjectHeader *GetCoreType() const
    {
        return reinterpret_cast<const ObjectHeader *>(AsObject());
    }

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    static List *FromEtsObject(EtsObject *intrusiveList)
    {
        return reinterpret_cast<List *>(intrusiveList);
    }

    void TraverseAndFinalize()
    {
        auto *weakRef = GetHead();
        while (weakRef != nullptr) {
            auto finalizer = weakRef->ReleaseFinalizer();
            if (!finalizer.IsEmpty()) {
                finalizer.Run();
            }
            weakRef = weakRef->GetNext();
        }
    }

private:
    Node *GetHead() const
    {
        ASSERT(GetPrev() == nullptr);
        ASSERT(GetReferent() == nullptr);
        return GetNext();
    }

    void SetHead(EtsExecutionContext *executionCtx, Node *weakRef)
    {
        ASSERT(GetPrev() == nullptr);
        ASSERT(GetReferent() == nullptr);
        return SetNext(executionCtx, weakRef);
    }
};

static_assert(sizeof(EtsFinalizableWeakRefList) == sizeof(EtsFinalizableWeakRef));

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_FINALIZABLE_WEAK_REF_LIST_H
