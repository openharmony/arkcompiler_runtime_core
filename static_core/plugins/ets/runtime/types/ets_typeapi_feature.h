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

#ifndef PANDA_PLUGINS_ETS_TYPEAPI_FEATURE_H_
#define PANDA_PLUGINS_ETS_TYPEAPI_FEATURE_H_

#include "types/ets_primitives.h"

namespace panda::ets {

enum class EtsTypeAPIFeatureAccessModifier : EtsByte { PUBLIC = 0, PRIVATE = 1, PROTECTED = 2 };

// Type features' attributes "flat" representation
enum class EtsTypeAPIFeatureAttributes : EtsInt {
    STATIC = 1U << 0U,       // Field, Method
    INHERITED = 1U << 1U,    // Field, Method
    READONLY = 1U << 2U,     // Field
    FINAL = 1U << 3U,        // Method
    ABSTRACT = 1U << 4U,     // Method
    CONSTRUCTOR = 1U << 5U,  // Method
    REST = 1U << 6U,         // Parameter
    OPTIONAL = 1U << 7U      // Parameter
};

}  // namespace panda::ets

#endif  // PANDA_PLUGINS_ETS_TYPEAPI_FEATURE_H_