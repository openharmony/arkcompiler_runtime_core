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

#ifndef CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_H
#define CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_H

#include "../core/annotation_interface.h"
#include "../base_concepts.h"

namespace abckit::arkts {

/**
 * @brief AnnotationInterface
 */
class AnnotationInterface : public core::AnnotationInterface {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class arkts::Class;
    /// @brief to access private constructor
    friend class arkts::Function;
    /// @brief abckit::DefaultHash<AnnotationInterface>
    friend class abckit::DefaultHash<AnnotationInterface>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<AnnotationInterface>;

public:
    /**
     * @brief Constructor Arkts API AnnotationInterface from the Core API with compatibility check
     * @param other - Core API AnnotationInterface
     */
    explicit AnnotationInterface(const core::AnnotationInterface &other);

    /**
     * @brief Construct a new Annotation Interface object
     * @param other
     */
    AnnotationInterface(const AnnotationInterface &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationInterface&
     */
    AnnotationInterface &operator=(const AnnotationInterface &other) = default;

    /**
     * @brief Construct a new Annotation Interface object
     * @param other
     */
    AnnotationInterface(AnnotationInterface &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationInterface&
     */
    AnnotationInterface &operator=(AnnotationInterface &&other) = default;

    /**
     * @brief Destroy the Annotation Interface object
     */
    ~AnnotationInterface() override = default;
    // Other API.
    // ...

private:
    /**
     * @brief Converts annotation interface from Core to Arkts target
     * @return AbckitArktsAnnotationInterface* - converted annotation interface
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` is does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsAnnotationInterface *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<AnnotationInterface> targetChecker_;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_H
