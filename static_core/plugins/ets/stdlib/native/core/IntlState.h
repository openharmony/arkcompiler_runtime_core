/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLSTATE_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLSTATE_H

#include "plugins/ets/stdlib/native/core/IntlFormattersCache.h"
#include <memory>

namespace ark::ets::stdlib::intl {

struct IntlState {
    IntlFormattersCache fmtsCache;
};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
extern std::unique_ptr<IntlState> g_intlState;

void CreateIntlState();

}  // namespace ark::ets::stdlib::intl

#endif  //  PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLSTATE_H
