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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_STUBS_H
#define PANDA_PLUGINS_ETS_RUNTIME_STUBS_H

#include <cstdint>

namespace ark::ets {

class EtsCoroutine;
class EtsObject;
class EtsClass;
class EtsString;

// Generic comparator for ets reference types
inline bool EtsReferenceEquals(EtsCoroutine *coro, EtsObject *ref1, EtsObject *ref2);

inline bool EtsReferenceNullish(EtsCoroutine *coro, EtsObject *ref);

// Comparison slowpath for value-typed references
bool EtsValueTypedEquals(EtsCoroutine *coro, EtsObject *obj1, EtsObject *obj2);

// Obtain owner class of method in ets frames
inline EtsClass *GetMethodOwnerClassInFrames(EtsCoroutine *coro, uint32_t depth);

// Compute typeof
EtsString *EtsGetTypeof(EtsCoroutine *coro, EtsObject *obj);

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_STUBS_H
