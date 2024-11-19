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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_ELEMENT_H
#define CPP_ABCKIT_CORE_ANNOTATION_ELEMENT_H

#include "../base_classes.h"

#include <string_view>

namespace abckit::core {

/**
 * @brief AnnotationElement
 */
class AnnotationElement : public View<AbckitCoreAnnotationElement *> {
    /// @brief core::Annotation
    friend class core::Annotation;
    /// @brief arkts::Annotation
    friend class arkts::Annotation;
    /// @brief core::Module
    friend class core::Module;
    /// @brief arkts::Module
    friend class arkts::Module;
    /// @brief abckit::DefaultHash<AnnotationElement>
    friend class abckit::DefaultHash<AnnotationElement>;

public:
    /**
     * @brief Constructor
     * @param other
     */
    AnnotationElement(const AnnotationElement &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return this
     */
    AnnotationElement &operator=(const AnnotationElement &other) = default;

    /**
     * @brief Destructor
     */
    ~AnnotationElement() override = default;

    // Core API's.
    // ...

private:
    /**
     * Constructor
     * @param conf
     * @param anne
     */
    AnnotationElement(AbckitCoreAnnotationElement *anne, const ApiConfig *conf) : View(anne), conf_(conf) {};
    const ApiConfig *conf_;

protected:
    /**
     * @brief Constructor
     * @param other
     */
    AnnotationElement(AnnotationElement &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return this
     */
    AnnotationElement &operator=(AnnotationElement &&other) = default;

    /**
     * @brief Get the Api Config object
     * @return const ApiConfig*
     */
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_ANNOTATION_ELEMENT_H
