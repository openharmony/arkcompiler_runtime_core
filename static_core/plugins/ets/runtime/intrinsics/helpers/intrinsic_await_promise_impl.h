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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTRINSIC_AWAIT_PROMISE_IMPL_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTRINSIC_AWAIT_PROMISE_IMPL_H

#include <cstdint>
#include "plugins/ets/runtime/types/ets_promise.h"

namespace ark::ets::intrinsics::helpers {

EtsObject *EtsAwaitPromiseImpl(EtsPromise *promise, int32_t refCount = -1, int32_t primCount = -1, int32_t pc = -1);

}  // namespace ark::ets::intrinsics::helpers

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTRINSIC_AWAIT_PROMISE_IMPL_H
