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

#ifndef COMMON_INTERFACES_OBJECTS_BASE_OBJECT_DISPATCHER_H
#define COMMON_INTERFACES_OBJECTS_BASE_OBJECT_DISPATCHER_H

#include "objects/base_object.h"
#include "objects/base_object_accessor.h"
#include "objects/base_object_descriptor.h"
#include "objects/base_type.h"
#include "objects/base_type_converter.h"
#include "thread/thread_holder.h"

namespace panda {
class BaseObjectDispatcher {
enum class ObjectType : uint8_t { STATIC = 0x0, DYNAMIC, UNKNOWN };
public:
    // Singleton
    static BaseObjectDispatcher& GetDispatcher()
    {
        static BaseObjectDispatcher instance;
        return instance;
    }

    void RegisterDynamicTypeConverter(DynamicTypeConverterInterface *dynTypeConverter)
    {
        dynTypeConverter_ = dynTypeConverter;
    }

    void RegisterDynamicObjectAccessor(DynamicObjectAccessorInterface *dynObjAccessor)
    {
        dynObjAccessor_ = dynObjAccessor;
    }

    void RegisterDynamicObjectDescriptor(DynamicObjectDescriptorInterface *dynObjDescriptor)
    {
        dynObjDescriptor_ = dynObjDescriptor;
    }

    void RegisterStaticTypeConverter(StaticTypeConverterInterface *stcTypeConverter)
    {
        stcTypeConverter_ = stcTypeConverter;
    }

    void RegisterStaticObjectAccessor(StaticObjectAccessorInterface *stcObjAccessor)
    {
        stcObjAccessor_ = stcObjAccessor;
    }

    void RegisterStaticObjectDescriptor(StaticObjectDescriptorInterface *stcObjDescriptor)
    {
        stcObjDescriptor_ = stcObjDescriptor;
    }

