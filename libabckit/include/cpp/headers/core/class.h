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

#ifndef CPP_ABCKIT_CORE_CLASS_H
#define CPP_ABCKIT_CORE_CLASS_H

#include "../base_classes.h"
#include "./function.h"

#include <functional>
#include <vector>

namespace abckit::core {

/**
 * @brief Class
 */
class Class : public ViewInResource<AbckitCoreClass *, const File *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class Module;
    /// @brief to access private constructor
    friend class Namespace;
    /// @brief to access private constructor
    friend class Function;
    /// @brief abckit::DefaultHash<Class>
    friend class abckit::DefaultHash<Class>;

public:
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
     * @brief Get Class name
     * @return std::string_view
     */
    std::string_view GetName() const;

    /**
     * @brief Get the All Methods object
     * @return std::vector<core::Function>
     */
    std::vector<core::Function> GetAllMethods() const;

    /**
     * @brief Get the Annotations object
     * @return std::vector<core::Annotation>
     */
    std::vector<core::Annotation> GetAnnotations() const;

    /**
     * @brief EnumerateMethods
     * @param cb
     */
    void EnumerateMethods(const std::function<bool(core::Function)> &cb) const;

    // Core API's.
    // ...

private:
    inline void GetAllMethodsInner(std::vector<core::Function> &methods) const
    {
        Payload<std::vector<core::Function> *> payload {&methods, GetApiConfig(), GetResource()};

        GetApiConfig()->cIapi_->classEnumerateMethods(GetView(), &payload, [](AbckitCoreFunction *method, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Function> *> *>(data);
            payload.data->push_back(core::Function(method, payload.config, payload.resource));
            return true;
        });
    }

    inline void GetAllAnnotationsInner(std::vector<core::Annotation> &anns) const
    {
        Payload<std::vector<core::Annotation> *> payload {&anns, GetApiConfig(), GetResource()};

        GetApiConfig()->cIapi_->classEnumerateAnnotations(
            GetView(), &payload, [](AbckitCoreAnnotation *method, void *data) {
                const auto &payload = *static_cast<Payload<std::vector<core::Annotation> *> *>(data);
                payload.data->push_back(core::Annotation(method, payload.config, payload.resource));
                return true;
            });
    }

    Class(AbckitCoreClass *klass, const ApiConfig *conf, const File *file) : ViewInResource(klass), conf_(conf)
    {
        SetResource(file);
    };
    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_CLASS_H
