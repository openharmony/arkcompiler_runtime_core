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

#ifndef COMMON_INTERFACES_OBJECTS_BASE_OBJECT_ACCESSOR_H
#define COMMON_INTERFACES_OBJECTS_BASE_OBJECT_ACCESSOR_H

#include "objects/base_object.h"
#include "objects/base_type.h"
#include "thread/thread_holder.h"

namespace common {
// The interface will be implemented in the dynamic runtime to provide the ability to access properties of 1.0 objects.
class DynamicObjectAccessorInterface {
public:
    //  HasProperty is used to check if the dynamic object has a property with the given name.
    virtual bool HasProperty(ThreadHolder *thread, BaseObject *obj, char *name) = 0;

    // GetProperty is used to get the value of a property from a dynamic object with the given name.
    virtual JSTaggedValue GetProperty(ThreadHolder *thread, BaseObject *obj, char *name) = 0;

    // SetProperty is used to set the value of a property in a dynamic object with the given name.
    virtual bool SetProperty(ThreadHolder *thread, BaseObject *obj, char *name, JSTaggedValue value) = 0;

    // HasElementByIdx is used to check if the dynamic object has an element with the given index.
    virtual bool HasElementByIdx(ThreadHolder *thread, BaseObject *obj, uint32_t index) = 0;

    // GetElementByIdx is used to get the value of an element from a dynamic object with the given index.
    virtual JSTaggedValue GetElementByIdx(ThreadHolder *thread, BaseObject *obj, uint32_t index) = 0;

    // SetElementByIdx is used to set the value of an element in a dynamic object with the given index.
    virtual bool SetElementByIdx(ThreadHolder *thread, BaseObject *obj, uint32_t index, JSTaggedValue value) = 0;
};

// The interface will be implemented in the static runtime to provide the ability to access
// properties of static objects.
class StaticObjectAccessorInterface {
public:
    // HasProperty is used to check if the static object has a property with the given name.
    virtual bool HasProperty(ThreadHolder *thread, BaseObject *obj, char *name) = 0;

    // GetProperty is used to get the value of a property from a static object with the given name.
    virtual BoxedValue GetProperty(ThreadHolder *thread, BaseObject *obj, char *name) = 0;

    // SetProperty is used to set the value of a property in a static object with the given name.
    virtual bool SetProperty(ThreadHolder *thread, BaseObject *obj, char *name, BoxedValue value) = 0;

    // HasElementByIdx is used to check if the static object has an element with the given index.
    virtual bool HasElementByIdx(ThreadHolder *thread, BaseObject *obj, uint32_t index) = 0;

    // GetElementByIdx is used to get the value of an element from a static object with the given index.
    virtual BoxedValue GetElementByIdx(ThreadHolder *thread, BaseObject *obj, uint32_t index) = 0;

    // SetElementByIdx is used to set the value of an element in a static object with the given index.
    virtual bool SetElementByIdx(ThreadHolder *thread, BaseObject *obj, uint32_t index, BoxedValue value) = 0;
};
}  // namespace common
#endif  // COMMON_INTERFACES_BASE_OBJECT_ACCESSOR_H