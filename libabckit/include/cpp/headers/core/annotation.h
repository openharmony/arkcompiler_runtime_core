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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_H
#define CPP_ABCKIT_CORE_ANNOTATION_H

#include "./annotation_interface.h"
#include "../base_classes.h"

namespace abckit::core {

/**
 * @brief Annotation
 */
class Annotation : public View<AbckitCoreAnnotation *> {
    /// @brief core::Function
    friend class core::Function;
    /// @brief arkts::Function
    friend class arkts::Function;
    /// @brief core::Class
    friend class core::Class;
    /// @brief arkts::Class
    friend class arkts::Class;
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

    /**
     * @brief Destroy the Annotation object
     */
    ~Annotation() override = default;

    // Core API's.
    // ...

    /**
     * @brief Get the Interface object
     * @return core::AnnotationInterface
     */
    core::AnnotationInterface GetInterface()
    {
        AnnotationInterface iface(GetApiConfig()->cIapi_->annotationGetInterface(GetView()), GetApiConfig());
        CheckError(GetApiConfig());
        return iface;
    }

private:
    Annotation(AbckitCoreAnnotation *ann, const ApiConfig *conf) : View(ann), conf_(conf) {};
    const ApiConfig *conf_;

protected:
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

#endif  // CPP_ABCKIT_CORE_ANNOTATION_H
