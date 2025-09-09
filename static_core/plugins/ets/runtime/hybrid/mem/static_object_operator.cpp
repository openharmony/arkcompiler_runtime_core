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

namespace ark::mem {

StaticObjectOperator StaticObjectOperator::instance_;

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

void StaticObjectOperator::Initialize()
{
#if defined(ARK_HYBRID)
    panda::BaseObject::RegisterStatic(&instance_);
#endif
}

void StaticObjectOperator::ForEachRefField(const panda::BaseObject *object, const panda::RefFieldVisitor &visitor) const
{
    Handler handler(visitor);
    auto *objHeader = reinterpret_cast<ObjectHeader *>(const_cast<panda::BaseObject *>(object));
    ObjectIterator<LANG_TYPE_STATIC>::template Iterate<false>(objHeader->ClassAddr<Class>(), objHeader, &handler);
}

size_t StaticObjectOperator::ForEachRefFieldAndGetSize(const panda::BaseObject *object,
                                                       const panda::RefFieldVisitor &visitor) const
{
    size_t size = GetSize(object);
    ForEachRefField(object, visitor);
    return size;
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
