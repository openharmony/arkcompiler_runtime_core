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
#ifndef PANDA_TOOLING_INSPECTOR_REMOTE_OBJECT_H
#define PANDA_TOOLING_INSPECTOR_REMOTE_OBJECT_H

#include "numeric_id.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace ark {
class JsonObjectBuilder;
}  // namespace ark

namespace ark::tooling::inspector {
class PropertyDescriptor;

class RemoteObject {
public:
    static RemoteObject Undefined()
    {
        return RemoteObject();
    }

    static RemoteObject Null()
    {
        return RemoteObject(nullptr);
    }

    static RemoteObject Boolean(bool boolean)
    {
        return RemoteObject(boolean);
    }

    static RemoteObject Number(int32_t number)
    {
        return RemoteObject(NumberT {number});
    }

    template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
    static RemoteObject Number(T number)
    {
        return RemoteObject(NumberT {number});
    }

    template <typename T,
              std::enable_if_t<std::is_integral_v<T> && std::is_signed_v<T> && sizeof(int32_t) < sizeof(T), int> = 0>
    static RemoteObject Number(T number)
    {
        if (INT32_MIN <= number && number <= INT32_MAX) {
            return RemoteObject(NumberT {static_cast<int32_t>(number)});
        }
        if (number < 0) {
            return RemoteObject(BigIntT {-1, -static_cast<uintmax_t>(number)});
        }
        return RemoteObject(BigIntT {1, static_cast<uintmax_t>(number)});
    }

    template <typename T, std::enable_if_t<std::is_unsigned_v<T> && sizeof(int32_t) <= sizeof(T), int> = 0>
    static RemoteObject Number(T number)
    {
        if (number <= INT32_MAX) {
            return RemoteObject(NumberT {static_cast<int32_t>(number)});
        }
        return RemoteObject(BigIntT {1, number});
    }

    static RemoteObject String(std::string string)
    {
        return RemoteObject(std::move(string));
    }

    static RemoteObject Symbol(std::string description)
    {
        return RemoteObject(SymbolT {std::move(description)});
    }

    static RemoteObject Object(std::string className, std::optional<RemoteObjectId> objectId = std::nullopt,
                               std::optional<std::string> description = std::nullopt)
    {
        return RemoteObject(ObjectT {std::move(className), objectId, std::move(description)});
    }

    static RemoteObject Array(std::string className, size_t length,
                              std::optional<RemoteObjectId> objectId = std::nullopt)
    {
        return RemoteObject(ArrayT {std::move(className), objectId, length});
    }

    static RemoteObject Function(std::string className, std::string name, size_t length,
                                 std::optional<RemoteObjectId> objectId = std::nullopt)
    {
        return RemoteObject(FunctionT {std::move(className), objectId, std::move(name), length});
    }

    std::optional<RemoteObjectId> GetObjectId() const;

    void GeneratePreview(const std::vector<PropertyDescriptor> &properties) const;

    std::function<void(JsonObjectBuilder &)> ToJson() const;

private:
    using NumberT = std::variant<int32_t, double>;
    struct BigIntT {
        int8_t sign;
        uintmax_t value;
    };
    struct SymbolT {
        std::string description;
    };
    struct ObjectT {
        std::string className;
        std::optional<RemoteObjectId> objectId;
        std::optional<std::string> description;
    };
    struct ArrayT {
        std::string className;
        std::optional<RemoteObjectId> objectId;
        size_t length;
    };
    struct FunctionT {
        std::string className;
        std::optional<RemoteObjectId> objectId;
        std::string name;
        size_t length;
    };

    using Value = std::variant<std::monostate, std::nullptr_t, bool, NumberT, BigIntT, std::string, SymbolT, ObjectT,
                               ArrayT, FunctionT>;

    class Type {
        friend class RemoteObject;

    public:
        static Type Accessor()
        {
            return Type("accessor");
        }

        std::function<void(JsonObjectBuilder &)> ToJson() const;

    private:
        explicit Type(const char *type, const char *subtype = nullptr) : type_(type), subtype_(subtype) {}

        const char *type_;
        const char *subtype_;
    };

    struct PreviewProperty {
        std::string name;
        Type type;
        std::optional<std::string> value;
        bool isEntry;
    };

    template <typename... T>
    explicit RemoteObject(T &&...value) : value_(std::forward<T>(value)...)
    {
    }

    static std::string GetDescription(const BigIntT &bigint);
    static std::string GetDescription(const ObjectT &object);
    static std::string GetDescription(const ArrayT &array);
    static std::string GetDescription(const FunctionT &function);
    Type GetType() const;
    std::function<void(JsonObjectBuilder &)> PreviewToJson() const;

    Value value_;
    mutable std::vector<PreviewProperty> preview_;
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_REMOTE_OBJECT_H
