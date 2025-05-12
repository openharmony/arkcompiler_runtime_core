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

#ifndef COMMON_INTERFACES_OBJECTS_BASE_OBJECT_OPERATOR_H
#define COMMON_INTERFACES_OBJECTS_BASE_OBJECT_OPERATOR_H

#include <cstddef>
#include <cstdint>

#include "heap/heap_visitor.h"
#include "objects/field.h"
#include "objects/ref_field.h"
#include "objects/base_state_word.h"

namespace panda {
class BaseObject;

class BaseObjectOperatorInterfaces {
public:
    // Get Object size.
    virtual size_t GetSize(const BaseObject *object) const = 0;
    // Check is valid object.
    virtual bool IsValidObject(const BaseObject *object) const = 0;
    // Iterate object field.
    virtual void ForEachRefField(const BaseObject *object, const RefFieldVisitor &visitor) const = 0;
    // Get forwarding pointer.
    virtual BaseObject *GetForwardingPointer(const BaseObject *object) const = 0;
    // Set forwarding pointer.
    virtual void SetForwardingPointerExclusive(BaseObject *object, BaseObject *fwdPtr) = 0;

    virtual ~BaseObjectOperatorInterfaces() = default;
};

#ifdef USE_CMC_GC
class BaseObjectOperator {
private:
    BaseObjectOperatorInterfaces *dynamicObjOp_;
    BaseObjectOperatorInterfaces *staticObjOp_;
    friend BaseObject;
};
#endif
}  // namespace panda
#endif  // COMMON_INTERFACES_OBJECTS_BASE_OBJECT_OPERATOR_H
