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

#ifndef CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_FIELD_H
#define CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_FIELD_H

#include "libabckit/include/c/abckit.h"
#include "cpp/headers/declarations.h"
#include "cpp/headers/config.h"
#include "cpp/headers/base_classes.h"
#include "cpp/headers/core/annotation_interface_field.h"

namespace abckit::arkts {

class AnnotationInterfaceField : public core::AnnotationInterfaceField {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    friend class arkts::Annotation;
    friend class arkts::AnnotationInterface;

public:
    AnnotationInterfaceField(const AnnotationInterfaceField &other) = default;
    AnnotationInterfaceField &operator=(const AnnotationInterfaceField &other) = default;
    AnnotationInterfaceField(AnnotationInterfaceField &&other) = default;
    AnnotationInterfaceField &operator=(AnnotationInterfaceField &&other) = default;

    // CC-OFFNXT(G.FMT.02) project code style
    explicit AnnotationInterfaceField(const core::AnnotationInterfaceField &coreOther)
        : core::AnnotationInterfaceField(coreOther) {};

    ~AnnotationInterfaceField() override = default;
    // Other API.
    // ...
};

}  // namespace abckit::arkts

#endif  // CPP_ABCKIT_ARKTS_ANNOTATION_INTERFACE_FIELD_H
