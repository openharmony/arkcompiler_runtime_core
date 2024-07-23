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

#include "platforms/unix/libpandabase/property.h"

#ifdef PANDA_TARGET_MOBILE
#include <sys/system_properties.h>
#endif  // PANDA_TARGET_MOBILE

namespace ark::os::unix::property {

bool GetPropertyBuffer([[maybe_unused]] const char *arkProp, [[maybe_unused]] std::string &out)
{
#ifdef PANDA_TARGET_MOBILE
    char buf[PROP_VALUE_MAX] = {};
    if (__system_property_get(arkProp, buf) > 0) {
        out = buf;
        return true;
    }
#endif  // PANDA_TARGET_MOBILE
    return false;
}

}  // namespace ark::os::unix::property
