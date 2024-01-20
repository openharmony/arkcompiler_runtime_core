/**
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_ETS_ETS_STRING_BUILDER_H
#define PANDA_RUNTIME_ETS_ETS_STRING_BUILDER_H

#include "runtime/include/object_header.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace panda::ets {

EtsString *StringBuilderToString(ObjectHeader *sb);
ObjectHeader *StringBuilderAppendString(ObjectHeader *sb, EtsString *str);
ObjectHeader *StringBuilderAppendBool(ObjectHeader *sb, EtsBoolean v);
ObjectHeader *StringBuilderAppendChar(ObjectHeader *sb, EtsChar v);
ObjectHeader *StringBuilderAppendLong(ObjectHeader *sb, EtsLong v);

}  // namespace panda::ets

#endif  // PANDA_RUNTIME_ETS_ETS_STRING_BUILDER_H
