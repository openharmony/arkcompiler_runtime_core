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

#ifndef CPP_ABCKIT_FILE_IMPL_H
#define CPP_ABCKIT_FILE_IMPL_H

#include "./file.h"

namespace abckit {

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<core::Module> File::GetModules() const
{
    std::vector<core::Module> modules;
    Payload<std::vector<core::Module> *> payload {&modules, GetApiConfig(), this};

    GetApiConfig()->cIapi_->fileEnumerateModules(GetResource(), &payload, [](AbckitCoreModule *module, void *data) {
        const auto &payload = *static_cast<Payload<std::vector<core::Module> *> *>(data);
        payload.data->push_back(core::Module(module, payload.config, payload.resource));
        return true;
    });

    CheckError(GetApiConfig());

    return modules;
}

// CC-OFFNXT(G.FUD.06) perf critical
inline std::vector<core::Function> File::GetAllFunctions() const
{
    std::vector<core::Function> functions;

    for (auto &module : GetModules()) {
        for (auto &klass : module.GetClasses()) {
            for (auto &function : klass.GetAllMethods()) {
                functions.push_back(function);
            }
        }
        for (auto &function : module.GetTopLevelFunctions()) {
            functions.push_back(function);
        }
    }

    CheckError(GetApiConfig());

    return functions;
}

inline abckit::LiteralArray File::CreateLiteralArray(const std::vector<abckit::Literal> &literals) const
{
    std::vector<AbckitLiteral *> litsImpl;
    litsImpl.reserve(literals.size());
    for (const auto &literal : literals) {
        litsImpl.push_back(literal.GetView());
    }
    AbckitLiteralArray *litaIml =
        GetApiConfig()->cMapi_->createLiteralArray(GetResource(), litsImpl.data(), litsImpl.size());
    CheckError(GetApiConfig());
    return abckit::LiteralArray(litaIml, GetApiConfig(), this);
}

inline bool File::EnumerateModules(const std::function<bool(core::Module)> &cb) const
{
    std::vector<core::Module> modules;
    Payload<const std::function<bool(core::Module)> &> payload {cb, GetApiConfig(), this};

    auto isNormalExit =
        GetApiConfig()->cIapi_->fileEnumerateModules(GetResource(), &payload, [](AbckitCoreModule *module, void *data) {
            const auto &payload = *static_cast<Payload<const std::function<bool(core::Module)> &> *>(data);
            return payload.data(core::Module(module, payload.config, payload.resource));
        });

    CheckError(GetApiConfig());
    return isNormalExit;
}

}  // namespace abckit

#endif  // CPP_ABCKIT_FILE_IMPL_H
