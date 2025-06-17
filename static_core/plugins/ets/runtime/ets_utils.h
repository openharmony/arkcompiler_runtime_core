/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_UTILS_H
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_UTILS_H

#include "libpandabase/macros.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets {

// NOLINTBEGIN(modernize-avoid-c-arrays)
static constexpr char const ETSGLOBAL_CLASS_NAME[] = "ETSGLOBAL";
// NOLINTEND(modernize-avoid-c-arrays)

static constexpr std::string_view INDEXED_INT_GET_METHOD_SIGNATURE = "I:Lstd/core/Object;";
static constexpr std::string_view INDEXED_INT_SET_METHOD_SIGNATURE = "ILstd/core/Object;:V";

bool IsEtsGlobalClassName(const std::string &descriptor);

EtsObject *GetBoxedValue(EtsCoroutine *coro, Value value, EtsType type);

Value GetUnboxedValue(EtsCoroutine *coro, EtsObject *obj);

EtsObject *GetPropertyValue(EtsCoroutine *coro, const EtsObject *etsObj, EtsField *field);
bool SetPropertyValue(EtsCoroutine *coro, EtsObject *etsObj, EtsField *field, EtsObject *valToSet);

class LambdaUtils {
public:
    PANDA_PUBLIC_API static void InvokeVoid(EtsCoroutine *coro, EtsObject *lambda);

    NO_COPY_SEMANTIC(LambdaUtils);
    NO_MOVE_SEMANTIC(LambdaUtils);

private:
    LambdaUtils() = default;
    ~LambdaUtils() = default;
};

class ManglingUtils {
public:
    PANDA_PUBLIC_API static EtsString *GetDisplayNameStringFromField(EtsField *field);
    PANDA_PUBLIC_API static EtsField *GetFieldIDByDisplayName(EtsClass *klass, const PandaString &name,
                                                              const char *sig = nullptr);

    NO_COPY_SEMANTIC(ManglingUtils);
    NO_MOVE_SEMANTIC(ManglingUtils);

private:
    ManglingUtils() = default;
    ~ManglingUtils() = default;
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ETS_UTILS_H
