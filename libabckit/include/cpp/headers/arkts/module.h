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

#ifndef CPP_ABCKIT_ARKTS_MODULE_H
#define CPP_ABCKIT_ARKTS_MODULE_H

#include "../core/module.h"
#include "../base_concepts.h"
#include "./import_descriptor.h"

#include <string_view>

namespace abckit::arkts {

/**
 * @brief Module
 */
class Module final : public core::Module {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class abckit::File;
    /// @brief abckit::DefaultHash<Module>
    friend class abckit::DefaultHash<Module>;
    /// @brief to access private TargetCast
    friend class abckit::traits::TargetCheckCast<Module>;

public:
    /**
     * @brief Constructor Arkts API Module from the Core API with compatibility check
     * @param other - Core API Module
     */
    explicit Module(const core::Module &other);

    /**
     * @brief Construct a new Module object
     * @param other
     */
    Module(const Module &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Module&
     */
    Module &operator=(const Module &other) = default;

    /**
     * @brief Construct a new Module object
     * @param other
     */
    Module(Module &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Module&
     */
    Module &operator=(Module &&other) = default;

    /**
     * @brief Destroy the Module object
     */
    ~Module() override = default;
    // Other API.
    // ...

    /**
     * @brief Adds import from one ArktsV1 module to another ArktsV1 module.
     * @return Pointer to the newly created import descriptor.
     * @param [ in ] imported - The module the `importing` module imports from.
     * @param [ in ] name - Import name. For namespace imports equals to "*". For default imports equals to "default".
     * For regular imports is the same as in user code.
     * @param [ in ] alias - Alias name for the import. For namespace imports is the same as in user code. For default
     * import is the same as the default import name in user code. For regular imports is the same as in user code.
     * @note Allocates
     */
    ImportDescriptor AddImportFromArktsV1ToArktsV1(const Module &imported, std::string_view name,
                                                   std::string_view alias);

private:
    /**
     * @brief Converts underlying module from Core to Arkts target
     * @return AbckitArktsModule* - converted module
     * @note Set `ABCKIT_STATUS_WRONG_TARGET` error if `this` does not have `ABCKIT_TARGET_ARK_TS_V1` or
     * `ABCKIT_TARGET_ARK_TS_V2` target.
     */
    AbckitArktsModule *TargetCast() const;

    ABCKIT_NO_UNIQUE_ADDRESS traits::TargetCheckCast<Module> targetChecker_;
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_MODULE_H
