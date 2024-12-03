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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_H
#define CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_H

#include "../base_classes.h"

#include <string_view>

namespace abckit::core {

/**
 * @brief AnnotationInterfaceField
 */
class AnnotationInterfaceField : public ViewInResource<AbckitCoreAnnotationInterfaceField *, const File *> {
    /// @brief core::Annotation
    friend class core::Annotation;
    /// @brief arkts::Annotation
    friend class arkts::Annotation;
    /// @brief core::AnnotationInterface
    friend class core::AnnotationInterface;
    /// @brief arkts::AnnotationInterface
    friend class arkts::AnnotationInterface;
    /// @brief abckit::DefaultHash<AnnotationInterfaceField>
    friend class abckit::DefaultHash<AnnotationInterfaceField>;

protected:
    /// @brief Core API View type
    using CoreViewT = AnnotationInterfaceField;

public:
    /**
     * @brief Construct a new Annotation Interface Field object
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

    /**
     * @brief Destroy the Annotation Interface Field object
     */
    ~AnnotationInterfaceField() override = default;

    // Core API's.
    // ...

    /**
     * @brief Get the Name object
     * @return std::string_view
     */
    std::string_view GetName();

private:
    AnnotationInterfaceField(AbckitCoreAnnotationInterfaceField *anni, const ApiConfig *conf, const File *file)
        : ViewInResource(anni), conf_(conf)
    {
        SetResource(file);
    };
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

#endif  // CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_FIELD_H
