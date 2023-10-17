/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_TYPES_PROPERTY_DESCRIPTOR_H
#define PANDA_TOOLING_INSPECTOR_TYPES_PROPERTY_DESCRIPTOR_H

#include "remote_object.h"

#include "macros.h"
#include "utils/json_builder.h"

#include <functional>
#include <string>
#include <utility>

namespace panda::tooling::inspector {
class PropertyDescriptor {
public:
    PropertyDescriptor(std::string name, RemoteObject value, bool is_entry = false)
        : name_(std::move(name)), value_(std::move(value)), is_entry_(is_entry)
    {
    }

    static PropertyDescriptor Accessor(std::string name, RemoteObject getter)
    {
        PropertyDescriptor property(std::move(name), std::move(getter));
        property.is_accessor_ = true;
        return property;
    }

    bool IsAccessor() const
    {
        return is_accessor_;
    }

    bool IsConfigurable() const
    {
        return configurable_;
    }

    bool IsEntry() const
    {
        return is_entry_;
    }

    bool IsEnumerable() const
    {
        return enumerable_;
    }

    bool IsWritable() const
    {
        ASSERT(!IsAccessor());
        return writable_;
    }

    const RemoteObject &GetGetter() const
    {
        ASSERT(IsAccessor());
        return value_;
    }

    const std::string &GetName() const
    {
        return name_;
    }

    const std::optional<RemoteObject> &GetSymbol() const
    {
        return symbol_;
    }

    const RemoteObject &GetValue() const
    {
        ASSERT(!IsAccessor());
        return value_;
    }

    void SetConfigurable(bool configurable)
    {
        configurable_ = configurable;
    }

    void SetEnumerable(bool enumerable)
    {
        enumerable_ = enumerable;
    }

    void SetSymbol(RemoteObject symbol)
    {
        symbol_.emplace(std::move(symbol));
    }

    void SetWritable(bool writable)
    {
        ASSERT(!IsAccessor());
        writable_ = writable;
    }

    std::function<void(JsonObjectBuilder &)> ToJson() const
    {
        return [this](JsonObjectBuilder &json_builder) {
            json_builder.AddProperty("name", name_);

            if (symbol_) {
                json_builder.AddProperty("symbol", symbol_->ToJson());
            }

            if (is_accessor_) {
                json_builder.AddProperty("get", value_.ToJson());
            } else {
                json_builder.AddProperty("value", value_.ToJson());
                json_builder.AddProperty("writable", writable_);
            }

            json_builder.AddProperty("configurable", configurable_);
            json_builder.AddProperty("enumerable", enumerable_);
        };
    }

private:
    std::string name_;
    std::optional<RemoteObject> symbol_;
    RemoteObject value_;
    bool is_accessor_ {false};
    bool is_entry_ {false};
    bool configurable_ {false};
    bool enumerable_ {true};
    bool writable_ {true};
};
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_TYPES_PROPERTY_DESCRIPTOR_H
