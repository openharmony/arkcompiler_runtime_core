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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_NAMESPACE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_NAMESPACE_H_

#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/types/ets_variable.h"

namespace ark::ets {

class EtsNamespace : private EtsClass {
public:
    EtsClass *AsClass()
    {
        return this;
    }

    static EtsNamespace *FromClass(EtsClass *cls)
    {
        // NODE: Check that class is namespace, when #22400 is resolved
        return static_cast<EtsNamespace *>(cls);
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
};

static_assert(sizeof(EtsNamespace) == sizeof(EtsClass));

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_NAMESPACE_H_
