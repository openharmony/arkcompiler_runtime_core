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

#include <ets_napi.h>
#include "libpandabase/utils/utils.h"

extern "C" {
ETS_EXPORT ets_int ETS_CALL ETS_InterfaceNativeTest_InnerNativeTest_Inner_innerNative([[maybe_unused]] EtsEnv *,
                                                                                      [[maybe_unused]] ets_class)
{
    return 2_I;
}

ETS_EXPORT ets_int ETS_CALL ETS_InterfaceNativeTest_InnerNativeTest_Inner_InnerInner_innerNative(
    [[maybe_unused]] EtsEnv *, [[maybe_unused]] ets_class)
{
    return 3_I;
}

}  // extern "C"
