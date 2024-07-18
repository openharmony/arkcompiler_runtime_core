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

#include "property.h"

namespace ark::os::property {

#if !defined(PANDA_TARGET_UNIX)
bool GetPropertyBuffer(const char * /* ark_prop */, std::string & /* out */)
{
    return false;
}
#endif  // PANDA_TARGET_UNIX

}  // namespace ark::os::property
