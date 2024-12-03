/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef CPP_ABCKIT_VALUE_H
#define CPP_ABCKIT_VALUE_H

#include "./base_classes.h"

namespace abckit {

/**
 * @brief Value
 */
class Value : public ViewInResource<AbckitValue *, const File *> {
    /// @brief abckit::File
    friend class abckit::File;
    /// @brief abckit::core::Annotation
    friend class abckit::core::Annotation;
    /// @brief for access to a private constructor
    friend class abckit::core::AnnotationElement;
    /// @brief abckit::arkts::Annotation
    friend class abckit::arkts::Annotation;
    /// @brief abckit::DefaultHash<Value>
    friend class abckit::DefaultHash<Value>;

public:
    /**
     * @brief Construct a new Value object
     * @param other
     */
    Value(const Value &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Value&
     */
    Value &operator=(const Value &other) = default;

    /**
     * @brief Construct a new Value object
     * @param other
     */
    Value(Value &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Value&
     */
    Value &operator=(Value &&other) = default;

    /**
     * @brief Destroy the Value object
     */
    ~Value() override = default;

    /**
     * @brief Get the U1 value
     * @return bool
     */
    bool GetU1() const;

    /**
     * @brief Get the Double object
     * @return double
     */
    double GetDouble() const;

    /**
     * @brief Returns string value that given `Value` holds.
     * @return std::string_view of the stored value.
     * @note Set `ABCKIT_STATUS_TODO` error if `value` type id differs from `ABCKIT_TYPE_ID_STRING`.
     * @note Allocates
     */
    std::string_view GetString() const;

protected:
    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }

private:
    explicit Value(AbckitValue *val, const ApiConfig *conf, const File *file) : ViewInResource(val), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_VALUE_H
