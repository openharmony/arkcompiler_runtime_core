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

#include "libabckit/include/c/abckit.h"
#include "cpp/headers/declarations.h"
#include "cpp/headers/config.h"
#include "cpp/headers/base_classes.h"
#include "cpp/headers/core/annotation_interface_field.h"
#include "libabckit/include/c/metadata_core.h"

#include <string>
#include <vector>
#include <string_view>

namespace abckit::core {

class AnnotationInterface : public View<AbckitCoreAnnotationInterface *> {
    friend class core::Annotation;
    friend class core::Module;

public:
    AnnotationInterface(const AnnotationInterface &other) = default;
    AnnotationInterface &operator=(const AnnotationInterface &other) = default;
    AnnotationInterface(AnnotationInterface &&other) = default;
    AnnotationInterface &operator=(AnnotationInterface &&other) = default;
    ~AnnotationInterface() override = default;

    // Core API's.
    // ...

    std::string_view GetName();
    std::vector<AnnotationInterfaceField> GetFields();

private:
    template <typename EnumerateData>
    inline void GetFieldsInner(EnumerateData enumerateData)
    {
        GetApiConfig()->cIapi_->annotationInterfaceEnumerateFields(
            GetView(), (void *)&enumerateData, [](AbckitCoreAnnotationInterfaceField *func, void *data) {
                auto *vec = static_cast<EnumerateData *>(data)->first;
                auto *config = static_cast<EnumerateData *>(data)->second;
                vec->push_back(core::AnnotationInterfaceField(func, config));
                return true;
            });
    }

    AnnotationInterface(AbckitCoreAnnotationInterface *anni, const ApiConfig *conf) : View(anni), conf_(conf) {};
    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_ANNOTATION_INTERFACE_H
