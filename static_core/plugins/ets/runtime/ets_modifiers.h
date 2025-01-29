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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_MODIFIERS_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_MODIFIERS_H_

#include <cstdint>

namespace ark::ets {

constexpr uint32_t ACC_FUNCTION = 0x40000000;               // method (ets runtime)
constexpr uint32_t ACC_DEPRECATED_NATIVE_API = 0x80000000;  // method (ets runtime)
// NOTE: Delete ACC_DEPRECATED_NATIVE_API flag whfen ets_napi will be dropped, #22435

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_ETS_MODIFIERS_H_
