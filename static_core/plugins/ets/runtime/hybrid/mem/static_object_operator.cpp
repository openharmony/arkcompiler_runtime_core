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

#include "runtime/mem/object_helpers.h"
#include "runtime/mem/object-references-iterator-inl.h"
#include "plugins/ets/runtime/hybrid/mem/static_object_operator.h"

#if defined(ARK_USE_CMC_GC)
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/hybrid/mem/external-gc.h"
#include "plugins/ets/runtime/interop_js/xgc/xgc.h"
#include "plugins/ets/runtime/mem/ets_reference_processor.h"
#endif

namespace ark::mem {

StaticObjectOperator StaticObjectOperator::instance_;

#ifdef PANDA_JS_ETS_HYBRID_MODE
class SkipReferentHandler {
public:
    explicit SkipReferentHandler(const panda::RefFieldVisitor &visitor, ObjectPointerType *weakReferentPointer)
        : visitor_(visitor), weakReferentPointer_(weakReferentPointer)
    {
    }

    ~SkipReferentHandler() = default;

    bool ProcessObjectPointer(ObjectPointerType *p)
    {
        if (p == weakReferentPointer_) {
            // skip referent
            return true;
        }
        if (*p != 0) {
            visitor_(reinterpret_cast<panda::RefField<> &>(*reinterpret_cast<panda::BaseObject **>(p)));
        }
        return true;
    }

private:
    const panda::RefFieldVisitor &visitor_;
    const ObjectPointerType *weakReferentPointer_ {nullptr};
};
#endif

class Handler {
public:
    explicit Handler(const panda::RefFieldVisitor &visitor) : visitor_(visitor) {}

    ~Handler() = default;

    bool ProcessObjectPointer(ObjectPointerType *p)
    {
        if (*p != 0) {
            visitor_(reinterpret_cast<panda::RefField<> &>(*reinterpret_cast<panda::BaseObject **>(p)));
        }
        return true;
    }

private:
    const panda::RefFieldVisitor &visitor_;
};

void StaticObjectOperator::Initialize(ark::ets::PandaEtsVM *vm)
{
#if defined(ARK_USE_CMC_GC)
    instance_.vm_ = vm;
    panda::BaseObject::RegisterStatic(&instance_);

    panda::RegisterStaticRootsProcessFunc();
#endif
}

// A temporary impl only in interop
void StaticObjectOperator::ForEachRefFieldSkipReferent(const panda::BaseObject *object,
                                                       const panda::RefFieldVisitor &visitor) const
{
#ifdef PANDA_JS_ETS_HYBRID_MODE
    auto *objHeader = reinterpret_cast<ObjectHeader *>(const_cast<panda::BaseObject *>(object));
    auto *baseCls = objHeader->template ClassAddr<BaseClass>();
    auto *process = static_cast<ark::mem::ets::EtsReferenceProcessor *>(vm_->GetReferenceProcessor());
    ObjectPointerType *referentPointer = nullptr;
    if (process->IsReference(baseCls)) {
        process->HandleReference(objHeader, referentPointer);
    }
    SkipReferentHandler handler(visitor, referentPointer);
    ObjectIterator<LANG_TYPE_STATIC>::template Iterate<false>(objHeader->ClassAddr<Class>(), objHeader, &handler);
#else
    // Only support in interop
    std::abort();
#endif
}

void StaticObjectOperator::ForEachRefField(const panda::BaseObject *object, const panda::RefFieldVisitor &visitor) const
{
    Handler handler(visitor);
    auto *objHeader = reinterpret_cast<ObjectHeader *>(const_cast<panda::BaseObject *>(object));
    ObjectIterator<LANG_TYPE_STATIC>::template Iterate<false>(objHeader->ClassAddr<Class>(), objHeader, &handler);
}

void StaticObjectOperator::IterateXRef(const panda::BaseObject *object, const panda::RefFieldVisitor &visitor) const
{
#ifdef PANDA_JS_ETS_HYBRID_MODE
    auto *obj = reinterpret_cast<ObjectHeader *>(const_cast<panda::BaseObject *>(object));
    auto *etsObj = ark::ets::EtsObject::FromCoreType(obj);
    if (!etsObj->HasInteropIndex()) {
        return;
    }
    ark::ets::interop::js::XGC::GetInstance()->IterateEtsObjectXRef(etsObj, visitor);
#else
    // Only support in interop
    std::abort();
#endif  // PANDA_JS_ETS_HYBRID_MODE
}

panda::BaseObject *StaticObjectOperator::GetForwardingPointer(const panda::BaseObject *object) const
{
    // Overwrite class by forwarding address. Read barrier must be called before reading class.
    uint64_t fwdAddr = *reinterpret_cast<const uint64_t *>(object);
    return reinterpret_cast<panda::BaseObject *>(fwdAddr & ObjectHeader::GetClassMask());
}

void StaticObjectOperator::SetForwardingPointerAfterExclusive(panda::BaseObject *object, panda::BaseObject *fwdPtr)
{
    auto &word = *reinterpret_cast<uint64_t *>(object);
    uint64_t flags = word & (~ObjectHeader::GetClassMask());
    word = flags | (reinterpret_cast<uint64_t>(fwdPtr) & ObjectHeader::GetClassMask());
}

}  // namespace ark::mem
