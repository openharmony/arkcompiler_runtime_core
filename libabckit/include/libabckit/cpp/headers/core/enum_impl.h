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

#ifndef CPP_ABCKIT_CORE_ENUM_IMPL_H
#define CPP_ABCKIT_CORE_ENUM_IMPL_H

#include "./enum.h"

namespace abckit::core {

inline std::string Enum::GetName() const
{
    const ApiConfig *conf = GetApiConfig();
    AbckitString *cString = conf->cIapi_->enumGetName(GetView());
    CheckError(conf);
    std::string str = conf->cIapi_->abckitStringToString(cString);
    CheckError(conf);
    return str;
}

inline std::vector<core::Function> Enum::GetAllMethods() const
{
    std::vector<core::Function> methods;

    GetAllMethodsInner(methods);

    CheckError(GetApiConfig());

    return methods;
}

inline std::vector<core::EnumField> Enum::GetFields() const
{
    std::vector<core::EnumField> fields;

    GetFieldsInner(fields);

    CheckError(GetApiConfig());

    return fields;
}

inline bool Enum::GetAllMethodsInner(std::vector<core::Function> &methods) const
{
    Payload<std::vector<core::Function> *> payload {&methods, GetApiConfig(), GetResource()};

    auto isNormalExit =
        GetApiConfig()->cIapi_->enumEnumerateMethods(GetView(), &payload, [](AbckitCoreFunction *method, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Function> *> *>(data);
            payload.data->push_back(core::Function(method, payload.config, payload.resource));
            return true;
        });
    return isNormalExit;
}

inline bool Enum::GetFieldsInner(std::vector<core::EnumField> &fields) const
{
    Payload<std::vector<core::EnumField> *> payload {&fields, GetApiConfig(), GetResource()};

    auto isNormalExit =
        GetApiConfig()->cIapi_->enumEnumerateFields(GetView(), &payload, [](AbckitCoreEnumField *field, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::EnumField> *> *>(data);
            payload.data->push_back(core::EnumField(field, payload.config));
            return true;
        });
    return isNormalExit;
}
}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_ENUM_IMPL_H
