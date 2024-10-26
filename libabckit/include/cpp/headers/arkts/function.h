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

#ifndef CPP_ABCKIT_ARKTS_FUNCTION_H
#define CPP_ABCKIT_ARKTS_FUNCTION_H

#include "libabckit/include/c/abckit.h"
#include "cpp/headers/declarations.h"
#include "cpp/headers/config.h"
#include "cpp/headers/base_classes.h"
#include "cpp/headers/core/function.h"
#include "cpp/headers/core/annotation_interface.h"
#include "cpp/headers/arkts/annotation_interface.h"

namespace abckit::arkts {

class Function final : public core::Function {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    friend class Class;

public:
    Function(const Function &other) = default;
    Function &operator=(const Function &other) = default;
    Function(Function &&other) = default;
    Function &operator=(Function &&other) = default;

    // CC-OFFNXT(G.FMT.02) project code style
    explicit Function(const core::Function &coreOther) : core::Function(coreOther) {};

    ~Function() override = default;

    arkts::Function &AddAnnotation(const arkts::AnnotationInterface &iface);

    // Other API.
    // ...
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_FUNCTION_H
