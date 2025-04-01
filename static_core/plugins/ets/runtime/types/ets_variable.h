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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_VARIABLE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_VARIABLE_H_

#include "plugins/ets/runtime/types/ets_field.h"

namespace ark::ets {

class EtsVariable : private EtsField {
public:
    EtsField *AsField()
    {
        return this;
    }

    static EtsVariable *FromField(EtsField *field)
    {
        ASSERT(field == nullptr || field->IsStatic());
        return static_cast<EtsVariable *>(field);
    }
};

static_assert(sizeof(EtsVariable) == sizeof(EtsField));

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_VARIABLE_H_
