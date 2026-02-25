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

#ifndef COMMON_INTERFACES_OBJECTS_BASE_OBJECT_DISPATCHER_H
#define COMMON_INTERFACES_OBJECTS_BASE_OBJECT_DISPATCHER_H

#include "common_interfaces/objects/base_object.h"
#include "common_interfaces/objects/base_object_accessor.h"
#include "common_interfaces/objects/base_object_descriptor.h"
#include "common_interfaces/objects/base_type.h"
#include "common_interfaces/objects/base_type_converter.h"
#include "common_interfaces/objects/static_object_accessor_interface.h"
#include "common_interfaces/objects/static_type_converter_interface.h"
#include "common_interfaces/thread/mutator.h"

namespace common {
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

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    JSTaggedValue GetTaggedProperty(Mutator *mutator, const BaseObject *obj, const char *name) const
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return dynObjAccessor_->GetProperty(mutator, obj, name);
        } else if constexpr (objType == ObjectType::STATIC) {
            BoxedValue value = stcObjAccessor_->GetProperty(mutator, obj, name);
            // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
            return dynTypeConverter_->WrapTagged(mutator, stcTypeConverter_->UnwrapBoxed(value));
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return default value.
                return dynObjAccessor_->GetProperty(mutator, obj, name);
            } else {  // NOLINT(readability-else-after-return)
                BoxedValue value = stcObjAccessor_->GetProperty(mutator, obj, name);
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                return dynTypeConverter_->WrapTagged(mutator, stcTypeConverter_->UnwrapBoxed(value));
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    BoxedValue GetBoxedProperty(Mutator *mutator, const BaseObject *obj, const char *name) const
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            JSTaggedValue value = dynObjAccessor_->GetProperty(mutator, obj, name);
            // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
            return stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value));
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return stcObjAccessor_->GetProperty(mutator, obj, name);
        } else {
            if (obj->IsDynamic()) {
                JSTaggedValue value = dynObjAccessor_->GetProperty(mutator, obj, name);
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                return stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value));
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return default value.
                return stcObjAccessor_->GetProperty(mutator, obj, name);
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    JSTaggedValue GetTaggedElementByIdx(Mutator *mutator, const BaseObject *obj, const uint32_t index) const
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return dynObjAccessor_->GetElementByIdx(mutator, obj, index);
        } else if constexpr (objType == ObjectType::STATIC) {
            BoxedValue value = stcObjAccessor_->GetElementByIdx(mutator, obj, index);
            // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
            return dynTypeConverter_->WrapTagged(mutator, stcTypeConverter_->UnwrapBoxed(value));
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return default value.
                return dynObjAccessor_->GetElementByIdx(mutator, obj, index);
            } else {  // NOLINT(readability-else-after-return)
                BoxedValue value = stcObjAccessor_->GetElementByIdx(mutator, obj, index);
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                return dynTypeConverter_->WrapTagged(mutator, stcTypeConverter_->UnwrapBoxed(value));
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    BoxedValue GetBoxedElementByIdx(Mutator *mutator, const BaseObject *obj, const uint32_t index) const
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            JSTaggedValue value = dynObjAccessor_->GetElementByIdx(mutator, obj, index);
            // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
            return stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value));
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return stcObjAccessor_->GetElementByIdx(mutator, obj, index);
        } else {
            if (obj->IsDynamic()) {
                JSTaggedValue value = dynObjAccessor_->GetElementByIdx(mutator, obj, index);
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                return stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value));
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return default value.
                return stcObjAccessor_->GetElementByIdx(mutator, obj, index);
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    bool SetTaggedProperty(Mutator *mutator, BaseObject *obj, const char *name, JSTaggedValue value)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return dynObjAccessor_->SetProperty(mutator, obj, name, value);
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return stcObjAccessor_->SetProperty(mutator, obj, name,
                                                stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value)));
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return default value.
                return dynObjAccessor_->SetProperty(mutator, obj, name, value);
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                return stcObjAccessor_->SetProperty(
                    mutator, obj, name, stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value)));
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    bool SetBoxedProperty(Mutator *mutator, BaseObject *obj, const char *name, BoxedValue value)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
            auto wrapedValue = dynTypeConverter_->WrapTagged(mutator, stcTypeConverter_->UnwrapBoxed(value));
            return dynObjAccessor_->SetProperty(mutator, obj, name, wrapedValue);
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return stcObjAccessor_->SetProperty(mutator, obj, name, value);
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                auto wrapedValue = dynTypeConverter_->WrapTagged(mutator, stcTypeConverter_->UnwrapBoxed(value));
                return dynObjAccessor_->SetProperty(mutator, obj, name, wrapedValue);
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return default value.
                return stcObjAccessor_->SetProperty(mutator, obj, name, value);
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    bool SetTaggedElementByIdx(Mutator *mutator, BaseObject *obj, const uint32_t index, JSTaggedValue value)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return dynObjAccessor_->SetElementByIdx(mutator, obj, index, value);
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
            return stcObjAccessor_->SetElementByIdx(
                mutator, obj, index, stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value)));
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return default value.
                return dynObjAccessor_->SetElementByIdx(mutator, obj, index, value);
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                return stcObjAccessor_->SetElementByIdx(
                    mutator, obj, index, stcTypeConverter_->WrapBoxed(dynTypeConverter_->UnWrapTagged(value)));
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    bool SetBoxedElementByIdx(Mutator *mutator, BaseObject *obj, const uint32_t index, BoxedValue value)
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
            return dynObjAccessor_->SetElementByIdx(
                mutator, obj, index, dynTypeConverter_->WrapTagged(mutator, stcTypeConverter_->UnwrapBoxed(value)));
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return stcObjAccessor_->SetElementByIdx(mutator, obj, index, value);
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return. Also, check exceptions in Wrap functions.
                return dynObjAccessor_->SetElementByIdx(
                    mutator, obj, index, dynTypeConverter_->WrapTagged(mutator, stcTypeConverter_->UnwrapBoxed(value)));
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return default value.
                return stcObjAccessor_->SetElementByIdx(mutator, obj, index, value);
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    bool HasProperty(Mutator *mutator, const BaseObject *obj, const char *name) const
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return dynObjAccessor_->HasProperty(mutator, obj, name);
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return stcObjAccessor_->HasProperty(mutator, obj, name);
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return default value.
                return dynObjAccessor_->HasProperty(mutator, obj, name);
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return default value.
                return stcObjAccessor_->HasProperty(mutator, obj, name);
            }
        }
    }

    // NOLINTNEXTLINE(readability-identifier-naming)
    template <ObjectType objType = ObjectType::UNKNOWN>
    bool HasElementByIdx(Mutator *mutator, const BaseObject *obj, const uint32_t index) const
    {
        if constexpr (objType == ObjectType::DYNAMIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return dynObjAccessor_->HasElementByIdx(mutator, obj, index);
        } else if constexpr (objType == ObjectType::STATIC) {
            // fix(hewei): exceptions may occur, check here and return default value.
            return stcObjAccessor_->HasElementByIdx(mutator, obj, index);
        } else {
            if (obj->IsDynamic()) {
                // fix(hewei): exceptions may occur, check here and return default value.
                return dynObjAccessor_->HasElementByIdx(mutator, obj, index);
            } else {  // NOLINT(readability-else-after-return)
                // fix(hewei): exceptions may occur, check here and return default value.
                return stcObjAccessor_->HasElementByIdx(mutator, obj, index);
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
}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_BASE_OBJECT_DISPATCHER_H