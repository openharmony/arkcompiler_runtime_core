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
#ifndef CPP_ABCKIT_CONFIG_H
#define CPP_ABCKIT_CONFIG_H

#include "libabckit/include/c/abckit.h"
#include "libabckit/include/c/metadata_core.h"
#include "libabckit/include/c/ir_core.h"
#include "libabckit/include/c/isa/isa_dynamic.h"
#include "libabckit/src/include_v2/c/isa/isa_static.h"
#include "libabckit/include/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/include/c/extensions/js/metadata_js.h"

#include "cpp/headers/declarations.h"
#include "cpp/headers/utils.h"

#include <memory>

namespace abckit {

// Class containing pointers to underlying C API's,
// hides C implementation from C++ API user
class ApiConfig final {
    // Befrend with all core entities so they have an access to config
    friend DynamicIsa;
    friend StaticIsa;

    friend File;
    friend Graph;
    friend BasicBlock;
    friend Instruction;
    friend Value;
    friend Literal;
    friend LiteralArray;

    friend core::Module;
    friend core::Namespace;
    friend core::Class;
    friend core::Function;
    friend core::Field;
    friend core::Annotation;
    friend core::AnnotationInterface;
    friend core::AnnotationElement;
    friend core::AnnotationInterfaceField;
    friend core::ImportDescriptor;
    friend core::ExportDescriptor;

    friend arkts::Module;
    friend arkts::Namespace;
    friend arkts::Class;
    friend arkts::Function;
    friend arkts::Field;
    friend arkts::Annotation;
    friend arkts::AnnotationInterface;
    friend arkts::AnnotationElement;
    friend arkts::AnnotationInterfaceField;
    friend arkts::ImportDescriptor;
    friend arkts::ExportDescriptor;

    friend void CheckError(const ApiConfig *conf);

public:
    ApiConfig(const ApiConfig &other) = delete;
    ApiConfig &operator=(const ApiConfig &other) = delete;
    ApiConfig(ApiConfig &&other) = delete;
    ApiConfig &operator=(ApiConfig &&other) = delete;
    ~ApiConfig() = default;

protected:
    explicit ApiConfig(std::unique_ptr<IErrorHandler> eh)
        : cApi_(AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
          cIapi_(AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
          cMapi_(AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
          cArktsIapi_(AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
          cArktsMapi_(AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
          cGapi_(AbckitGetGraphApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
          cDynapi_(AbckitGetIsaApiDynamicImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
          cStatapi_(AbckitGetIsaApiStaticImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
          eh_(std::move(eh)) {};

private:
    // NOTE(nsizov): make getters for these pointers
    const AbckitApi *cApi_;
    const AbckitInspectApi *cIapi_;
    const AbckitModifyApi *cMapi_;
    const AbckitArktsInspectApi *cArktsIapi_;
    const AbckitArktsModifyApi *cArktsMapi_;
    const AbckitGraphApi *cGapi_;
    const AbckitIsaApiDynamic *cDynapi_;
    const AbckitIsaApiStatic *cStatapi_;
    const std::unique_ptr<IErrorHandler> eh_;
};

inline void CheckError(const ApiConfig *conf)
{
    const AbckitStatus status = conf->cApi_->getLastError();
    if (status != ABCKIT_STATUS_NO_ERROR) {
        conf->eh_->HandleError(abckit::Exception(status));
    }
}

}  // namespace abckit

#endif  // CPP_ABCKIT_CONFIG_H
