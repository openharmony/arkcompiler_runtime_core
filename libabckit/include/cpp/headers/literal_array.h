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

#ifndef CPP_ABCKIT_LITERAL_ARRAY_H
#define CPP_ABCKIT_LITERAL_ARRAY_H

#include "./base_classes.h"

namespace abckit {

/**
 * @brief LiteralArray
 */
class LiteralArray : public View<AbckitLiteralArray *> {
    /// @brief abckit::File
    friend class abckit::File;
    /// @brief abckit::Literal
    friend class abckit::Literal;
    /// @brief abckit::DefaultHash<LiteralArray>
    friend class abckit::DefaultHash<LiteralArray>;

public:
    /**
     * @brief Construct a new Literal Array object
     * @param other
     */
    LiteralArray(const LiteralArray &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return LiteralArray&
     */
    LiteralArray &operator=(const LiteralArray &other) = default;

    /**
     * @brief Construct a new Literal Array object
     * @param other
     */
    LiteralArray(LiteralArray &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return LiteralArray&
     */
    LiteralArray &operator=(LiteralArray &&other) = default;

    /**
     * @brief Destroy the Literal Array object
     *
     */
    ~LiteralArray() override = default;

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
    LiteralArray(AbckitLiteralArray *lita, const ApiConfig *conf) : View(lita), conf_(conf) {};
    const ApiConfig *conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_LITERAL_ARRAY_H
