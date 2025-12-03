/**
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_MODULE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_MODULE_H_

#include "ani.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_variable.h"

namespace ark::ets {

class EtsModule : private EtsClass {
public:
    EtsClass *AsClass()
    {
        return this;
    }

    static EtsModule *FromClass(EtsClass *cls)
    {
        // NODE: Check that class is module, when #22400 is resolved
        return static_cast<EtsModule *>(cls);
    }

    EtsVariable *GetVariabe(const char *name)
    {
        EtsField *field = GetStaticFieldIDByName(name, nullptr);
        return EtsVariable::FromField(field);
    }

    EtsMethod *GetFunction(const char *name, const char *signature)
    {
        return GetStaticMethod(name, signature);
    }

    ani_status GetModulePrefix(PandaString &prefix)
    {
        static constexpr std::string_view SUFFIX = "/ETSGLOBAL;";
        const char *moduleDescriptor = AsClass()->GetDescriptor();
        size_t len = strlen(moduleDescriptor);
        if (len < SUFFIX.size()) {
            return ANI_INVALID_DESCRIPTOR;
        }
        std::string_view descriptorView(moduleDescriptor, len);
        if (!std::equal(SUFFIX.rbegin(), SUFFIX.rend(), descriptorView.rbegin())) {
            return ANI_INVALID_DESCRIPTOR;
        }
        prefix = PandaString(moduleDescriptor, len - SUFFIX.size());
        return ANI_OK;
    }
};

static_assert(sizeof(EtsModule) == sizeof(EtsClass));

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_MODULE_H_
