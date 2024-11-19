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

#ifndef CPP_ABCKIT_ARKTS_EXPORT_DESCRIPTOR_H
#define CPP_ABCKIT_ARKTS_EXPORT_DESCRIPTOR_H

#include "../core/export_descriptor.h"

namespace abckit::arkts {

/**
 * @brief ExportDescriptor
 */
class ExportDescriptor final : public core::ExportDescriptor {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class core::Module;
    /// @brief to access private constructor
    friend class arkts::Module;
    /// @brief abckit::DefaultHash<ExportDescriptor>
    friend class abckit::DefaultHash<ExportDescriptor>;

public:
    /**
     * @brief Construct a new Export Descriptor object
     * @param other
     */
    ExportDescriptor(const ExportDescriptor &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return ExportDescriptor&
     */
    ExportDescriptor &operator=(const ExportDescriptor &other) = default;

    /**
     * @brief Construct a new Export Descriptor object
     * @param other
     */
    ExportDescriptor(ExportDescriptor &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return ExportDescriptor&
     */
    ExportDescriptor &operator=(ExportDescriptor &&other) = default;

    /**
     * @brief Destroy the Export Descriptor object
     */
    ~ExportDescriptor() override = default;
    // Other API.
    // ...
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_EXPORT_DESCRIPTOR_H
