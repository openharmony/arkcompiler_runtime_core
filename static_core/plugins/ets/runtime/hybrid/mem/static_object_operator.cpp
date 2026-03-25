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

#include "runtime/mem/object_helpers.h"
#include "runtime/mem/object-references-iterator-inl.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_weak_reference.h"
#include "plugins/ets/runtime/hybrid/mem/static_object_operator.h"

#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"

namespace ark::mem {

StaticObjectOperator StaticObjectOperator::instance_;

class Handler {
public:
    explicit Handler(const common_vm::RefFieldVisitor &visitor) : visitor_(visitor) {}

    ~Handler() = default;

    bool ProcessObjectPointer([[maybe_unused]] ObjectHeader *obj, ObjectPointerType *p)
    {
        if (*p == 0) {
            return true;
        }
        auto **ref = reinterpret_cast<common_vm::BaseObject **>(p);
        visitor_(reinterpret_cast<common_vm::RefField<> &>(*ref));
        return true;
    }

private:
    const common_vm::RefFieldVisitor &visitor_;
};

class SkipReferentHandler : public Handler {
public:
    explicit SkipReferentHandler(const common_vm::RefFieldVisitor &visitor, ObjectPointerType *weakReferentPointer)
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

void StaticObjectOperator::Initialize()
{
#if defined(ARK_USE_COMMON_RUNTIME)
    common_vm::BaseObject::RegisterStatic(&instance_);
#endif  // ARK_USE_COMMON_RUNTIME
}

void StaticObjectOperator::ForEachRefField(const common_vm::BaseObject *object,
                                           const common_vm::RefFieldVisitor &fieldHandler,
                                           const common_vm::RefFieldVisitor &weakFieldHandler) const
{
    const auto *objHeader = reinterpret_cast<const ObjectHeader *>(object);
    auto *cls = objHeader->template ClassAddr<Class>();
    auto *etsClass = ark::ets::EtsClass::FromRuntimeClass(cls);
    if (UNLIKELY(etsClass->IsReference())) {
        ObjectPointerType *referentPointer = reinterpret_cast<ObjectPointerType *>(
            ToUintPtr(objHeader) + ark::ets::EtsWeakReference::GetReferentOffset());
        if (*referentPointer != 0) {
            weakFieldHandler(reinterpret_cast<common_vm::RefField<> &>(
                *reinterpret_cast<common_vm::BaseObject **>(referentPointer)));
        }
        SkipReferentHandler handler(fieldHandler, referentPointer);
        ObjectIterator<LangTypeT::LANG_TYPE_STATIC>::template Iterate<false>(cls, const_cast<ObjectHeader *>(objHeader),
                                                                             &handler);
    } else {
        Handler handler(fieldHandler);
        ObjectIterator<LangTypeT::LANG_TYPE_STATIC>::template Iterate<false>(cls, const_cast<ObjectHeader *>(objHeader),
                                                                             &handler);
    }
}

size_t StaticObjectOperator::ForEachRefFieldAndGetSize(const common_vm::BaseObject *object,
                                                       const common_vm::RefFieldVisitor &fieldHandler,
                                                       const common_vm::RefFieldVisitor &weakFieldHandler) const
{
    size_t size = GetSize(object);
    ForEachRefField(object, fieldHandler, weakFieldHandler);
    return size;
}

void StaticObjectOperator::ClearRef(common_vm::BaseObject *object, common_vm::RefField<> &field) const
{
    field.SetTargetObject(nullptr);
    auto *vm = ark::ets::PandaEtsVM::GetCurrent();
    auto *referenceProcessor = static_cast<mem::ets::EtsReferenceProcessor *>(vm->GetReferenceProcessor());
    referenceProcessor->EnqueueReference(reinterpret_cast<ObjectHeader *>(object));
}

common_vm::BaseObject *StaticObjectOperator::GetForwardingPointer(const common_vm::BaseObject *object) const
{
    // Overwrite class by forwarding address. Read barrier must be called before reading class.
    uint64_t fwdAddr = *reinterpret_cast<const uint64_t *>(object);
    return reinterpret_cast<common_vm::BaseObject *>(fwdAddr & ObjectHeader::GetClassMask());
}

void StaticObjectOperator::SetForwardingPointerAfterExclusive(common_vm::BaseObject *object,
                                                              common_vm::BaseObject *fwdPtr)
{
    auto &word = *reinterpret_cast<uint64_t *>(object);
    uint64_t flags = word & (~ObjectHeader::GetClassMask());
    word = flags | (reinterpret_cast<uint64_t>(fwdPtr) & ObjectHeader::GetClassMask());
}

}  // namespace ark::mem
