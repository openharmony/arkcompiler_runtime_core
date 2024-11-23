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

#ifndef CPP_ABCKIT_ARKTS_ANNOTATION_H
#define CPP_ABCKIT_ARKTS_ANNOTATION_H

#include "../core/annotation.h"

namespace abckit::arkts {

/**
 * @brief Annotation
 */
class Annotation : public core::Annotation {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class arkts::Class;
    /// @brief to access private constructor
    friend class arkts::Function;
    /// @brief abckit::DefaultHash<Annotation>
    friend class abckit::DefaultHash<Annotation>;

public:
    /**
     * @brief Construct a new Annotation object
     * @param other
     */
    Annotation(const Annotation &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Annotation&
     */
    Annotation &operator=(const Annotation &other) = default;

    /**
     * @brief Construct a new Annotation object
     * @param other
     */
    Annotation(Annotation &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Annotation&
     */
    Annotation &operator=(Annotation &&other) = default;

    // CC-OFFNXT(G.FMT.02) project code style
    /**
     * @brief Construct a new Annotation object
     * @param coreOther
     */
    explicit Annotation(const core::Annotation &coreOther) : core::Annotation(coreOther) {};

    /**
     * @brief Destroy the Annotation object
     */
    ~Annotation() override = default;

    /**
     * @brief Add element
     * @param val
     * @param name
     * @return arkts::Annotation&
     */
    arkts::Annotation &AddElement(const abckit::Value &val, const std::string &name);

    /**
     * @brief add and get element
     * @param val
     * @param name
     * @return arkts::AnnotationElement
     */
    arkts::AnnotationElement AddAndGetElement(const abckit::Value &val, const std::string_view name);

    /**
     * @brief add and get element impl
     * @param params
     * @return AbckitCoreAnnotationElement*
     */
    AbckitCoreAnnotationElement *AddAndGetElementImpl(AbckitArktsAnnotationElementCreateParams *params);

    // Other API.
    // ...
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_ANNOTATION_H
