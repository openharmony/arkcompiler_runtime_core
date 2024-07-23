/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_INTEROP_ETS_NAPI_INTERNAL_H
#define PANDA_RUNTIME_INTEROP_ETS_NAPI_INTERNAL_H

#include "ets_napi.h"

// NOLINTBEGIN(readability-identifier-naming)

extern "C" ETS_EXPORT ets_int _internal_ETS_GetDefaultVMInitArgs(EtsVMInitArgs *vmArgs);
extern "C" ETS_EXPORT ets_int _internal_ETS_GetCreatedVMs(EtsVM **vmBuf, ets_size bufLen, ets_size *nVms);
extern "C" ETS_EXPORT ets_int _internal_ETS_CreateVM(EtsVM **pVm, EtsEnv **pEnv, EtsVMInitArgs *vmArgs);

// NOLINTEND(readability-identifier-naming)

#endif  // PANDA_RUNTIME_INTEROP_ETS_NAPI_INTERNAL_H
