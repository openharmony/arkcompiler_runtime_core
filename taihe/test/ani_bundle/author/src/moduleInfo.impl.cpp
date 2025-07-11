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

#include "moduleInfo.proj.hpp"
#include "moduleInfo.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"

using namespace taihe;
using namespace moduleInfo;

namespace {
// To be implemented.

class ModuleInfoImpl {
public:
    string moduleName_ = "this is moduleinfo with name";
    string moduleSourceDir_ = "this is moduleinfo with moduleSourceDir";
    ModuleInfoImpl() {}

    string GetModuleName()
    {
        return moduleName_;
    }

    string GetModuleSourceDir()
    {
        return moduleSourceDir_;
    }
};

ModuleInfo GetModuleInfo()
{
    return make_holder<ModuleInfoImpl, ModuleInfo>();
}
}  // namespace

TH_EXPORT_CPP_API_GetModuleInfo(GetModuleInfo);
