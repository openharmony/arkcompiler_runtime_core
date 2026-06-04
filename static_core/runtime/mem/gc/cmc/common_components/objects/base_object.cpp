/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "common_interfaces/objects/base_object.h"
#include "mem/object-references-iterator-inl.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_weak_reference.h"
#include "runtime/mem/gc/reference-processor/reference_processor.h"
#include "include/panda_vm.h"

namespace ark::common_vm {

class Handler {
public:
    explicit Handler(const ark::mem::RefFieldVisitor &visitor) : visitor_(visitor) {}

    ~Handler() = default;

    bool ProcessObjectPointer([[maybe_unused]] ObjectHeader *obj, ObjectPointerType *p)
    {
        if (*p == 0) {
            return true;
        }
        auto **ref = reinterpret_cast<ark::common_vm::BaseObject **>(p);
        visitor_(reinterpret_cast<ark::mem::RefField<> &>(*ref));
        return true;
    }

private:
    const ark::mem::RefFieldVisitor &visitor_;
};

class SkipReferentHandler : public Handler {
public:
    explicit SkipReferentHandler(const ark::mem::RefFieldVisitor &visitor, ObjectPointerType *weakReferentPointer)
        : Handler(visitor), weakReferentPointer_(weakReferentPointer)
    {
    }

    ~SkipReferentHandler() = default;

    bool ProcessObjectPointer(ObjectHeader *obj, ObjectPointerType *p)
    {
        if (p == weakReferentPointer_) {
            // skip referent
            return true;
        }
        return Handler::ProcessObjectPointer(obj, p);
    }

private:
    const ObjectPointerType *weakReferentPointer_ {nullptr};
};

void BaseObject::ForEachRefField(const RefFieldVisitor &fieldHandler, const RefFieldVisitor &weakFieldHandler)
{
    const auto *objHeader = reinterpret_cast<const ObjectHeader *>(this);
    auto *cls = objHeader->template ClassAddr<Class>();
    auto *etsClass = ark::ets::EtsClass::FromRuntimeClass(cls);
    if (UNLIKELY(etsClass->IsReference())) {
        ObjectPointerType *referentPointer = reinterpret_cast<ObjectPointerType *>(
            ToUintPtr(objHeader) + ark::ets::EtsWeakReference::GetReferentOffset());
        if (*referentPointer != 0) {
            weakFieldHandler(reinterpret_cast<ark::mem::RefField<> &>(
                *reinterpret_cast<ark::common_vm::BaseObject **>(referentPointer)));
        }
        SkipReferentHandler handler(fieldHandler, referentPointer);
        mem::ObjectIterator<LangTypeT::LANG_TYPE_STATIC>::template Iterate<false>(
            cls, const_cast<ObjectHeader *>(objHeader), &handler);
    } else {
        Handler handler(fieldHandler);
        mem::ObjectIterator<LangTypeT::LANG_TYPE_STATIC>::template Iterate<false>(
            cls, const_cast<ObjectHeader *>(objHeader), &handler);
    }
}

void BaseObject::ClearRef(RefField<> &field)
{
    field.SetTargetObject(nullptr);
    auto *vm = ark::PandaVM::GetCurrent();
    auto *referenceProcessor = static_cast<ark::mem::ets::EtsReferenceProcessor *>(vm->GetReferenceProcessor());
    referenceProcessor->EnqueueReference(static_cast<ObjectHeader *>(this));
}

}  // namespace ark::common_vm
