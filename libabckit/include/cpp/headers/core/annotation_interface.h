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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_H
#define CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_H

#include "../base_classes.h"
#include "./annotation_interface_field.h"

#include <vector>
#include <string_view>

namespace abckit::core {

/**
 * @brief AnnotationInterface
 */
class AnnotationInterface : public ViewInResource<AbckitCoreAnnotationInterface *, const File *> {
    /// @brief core::Annotation
    friend class core::Annotation;
    /// @brief core::Module
    friend class core::Module;
    /// @brief abckit::DefaultHash<AnnotationInterface>
    friend class abckit::DefaultHash<AnnotationInterface>;

protected:
    /// @brief Core API View type
    using CoreViewT = AnnotationInterface;

public:
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

    // Core API's.
    // ...

    /**
     * @brief Get the Name object
     * @return std::string_view
     */
    std::string_view GetName();

    /**
     * @brief Get the Fields object
     * @return std::vector<AnnotationInterfaceField>
     */
    std::vector<AnnotationInterfaceField> GetFields();

private:
    template <typename AnnotationPayload>
    inline void GetFieldsInner(AnnotationPayload annPayload)
    {
        GetApiConfig()->cIapi_->annotationInterfaceEnumerateFields(
            GetView(), &annPayload, [](AbckitCoreAnnotationInterfaceField *func, void *data) {
                const auto &payload = *static_cast<AnnotationPayload *>(data);
                payload.vec->push_back(core::AnnotationInterfaceField(func, payload.config, payload.file));
                return true;
            });
    }

    AnnotationInterface(AbckitCoreAnnotationInterface *anni, const ApiConfig *conf, const File *file)
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

#endif  // CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_H
