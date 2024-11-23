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
class AnnotationInterface : public View<AbckitCoreAnnotationInterface *> {
    /// @brief core::Annotation
    friend class core::Annotation;
    /// @brief core::Module
    friend class core::Module;
    /// @brief abckit::DefaultHash<AnnotationInterface>
    friend class abckit::DefaultHash<AnnotationInterface>;

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
    template <typename EnumerateData>
    inline void GetFieldsInner(EnumerateData enumerateData)
    {
        GetApiConfig()->cIapi_->annotationInterfaceEnumerateFields(
            GetView(), &enumerateData, [](AbckitCoreAnnotationInterfaceField *func, void *data) {
                auto *vec = static_cast<EnumerateData *>(data)->first;
                auto *config = static_cast<EnumerateData *>(data)->second;
                vec->push_back(core::AnnotationInterfaceField(func, config));
                return true;
            });
    }

    AnnotationInterface(AbckitCoreAnnotationInterface *anni, const ApiConfig *conf) : View(anni), conf_(conf) {};
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
