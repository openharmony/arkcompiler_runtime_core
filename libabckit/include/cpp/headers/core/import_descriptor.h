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

#ifndef CPP_ABCKIT_CORE_IMPORT_DESCRIPTOR_H
#define CPP_ABCKIT_CORE_IMPORT_DESCRIPTOR_H

#include "../base_classes.h"

#include <string_view>

namespace abckit::core {

class ImportDescriptor : public View<AbckitCoreImportDescriptor *> {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    friend class abckit::File;
    friend class abckit::core::Module;
    friend class abckit::arkts::Module;
    friend class abckit::DynamicIsa;

public:
    ImportDescriptor(const ImportDescriptor &other) = default;
    ImportDescriptor &operator=(const ImportDescriptor &other) = default;
    ImportDescriptor(ImportDescriptor &&other) = default;
    ImportDescriptor &operator=(ImportDescriptor &&other) = default;
    ~ImportDescriptor() override = default;

    std::string_view GetName() const;

    // Core API's.
    core::Module GetImportedModule() const;

private:
    ImportDescriptor(AbckitCoreImportDescriptor *module, const ApiConfig *conf) : View(module), conf_(conf) {};
    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_IMPORT_DESCRIPTOR_H
