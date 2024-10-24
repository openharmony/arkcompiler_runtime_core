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

#include "libabckit/include/c/abckit.h"
#include "cpp/headers/declarations.h"
#include "cpp/headers/config.h"
#include "cpp/headers/base_classes.h"
#include "cpp/headers/core/module.h"
#include "cpp/headers/core/export_descriptor.h"

namespace abckit::arkts {

class ExportDescriptor final : public core::ExportDescriptor {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    friend class core::Module;
    friend class arkts::Module;

public:
    ExportDescriptor(const ExportDescriptor &other) = default;
    ExportDescriptor &operator=(const ExportDescriptor &other) = default;
    ExportDescriptor(ExportDescriptor &&other) = default;
    ExportDescriptor &operator=(ExportDescriptor &&other) = default;
    ~ExportDescriptor() override = default;
    // Other API.
    // ...
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_EXPORT_DESCRIPTOR_H
