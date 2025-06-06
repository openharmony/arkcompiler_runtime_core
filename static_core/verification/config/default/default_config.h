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

#ifndef PANDA_VERIFIER_DEFAULT_DEBUG_CONFIG_H_
#define PANDA_VERIFIER_DEFAULT_DEBUG_CONFIG_H_

namespace ark::verifier::config {

// CC-OFFNXT(G.ARR.07) antipattern of programming
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
extern const char VERIFIER_DEBUG_DEFAULT_CONFIG[];

}  // namespace ark::verifier::config

#endif  // PANDA_VERIFIER_DEFAULT_DEBUG_CONFIG_H_
