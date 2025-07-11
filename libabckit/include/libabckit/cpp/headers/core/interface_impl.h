/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef CPP_ABCKIT_CORE_INTERFACE_IMPL_H
#define CPP_ABCKIT_CORE_INTERFACE_IMPL_H

#include "./interface.h"

namespace abckit::core {

inline std::string Interface::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->interfaceGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

inline std::vector<core::Function> Interface::GetAllMethods() const
{
    std::vector<core::Function> methods;

    GetAllMethodsInner(methods);

    CheckError(GetApiConfig());

    return methods;
}

inline std::vector<core::InterfaceField> Interface::GetFields() const
{
    std::vector<core::InterfaceField> fields;

    GetFieldsInner(fields);

    CheckError(GetApiConfig());

    return fields;
}

inline bool Interface::GetAllMethodsInner(std::vector<core::Function> &methods) const
{
    Payload<std::vector<core::Function> *> payload {&methods, GetApiConfig(), GetResource()};

    auto isNormalExit = GetApiConfig()->cIapi_->interfaceEnumerateMethods(
        GetView(), &payload, [](AbckitCoreFunction *method, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Function> *> *>(data);
            payload.data->push_back(core::Function(method, payload.config, payload.resource));
            return true;
        });
    return isNormalExit;
}

inline bool Interface::GetFieldsInner(std::vector<core::InterfaceField> &fields) const
{
    Payload<std::vector<core::InterfaceField> *> payload {&fields, GetApiConfig(), GetResource()};

    auto isNormalExit = GetApiConfig()->cIapi_->interfaceEnumerateFields(
        GetView(), &payload, [](AbckitCoreInterfaceField *field, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::InterfaceField> *> *>(data);
            payload.data->push_back(core::InterfaceField(field, payload.config));
            return true;
        });
    return isNormalExit;
}
}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_INTERFACE_IMPL_H
