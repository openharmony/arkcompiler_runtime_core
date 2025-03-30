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

#ifndef CPP_ABCKIT_ARKTS_FIELD_H
#define CPP_ABCKIT_ARKTS_FIELD_H

#include "../core/field.h"

namespace abckit::arkts {

/**
 * @brief Field
 */
class Field final : public core::Field {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class abckit::core::Field;
    /// @brief abckit::DefaultHash<Field>
    friend class abckit::DefaultHash<Field>;

public:
    /**
     * @brief Construct a new Field object
     * @param other
     */
    Field(const Field &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Field&
     */
    Field &operator=(const Field &other) = default;

    /**
     * @brief Construct a new Field object
     * @param other
     */
    Field(Field &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Field&
     */
    Field &operator=(Field &&other) = default;

    /**
     * @brief Destroy the Field object
     */
    ~Field() override = default;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_FIELD_H
