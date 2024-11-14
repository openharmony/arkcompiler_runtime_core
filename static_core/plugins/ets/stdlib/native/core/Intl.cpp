/**
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

#include "plugins/ets/stdlib/native/core/Intl.h"
#include "plugins/ets/stdlib/native/core/IntlState.h"
#include "plugins/ets/stdlib/native/core/IntlNumberFormat.h"

namespace ark::ets::stdlib {

ets_int InitCoreIntl(EtsEnv *env)
{
    // Create internal data
    CreateIntlState();

    // Register Native methods
    ets_int hasError = ETS_OK;
    hasError += RegisterIntlNumberFormatNativeMethods(env);
    return hasError == ETS_OK ? ETS_OK : ETS_ERR;
}

}  // namespace ark::ets::stdlib
