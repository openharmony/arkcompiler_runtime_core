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

#ifndef CPP_ABCKIT_LITERAL_H
#define CPP_ABCKIT_LITERAL_H

#include "./base_classes.h"
#include "./literal_array.h"

namespace abckit {

/**
 * @brief Literal
 */
class Literal : public ViewInResource<AbckitLiteral *, const File *> {
    /// @brief abckit::File
    friend class abckit::File;
    /// @brief abckit::DefaultHash<Literal>
    friend class abckit::DefaultHash<Literal>;

public:
    /**
     * @brief Construct a new Literal object
     * @param other
     */
    Literal(const Literal &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Literal&
     */
    Literal &operator=(const Literal &other) = default;

    /**
     * @brief Construct a new Literal object
     * @param other
     */
    Literal(Literal &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Literal&
     */
    Literal &operator=(Literal &&other) = default;

    /**
     * @brief Destroy the Literal object
     */
    ~Literal() override = default;

    /**
     * @brief Get the Bool object
     * @return bool
     */
    bool GetBool() const;

    /**
     * @brief Get the Literal Array object
     * @return abckit::LiteralArray
     */
    abckit::LiteralArray GetLiteralArray() const;

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
    Literal(AbckitLiteral *lit, const ApiConfig *conf, const File *file) : ViewInResource(lit), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_LITERAL_H
