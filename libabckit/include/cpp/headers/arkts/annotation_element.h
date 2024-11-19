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

#ifndef CPP_ABCKIT_ARKTS_ANNOTATION_ELEMENT_H
#define CPP_ABCKIT_ARKTS_ANNOTATION_ELEMENT_H

#include "../core/annotation_element.h"

namespace abckit::arkts {

/**
 * @brief AnnotationElement
 */
class AnnotationElement : public core::AnnotationElement {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class arkts::Class;
    /// @brief to access private constructor
    friend class arkts::Function;
    /// @brief to access private constructor
    friend class arkts::Annotation;
    /// @brief abckit::DefaultHash<AnnotationElement>
    friend class abckit::DefaultHash<AnnotationElement>;

public:
    /**
     * @brief Construct a new Annotation Element object
     * @param other
     */
    AnnotationElement(const AnnotationElement &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationElement&
     */
    AnnotationElement &operator=(const AnnotationElement &other) = default;

    /**
     * @brief Construct a new Annotation Element object
     * @param other
     */
    AnnotationElement(AnnotationElement &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return AnnotationElement&
     */
    AnnotationElement &operator=(AnnotationElement &&other) = default;

    // CC-OFFNXT(G.FMT.02) project code style
    /**
     * @brief Construct a new Annotation Element object
     * @param coreOther
     */
    explicit AnnotationElement(const core::AnnotationElement &coreOther) : core::AnnotationElement(coreOther) {};

    /**
     * @brief Destroy the Annotation Element object
     */
    ~AnnotationElement() override = default;

    /**
     * @brief Get the Name object
     * @return std::string_view
     */
    std::string_view GetName() const;

    // Other API.
    // ...
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_ANNOTATION_ELEMENT_H
