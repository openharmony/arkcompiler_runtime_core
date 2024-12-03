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

#ifndef CPP_ABCKIT_ARKTS_CLASS_H
#define CPP_ABCKIT_ARKTS_CLASS_H

#include "../core/class.h"
#include "../base_concepts.h"

namespace abckit::arkts {

/**
 * @brief Class
 */
class Class final : public core::Class {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class Module;
    /// @brief to access private constructor
    friend class Namespace;
    /// @brief abckit::DefaultHash<Class>
    friend class abckit::DefaultHash<Class>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<Class>;

public:
    /**
     * @brief Constructor Arkts API Class from the Core API with compatibility check
     * @param other - Core API Class
     */
    explicit Class(const core::Class &other);

    /**
     * @brief Construct a new Class object
     * @param other
     */
    Class(const Class &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Class&
     */
    Class &operator=(const Class &other) = default;

    /**
     * @brief Construct a new Class object
     * @param other
     */
    Class(Class &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Class&
     */
    Class &operator=(Class &&other) = default;

    /**
     * @brief Destroy the Class object
     */
    ~Class() override = default;

    /**
     * @brief Remove annotation from the class declaration.
     * @param [ in ] anno - Annotation to remove.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if current `Class` is false.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `anno` is false.
     * @note Set `ABCKIT_STATUS_UNSUPPORTED` error if current `Class` doesn't have `ABCKIT_TARGET_ARK_TS_V1` target.
     */
    void RemoveAnnotation(const arkts::Annotation &anno) const;

    // Other API.
    // ...

private:
    /**
     * @brief Converts class from Core to Arkts target
     * @return AbckitArktsClass* - converted class
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsClass *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<Class> targetChecker_;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_CLASS_H
