/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef VERIFIER_CONFIG_DEBUG_BREAKPOINT_BREAKPOINT_PRIVATE_H
#define VERIFIER_CONFIG_DEBUG_BREAKPOINT_BREAKPOINT_PRIVATE_H

#include "breakpoint.h"
#include "runtime/include/mem/panda_string.h"

namespace panda::verifier::debug {
#ifndef NDEBUG
void AddBreakpointConfig(const PandaString &method_name, Offset offset);
#else
inline void AddBreakpointConfig([[maybe_unused]] const PandaString &method_name, [[maybe_unused]] Offset offset) {}
#endif
}  // namespace panda::verifier::debug

#endif  // VERIFIER_CONFIG_DEBUG_BREAKPOINT_BREAKPOINT_PRIVATE_H
