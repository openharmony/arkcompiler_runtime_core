/*
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

#ifndef PANDA_LIBPANDABASE_OS_SYSTEM_ENVIRONMENT_H
#define PANDA_LIBPANDABASE_OS_SYSTEM_ENVIRONMENT_H

#include <string>
#include "libpandabase/macros.h"

namespace ark::os::system_environment {

PANDA_PUBLIC_API int GetUidForName(const std::string &name);

PANDA_PUBLIC_API int GetUidForName(const char *name);

// Get configuration information
PANDA_PUBLIC_API int64_t GetSystemConfig(int name);

PANDA_PUBLIC_API std::string GetEnvironmentVar(const std::string &name);

PANDA_PUBLIC_API std::string GetEnvironmentVar(const char *name);

}  // namespace ark::os::system_environment

#endif  // PANDA_LIBPANDABASE_OS_SYSTEM_ENVIRONMENT_H
