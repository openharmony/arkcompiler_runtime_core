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

#ifndef CPP_ABCKIT_CORE_NAMESPACE_H
#define CPP_ABCKIT_CORE_NAMESPACE_H

#include "../base_classes.h"

#include <string_view>

namespace abckit::core {

/**
 * @brief Namespace
 */
class Namespace : public View<AbckitCoreNamespace *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief to access private constructor
    friend class Module;

public:
    /**
     * @brief Construct a new Namespace object
     * @param other
     */
    Namespace(const Namespace &other) = default;

    /**
     * @brief
     * @param other
     * @return Namespace&
     */
    Namespace &operator=(const Namespace &other) = default;

    /**
     * @brief Construct a new Namespace object
     * @param other
     */
    Namespace(Namespace &&other) = default;

    /**
     * @brief
     * @param other
     * @return Namespace&
     */
    Namespace &operator=(Namespace &&other) = default;

    /**
     * @brief Destroy the Namespace object
     */
    ~Namespace() override = default;

    /**
     * @brief Get the Name object
     * @return std::string_view
     */
    std::string_view GetName() const
    {
        AbckitString *abcName = GetApiConfig()->cIapi_->namespaceGetName(GetView());
        CheckError(GetApiConfig());
        std::string_view name = GetApiConfig()->cIapi_->abckitStringToString(abcName);
        CheckError(GetApiConfig());
        return name;
    }

    // Core API's.
    // ...

private:
    Namespace(AbckitCoreNamespace *ns, const ApiConfig *conf) : View(ns), conf_(conf) {};
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

#endif  // CPP_ABCKIT_CORE_NAMESPACE_H
