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

#include "tooling/inspector/types/remote_object.h"

#include "macros.h"
#include "tooling/inspector/types/property_descriptor.h"
#include "utils/json_builder.h"
#include "utils/string_helpers.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <variant>

using panda::helpers::string::Format;

namespace panda::tooling::inspector {
void RemoteObject::GeneratePreview(const std::vector<PropertyDescriptor> &properties) const
{
    preview_.clear();

    for (auto &property : properties) {
        if (!property.IsEnumerable()) {
            continue;
        }

        if (property.IsAccessor()) {
            preview_.push_back(PreviewProperty {property.GetName(), Type::Accessor(), {}, property.IsEntry()});
            continue;
        }

        const auto &value = property.GetValue().value_;
        std::optional<std::string> value_preview;

        if (std::holds_alternative<std::nullptr_t>(value)) {
            value_preview.emplace("null");
        } else if (auto boolean = std::get_if<bool>(&value)) {
            value_preview.emplace(*boolean ? "true" : "false");
        } else if (auto number = std::get_if<NumberT>(&value)) {
            if (auto integer = std::get_if<int32_t>(number)) {
                value_preview.emplace(std::to_string(*integer));
            } else if (auto floating_point = std::get_if<double>(number)) {
                value_preview.emplace(Format("%g", *floating_point));  // NOLINT(cppcoreguidelines-pro-type-vararg)
            } else {
                UNREACHABLE();
            }
        } else if (auto bigint = std::get_if<BigIntT>(&value)) {
            value_preview.emplace(GetDescription(*bigint));
        } else if (auto string = std::get_if<std::string>(&value)) {
            value_preview.emplace(*string);
        } else if (auto symbol = std::get_if<SymbolT>(&value)) {
            value_preview.emplace(symbol->description);
        } else if (auto object = std::get_if<ObjectT>(&value)) {
            value_preview.emplace(GetDescription(*object));
        } else if (auto array = std::get_if<ArrayT>(&value)) {
            value_preview.emplace(GetDescription(*array));
        } else if (auto function = std::get_if<FunctionT>(&value)) {
            value_preview.emplace(function->name);
        }

        preview_.push_back(PreviewProperty {property.GetName(), property.GetValue().GetType(), std::move(value_preview),
                                            property.IsEntry()});
    }
}

template <typename T>
static void AddElement(std::function<void(JsonArrayBuilder &)> &func, T value)
{
    func = [func = std::move(func), value = std::move(value)](JsonArrayBuilder &builder) {
        func(builder);
        builder.Add(value);
    };
}

template <typename T>
static void AddProperty(std::function<void(JsonObjectBuilder &)> &func, const char *key, T value)
{
    func = [func = std::move(func), key, value = std::move(value)](JsonObjectBuilder &builder) {
        func(builder);
        builder.AddProperty(key, value);
    };
}

std::string RemoteObject::GetDescription(const RemoteObject::BigIntT &bigint)
{
    return (bigint.sign >= 0 ? "" : "-") + std::to_string(bigint.value);
}

std::string RemoteObject::GetDescription(const RemoteObject::ObjectT &object)
{
    return object.description.value_or(object.class_name);
}

std::string RemoteObject::GetDescription(const RemoteObject::ArrayT &array)
{
    return array.class_name + "(" + std::to_string(array.length) + ")";
}

std::string RemoteObject::GetDescription(const RemoteObject::FunctionT &function)
{
    std::stringstream desc;

    desc << "function " << function.name << "(";

    for (auto arg_idx = 0U; arg_idx < function.length; ++arg_idx) {
        if (arg_idx != 0) {
            desc << ", ";
        }
        desc << static_cast<char>('a' + arg_idx);
    }

    desc << ") { [not available] }";

    return desc.str();
}

std::optional<RemoteObjectId> RemoteObject::GetObjectId() const
{
    if (auto object = std::get_if<ObjectT>(&value_)) {
        return object->object_id;
    }
    if (auto array = std::get_if<ArrayT>(&value_)) {
        return array->object_id;
    }
    if (auto function = std::get_if<FunctionT>(&value_)) {
        return function->object_id;
    }
    return {};
}

RemoteObject::Type RemoteObject::GetType() const
{
    if (std::holds_alternative<std::monostate>(value_)) {
        return Type("undefined");
    }
    if (std::holds_alternative<std::nullptr_t>(value_)) {
        return Type("object", "null");
    }
    if (std::holds_alternative<bool>(value_)) {
        return Type("boolean");
    }
    if (std::holds_alternative<NumberT>(value_)) {
        return Type("number");
    }
    if (std::holds_alternative<BigIntT>(value_)) {
        return Type("bigint");
    }
    if (std::holds_alternative<std::string>(value_)) {
        return Type("string");
    }
    if (std::holds_alternative<SymbolT>(value_)) {
        return Type("symbol");
    }
    if (std::holds_alternative<ObjectT>(value_)) {
        return Type("object");
    }
    if (std::holds_alternative<ArrayT>(value_)) {
        return Type("object", "array");
    }
    if (std::holds_alternative<FunctionT>(value_)) {
        return Type("function");
    }

    UNREACHABLE();
}

std::function<void(JsonObjectBuilder &)> RemoteObject::PreviewToJson() const
{
    std::function<void(JsonArrayBuilder &)> properties = [](auto &) {};
    std::function<void(JsonArrayBuilder &)> entries = [](auto &) {};
    bool has_entries = false;

    for (auto &preview_property : preview_) {
        auto property = preview_property.type.ToJson();

        if (preview_property.is_entry) {
            auto value = property;

            if (preview_property.value) {
                AddProperty(value, "description", *preview_property.value);
            }

            std::function<void(JsonObjectBuilder &)> entry = [](auto &) {};
            AddProperty(entry, "value", std::move(value));

            AddElement(entries, std::move(entry));
            has_entries = true;
        }

        AddProperty(property, "name", preview_property.name);

        if (preview_property.value) {
            AddProperty(property, "value", *preview_property.value);
        }

        AddElement(properties, std::move(property));
    }

    auto result = GetType().ToJson();

    AddProperty(result, "overflow", false);
    AddProperty(result, "properties", std::move(properties));

    if (has_entries) {
        AddProperty(result, "entries", std::move(entries));
    }

    return result;
}

std::function<void(JsonObjectBuilder &)> RemoteObject::ToJson() const
{
    auto result = GetType().ToJson();

    if (std::holds_alternative<std::nullptr_t>(value_)) {
        AddProperty(result, "value", nullptr);
    } else if (auto boolean = std::get_if<bool>(&value_)) {
        AddProperty(result, "value", *boolean);
    } else if (auto number = std::get_if<NumberT>(&value_)) {
        if (auto integer = std::get_if<int32_t>(number)) {
            AddProperty(result, "value", *integer);
        } else if (auto floating_point = std::get_if<double>(number)) {
            AddProperty(result, "value", *floating_point);
        } else {
            UNREACHABLE();
        }
    } else if (auto bigint = std::get_if<BigIntT>(&value_)) {
        AddProperty(result, "unserializableValue", GetDescription(*bigint));
    } else if (auto string = std::get_if<std::string>(&value_)) {
        AddProperty(result, "value", *string);
    } else if (auto symbol = std::get_if<SymbolT>(&value_)) {
        AddProperty(result, "description", symbol->description);
    } else if (auto object = std::get_if<ObjectT>(&value_)) {
        AddProperty(result, "className", object->class_name);
        AddProperty(result, "description", GetDescription(*object));
    } else if (auto array = std::get_if<ArrayT>(&value_)) {
        AddProperty(result, "className", array->class_name);
        AddProperty(result, "description", GetDescription(*array));
    } else if (auto function = std::get_if<FunctionT>(&value_)) {
        AddProperty(result, "className", function->class_name);
        AddProperty(result, "description", GetDescription(*function));
    }

    if (auto object_id = GetObjectId()) {
        AddProperty(result, "objectId", std::to_string(*object_id));
    }

    if (!preview_.empty()) {
        AddProperty(result, "preview", PreviewToJson());
    }

    return result;
}

std::function<void(JsonObjectBuilder &)> RemoteObject::Type::ToJson() const
{
    std::function<void(JsonObjectBuilder &)> result = [](auto &) {};

    AddProperty(result, "type", type_);

    if (subtype_ != nullptr) {
        AddProperty(result, "subtype", subtype_);
    }

    return result;
}
}  // namespace panda::tooling::inspector
