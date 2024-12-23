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

#include "../../c/abckit.h"
#include "../../c/metadata_core.h"
#include "../../c/ir_core.h"
#include "../../c/isa/isa_dynamic.h"
#include "../../../src/include_v2/c/isa/isa_static.h"
#include "../../c/extensions/arkts/metadata_arkts.h"
#include "../../c/extensions/js/metadata_js.h"

#include "./utils.h"

#include <cstdint>
#include <memory>

namespace abckit {

class DynamicIsa;
class StaticIsa;
class File;
class Graph;
class BasicBlock;
class Instruction;
class Value;
class Type;
class Literal;
class LiteralArray;
class Type;

namespace core {
class Module;
class Namespace;
class Class;
class Function;
class Field;
class Annotation;
class AnnotationInterface;
class AnnotationElement;
class AnnotationInterfaceField;
class ImportDescriptor;
class ExportDescriptor;
}  // namespace core

namespace arkts {
class Module;
class Namespace;
class Class;
class Function;
class Field;
class Annotation;
class AnnotationInterface;
class AnnotationElement;
class AnnotationInterfaceField;
class ImportDescriptor;
class ExportDescriptor;
}  // namespace arkts

namespace js {
class Module;
class ImportDescriptor;
class ExportDescriptor;
}  // namespace js

// Class containing pointers to underlying C API's,
// hides C implementation from C++ API user
/**
 * @brief ApiConfig
 */
class ApiConfig final {
    // Befrend with all core entities so they have an access to config
    friend DynamicIsa;
    friend StaticIsa;

    friend File;
    friend Graph;
    friend BasicBlock;
    friend Instruction;
    friend Value;
    friend Type;
    friend Literal;
    friend LiteralArray;
    friend Type;

    /// \relates abckit::core::Module
    friend core::Module;
    /// \relates abckit::core::Namespace
    friend core::Namespace;
    /// \relates abckit::core::Class
    friend core::Class;
    /// \relates abckit::core::Function
    friend core::Function;
    /// \relates abckit::core::Field
    friend core::Field;
    /// \relates abckit::core::Annotation
    friend core::Annotation;
    /// \relates abckit::core::AnnotationInterface
    friend core::AnnotationInterface;
    /// \relates abckit::core::AnnotationElement
    friend core::AnnotationElement;
    /// \relates abckit::core::AnnotationInterfaceField
    friend core::AnnotationInterfaceField;
    /// \relates abckit::core::ImportDescriptor
    friend core::ImportDescriptor;
    /// \relates abckit::core::ExportDescriptor
    friend core::ExportDescriptor;

    /// \relates abckit::arkts::Module
    friend arkts::Module;
    /// \relates abckit::arkts::Namespace
    friend arkts::Namespace;
    /// \relates abckit::arkts::Class
    friend arkts::Class;
    /// \relates abckit::arkts::Function
    friend arkts::Function;
    /// \relates abckit::arkts::Field
    friend arkts::Field;
    /// \relates abckit::arkts::Annotation
    friend arkts::Annotation;
    /// \relates abckit::arkts::AnnotationInterface
    friend arkts::AnnotationInterface;
    /// \relates abckit::arkts::AnnotationElement
    friend arkts::AnnotationElement;
    /// \relates abckit::arkts::AnnotationInterfaceField
    friend arkts::AnnotationInterfaceField;
    /// \relates abckit::arkts::ImportDescriptor
    friend arkts::ImportDescriptor;
    /// \relates abckit::arkts::ExportDescriptor
    friend arkts::ExportDescriptor;

    /// \relates abckit::js::Module
    friend js::Module;
    /// \relates abckit::js::ImportDescriptor
    friend js::ImportDescriptor;
    /// \relates abckit::js::ExportDescriptor
    friend js::ExportDescriptor;

    /**
     * Check error
     * @param conf
     */
    friend void CheckError(const ApiConfig *conf);

public:
    /**
     * @brief Deleted constructor
     * @param other
     */
    ApiConfig(const ApiConfig &other) = delete;

    /**
     * @brief Deleted constructor
     * @param other
     * @return ApiConfig
     */
    ApiConfig &operator=(const ApiConfig &other) = delete;

    /**
     * @brief Deleted constructor
     * @param other
     */
    ApiConfig(ApiConfig &&other) = delete;

    /**
     * @brief Deleted constructor
     * @param other
     * @return ApiConfig
     */
    ApiConfig &operator=(ApiConfig &&other) = delete;

    /**
     * @brief Destructor
     */
    ~ApiConfig() = default;

protected:
    /**
     * @brief Constructor
     * @param eh - unique ptr to IErrorHandler
     */
    explicit ApiConfig(std::unique_ptr<IErrorHandler> eh)
        : cApi_(AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
          cIapi_(AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
          cMapi_(AbckitGetModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
          cArktsIapi_(AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
          cArktsMapi_(AbckitGetArktsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
          cJsIapi_(AbckitGetJsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
          cJsMapi_(AbckitGetJsModifyApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)),
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
    const AbckitJsInspectApi *cJsIapi_;
    const AbckitJsModifyApi *cJsMapi_;
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

// Class implement default hash function for all View's
/**
 * @brief DefaultHash<T>
 */
template <typename T>
class DefaultHash {
public:
    /**
     * Operator ()
     * @param t
     * @return uintptr_t
     */
    virtual uintptr_t operator()(const T &t) const
    {
        return reinterpret_cast<uintptr_t>(t.GetView());
    }
};

}  // namespace abckit

#endif  // CPP_ABCKIT_CONFIG_H
