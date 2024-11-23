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

#ifndef CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_FIELD_H
#define CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_FIELD_H

#include "../core/annotation_interface_field.h"

namespace abckit::arkts {

/**
 * @brief AnnotationInterfaceField
 */
class AnnotationInterfaceField : public core::AnnotationInterfaceField {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.

    /// @brief to access private constructor
    friend class arkts::Annotation;
    /// @brief to access private constructor
    friend class arkts::AnnotationInterface;
    /// @brief abckit::DefaultHash<AnnotationInterfaceField>
    friend class abckit::DefaultHash<AnnotationInterfaceField>;

public:
    /**
     * @brief Construct a new Annotation Interface Field object
     *
     * @param other
     */
    AnnotationInterfaceField(const AnnotationInterfaceField &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationInterfaceField&
     */
    AnnotationInterfaceField &operator=(const AnnotationInterfaceField &other) = default;

    /**
     * @brief Construct a new Annotation Interface Field object
     * @param other
     */
    AnnotationInterfaceField(AnnotationInterfaceField &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationInterfaceField&
     */
    AnnotationInterfaceField &operator=(AnnotationInterfaceField &&other) = default;

    // CC-OFFNXT(G.FMT.02) project code style
    /**
     * @brief Construct a new Annotation Interface Field object
     * @param coreOther
     */
    explicit AnnotationInterfaceField(const core::AnnotationInterfaceField &coreOther)
        : core::AnnotationInterfaceField(coreOther) {};

    /**
     * @brief Destroy the Annotation Interface Field object
     */
    ~AnnotationInterfaceField() override = default;
    // Other API.
    // ...
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_FIELD_H
