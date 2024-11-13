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
    const ApiConfig *conf = GetApiConfig();
    std::vector<core::Module> modules;

    using EnumerateData = std::pair<std::vector<core::Module> *, const ApiConfig *>;
    EnumerateData enumerateData(&modules, conf);

    conf->cIapi_->fileEnumerateModules(GetResource(), &enumerateData, [](AbckitCoreModule *module, void *data) {
        auto *vec = static_cast<EnumerateData *>(data)->first;
        auto *config = static_cast<EnumerateData *>(data)->second;
        vec->push_back(core::Module(module, config));
        return true;
    });

    CheckError(conf);

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
    return abckit::LiteralArray(litaIml, GetApiConfig());
}

inline void File::EnumerateModules(const std::function<bool(core::Module)> &cb) const
{
    const ApiConfig *conf = GetApiConfig();

    using EnumerateData = std::pair<const std::function<bool(core::Module)> &, const ApiConfig *>;
    EnumerateData enumerateData(cb, conf);

    conf->cIapi_->fileEnumerateModules(GetResource(), &enumerateData, [](AbckitCoreModule *module, void *data) {
        const std::function<bool(core::Module)> &callback = static_cast<EnumerateData *>(data)->first;
        auto *config = static_cast<EnumerateData *>(data)->second;
        return callback(core::Module(module, config));
    });

    CheckError(conf);
}

}  // namespace abckit

#endif  // CPP_ABCKIT_FILE_IMPL_H
