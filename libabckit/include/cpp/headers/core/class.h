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
class Class : public View<AbckitCoreClass *> {
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
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<core::Function> *, const ApiConfig *>;
        EnumerateData enumerateData(&methods, conf);

        conf->cIapi_->classEnumerateMethods(GetView(), &enumerateData, [](AbckitCoreFunction *method, void *data) {
            auto *vec = static_cast<EnumerateData *>(data)->first;
            auto *config = static_cast<EnumerateData *>(data)->second;
            vec->push_back(core::Function(method, config));
            return true;
        });
    }

    inline void GetAllAnnotationsInner(std::vector<core::Annotation> &anns) const
    {
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<core::Annotation> *, const ApiConfig *>;
        EnumerateData enumerateData(&anns, conf);

        conf->cIapi_->classEnumerateAnnotations(GetView(), &enumerateData,
                                                [](AbckitCoreAnnotation *method, void *data) {
                                                    auto *vec = static_cast<EnumerateData *>(data)->first;
                                                    auto *config = static_cast<EnumerateData *>(data)->second;
                                                    vec->push_back(core::Annotation(method, config));
                                                    return true;
                                                });
    }

    Class(AbckitCoreClass *klass, const ApiConfig *conf) : View(klass), conf_(conf) {};
    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_CLASS_H