    template <ObjectType objType = ObjectType::UNKNOWN>
    JSTaggedValue GetTaggedProperty(ThreadHolder *thread, BaseObject *obj, char* name)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            return dynObjAccessor_->GetProperty(thread, obj, name);
        } else if constexpr (objType == ObjectType::STATIC) {
            BoxedValue value = stcObjAccessor_->GetProperty(thread, obj, name);
            return dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnWrapBoxed(value));
        } else {
            if (obj->IsDynamic()) {
                return dynObjAccessor_->GetProperty(thread, obj, name);
            } else {
                BoxedValue value = stcObjAccessor_->GetProperty(thread, obj, name);
                return dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnWrapBoxed(value));
            }
        }
    }

    template <ObjectType objType = ObjectType::UNKNOWN>
    BoxedValue GetBoxedProperty(ThreadHolder *thread, BaseObject *obj, char* name)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            JSTaggedValue value = dynObjAccessor_->GetProperty(thread, obj, name);
            return stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value));
        } else if constexpr (objType == ObjectType::STATIC) {
            return stcObjAccessor_->GetProperty(thread, obj, name);
        } else {
            if (obj->IsDynamic()) {
                JSTaggedValue value = dynObjAccessor_->GetProperty(thread, obj, name);
                return stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value));
            } else {
                return stcObjAccessor_->GetProperty(thread, obj, name);
            }
        }
    }

    template <ObjectType objType = ObjectType::UNKNOWN>
    JSTaggedValue GetTaggedElementByIdx(ThreadHolder *thread, BaseObject *obj, uint32_t index)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            return dynObjAccessor_->GetElementByIdx(thread, obj, index);
        } else if constexpr (objType == ObjectType::STATIC) {
            BoxedValue value = stcObjAccessor_->GetElementByIdx(thread, obj, index);
            return dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnWrapBoxed(value));
        } else {
            if (obj->IsDynamic()) {
                return dynObjAccessor_->GetElementByIdx(thread, obj, index);
            } else {
                BoxedValue value = stcObjAccessor_->GetElementByIdx(thread, obj, index);
                return dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnWrapBoxed(value));
            }
        }
    }

    template <ObjectType objType = ObjectType::UNKNOWN>
    BoxedValue GetBoxedElementByIdx(ThreadHolder *thread, BaseObject *obj, uint32_t index)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            JSTaggedValue value = dynObjAccessor_->GetElementByIdx(thread, obj, index);
            return stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value));
        } else if constexpr (objType == ObjectType::STATIC) {
            return stcObjAccessor_->GetElementByIdx(thread, obj, index);
        } else {
            if (obj->IsDynamic()) {
                JSTaggedValue value = dynObjAccessor_->GetElementByIdx(thread, obj, index);
                return stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value));
            } else {
                return stcObjAccessor_->GetElementByIdx(thread, obj, index);
            }
        }
    }

    template <ObjectType objType = ObjectType::UNKNOWN>
    bool SetTaggedProperty(ThreadHolder *thread, BaseObject *obj, char* name, JSTaggedValue value)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            return dynObjAccessor_->SetProperty(thread, obj, name, value);
        } else if constexpr (objType == ObjectType::STATIC) {
            return stcObjAccessor_->SetProperty(thread, obj, name,
                                                stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value)));
        } else {
            if (obj->IsDynamic()) {
                return dynObjAccessor_->SetProperty(thread, obj, name, value);
            } else {
                return stcObjAccessor_->SetProperty(
                    thread, obj, name, stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value)));
            }
        }
    }

    template <ObjectType objType = ObjectType::UNKNOWN>
    bool SetBoxedProperty(ThreadHolder *thread, BaseObject *obj, char *name, BoxedValue value)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            auto wrapedValue = dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnWrapBoxed(value));
            return dynObjAccessor_->SetProperty(thread, obj, name, wrapedValue);
        } else if constexpr (objType == ObjectType::STATIC) {
            return stcObjAccessor_->SetProperty(thread, obj, name, value);
        } else {
            if (obj->IsDynamic()) {
                auto wrapedValue = dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnWrapBoxed(value));
                return dynObjAccessor_->SetProperty(thread, obj, name, wrapedValue);
            } else {
                return stcObjAccessor_->SetProperty(thread, obj, name, value);
            }
        }
    }

    template <ObjectType objType = ObjectType::UNKNOWN>
    bool SetTaggedElementByIdx(ThreadHolder *thread, BaseObject *obj, uint32_t index, JSTaggedValue value)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            return dynObjAccessor_->SetElementByIdx(thread, obj, index, value);
        } else if constexpr (objType == ObjectType::STATIC) {
            return stcObjAccessor_->SetElementByIdx(thread, obj, index,
                stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value)));
        } else {
            if (obj->IsDynamic()) {
                return dynObjAccessor_->SetElementByIdx(thread, obj, index, value);
            } else {
                return stcObjAccessor_->SetElementByIdx(
                    thread, obj, index, stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value)));
            }
        }
    }

    template <ObjectType objType = ObjectType::UNKNOWN>
    bool SetBoxedElementByIdx(ThreadHolder *thread, BaseObject *obj, uint32_t index, BoxedValue value)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            return dynObjAccessor_->SetElementByIdx(thread, obj, index,
                dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnWrapBoxed(value)));
        } else if constexpr (objType == ObjectType::STATIC) {
            return stcObjAccessor_->SetElementByIdx(thread, obj, index, value);
        } else {
            if (obj->IsDynamic()) {
                return dynObjAccessor_->SetElementByIdx(thread, obj, index,
                    dynTypeConverter_->WrapTagged(thread, stcTypeConverter_->UnWrapBoxed(value)));
            } else {
                return stcObjAccessor_->SetElementByIdx(thread, obj, index, value);
            }
        }
    }

    template <ObjectType objType = ObjectType::UNKNOWN>
    bool HasProperty(ThreadHolder *thread, BaseObject *obj, char* name)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            return dynObjAccessor_->HasProperty(thread, obj, name);
        } else if constexpr (objType == ObjectType::STATIC) {
            return stcObjAccessor_->HasProperty(thread, obj, name);
        } else {
            if (obj->IsDynamic()) {
                return dynObjAccessor_->HasProperty(thread, obj, name);
            } else {
                return stcObjAccessor_->HasProperty(thread, obj, name);
            }
        }
    }

    template <ObjectType objType = ObjectType::UNKNOWN>
    bool HasElementByIdx(ThreadHolder *thread, BaseObject *obj, uint32_t index)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            return dynObjAccessor_->HasElementByIdx(thread, obj, index);
        } else if constexpr (objType == ObjectType::STATIC) {
            return stcObjAccessor_->HasElementByIdx(thread, obj, index);
        } else {
            if (obj->IsDynamic()) {
                return dynObjAccessor_->HasElementByIdx(thread, obj, index);
            } else {
                return stcObjAccessor_->HasElementByIdx(thread, obj, index);
            }
        }
    }

private:
    DynamicTypeConverterInterface *dynTypeConverter_;
    StaticTypeConverterInterface *stcTypeConverter_;
    DynamicObjectAccessorInterface *dynObjAccessor_;
    StaticObjectAccessorInterface *stcObjAccessor_;
    DynamicObjectDescriptorInterface *dynObjDescriptor_;
    StaticObjectDescriptorInterface *stcObjDescriptor_;
};
}  // namespace panda
#endif  // COMMON_INTERFACES_OBJECTS_BASE_OBJECT_DISPATCHER_H