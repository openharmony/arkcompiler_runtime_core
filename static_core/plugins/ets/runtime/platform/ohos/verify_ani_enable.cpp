/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "verify_ani_enable.h"
#ifdef PANDA_OHOS_GET_PARAMETER
#include "syspara/parameter.h"
#include "syspara/parameters.h"
#endif

namespace ark::ets::ohos {

bool GetVerifyANIEnable()
{
#ifdef PANDA_OHOS_GET_PARAMETER
    return OHOS::system::GetBoolParameter("debug.verifyANI.enable", false);
#else
    return false;
#endif
}

}  // namespace ark::ets::ohos
