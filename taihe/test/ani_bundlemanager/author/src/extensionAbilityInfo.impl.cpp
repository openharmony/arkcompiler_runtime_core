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
#include "extensionAbilityInfo.proj.hpp"
#include "extensionAbilityInfo.impl.hpp"
#include "taihe/runtime.hpp"
#include "stdexcept"
#include <iostream>

using namespace taihe;
using namespace extensionAbilityInfo;

namespace {
// To be implemented.

class ExtensionAbilityInfoImpl {
public:
    int32_t extensionAbilityInfoImpl = 21474;
    ExtensionAbilityInfoImpl() {}

    string GetBundleName()
    {
        return "ExtensionAbilityInfoImpl::getBundleName";
    }

    string GetModuleName()
    {
        return "ExtensionAbilityInfoImpl::getModuleName";
    }

    string GetName()
    {
        return "ExtensionAbilityInfoImpl::getName";
    }

    int32_t GetLabelId()
    {
        return extensionAbilityInfoImpl;
    }

    int32_t GetDescriptionId()
    {
        return extensionAbilityInfoImpl;
    }

    int32_t GetIconId()
    {
        return extensionAbilityInfoImpl;
    }

    bool GetExported()
    {
        return true;
    }

    bool GetExtensionAbilityTypeName()
    {
        return true;
    }

    array<string> GetPermissions()
    {
        array<string> str = {"ExtensionAbilityInfoImpl::getPermissions"};
        return str;
    }

    bool GetEnabled()
    {
        return true;
    }

    string GetReadPermission()
    {
        return "ExtensionAbilityInfoImpl::getReadPermission";
    }

    string GetWritePermission()
    {
        return "ExtensionAbilityInfoImpl::getWritePermission";
    }

    int32_t GetAppIndex()
    {
        return extensionAbilityInfoImpl;
    }
};

ExtensionAbilityInfo GetExtensionAbilityInfo()
{
    return make_holder<ExtensionAbilityInfoImpl, ExtensionAbilityInfo>();
}
}  // namespace

TH_EXPORT_CPP_API_GetExtensionAbilityInfo(GetExtensionAbilityInfo);