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

#ifndef CPP_ABCKIT_CORE_NAMESPACE_IMPL_H
#define CPP_ABCKIT_CORE_NAMESPACE_IMPL_H

#include "./namespace.h"
#include "./function.h"
#include "./class.h"

namespace abckit::core {

// CC-OFFNXT(G.FUD.06) perf critical
inline bool Namespace::EnumerateNamespaces(const std::function<bool(core::Namespace)> &cb) const
{
    Payload<const std::function<bool(core::Namespace)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit = GetApiConfig()->cIapi_->namespaceEnumerateNamespaces(
        GetView(), &payload, [](AbckitCoreNamespace *ns, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Namespace)> &> *>(data);
            return payload.data(core::Namespace(ns, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline bool Namespace::EnumerateClasses(const std::function<bool(core::Class)> &cb) const
{
    Payload<const std::function<bool(core::Class)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit =
        GetApiConfig()->cIapi_->namespaceEnumerateClasses(GetView(), &payload, [](AbckitCoreClass *ns, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Class)> &> *>(data);
            return payload.data(core::Class(ns, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline bool Namespace::EnumerateTopLevelFunctions(const std::function<bool(core::Function)> &cb) const
{
    Payload<const std::function<bool(core::Function)> &> payload {cb, GetApiConfig(), GetResource()};

    bool isNormalExit = GetApiConfig()->cIapi_->namespaceEnumerateTopLevelFunctions(
        GetView(), &payload, [](AbckitCoreFunction *func, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Function)> &> *>(data);
            return payload.data(core::Function(func, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
    return isNormalExit;
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_NAMESPACE_IMPL_H
