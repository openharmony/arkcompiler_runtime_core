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

#include "plugins/ets/stdlib/native/core/IntlState.h"
#include "libpandabase/macros.h"

namespace ark::ets::stdlib::intl {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::unique_ptr<IntlState> g_intlState {nullptr};

void CreateIntlState()
{
    if (g_intlState == nullptr) {
        g_intlState = std::make_unique<IntlState>();
        ASSERT(g_intlState != nullptr);
    }
}

}  // namespace ark::ets::stdlib::intl
