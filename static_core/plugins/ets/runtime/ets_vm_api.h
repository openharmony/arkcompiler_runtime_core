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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_API_H_
#define PANDA_PLUGINS_ETS_RUNTIME_ETS_VM_API_H_

#include <string>
#include <functional>

#include "libpandabase/macros.h"

namespace ark {
namespace base_options {
class Options;
}  // namespace base_options

class RuntimeOptions;
}  // namespace ark

namespace ark::ets {

PANDA_PUBLIC_API bool CreateRuntime(const std::string &stdlibAbc, const std::string &pathAbc, bool useJit, bool useAot);

PANDA_PUBLIC_API bool CreateRuntime(std::function<bool(base_options::Options *, RuntimeOptions *)> const &addOptions);

PANDA_PUBLIC_API bool DestroyRuntime();

PANDA_PUBLIC_API std::pair<bool, int> ExecuteMain();

PANDA_PUBLIC_API bool BindNative(const char *classDescriptor, const char *methodName, void *impl);

PANDA_PUBLIC_API void LogError(const std::string &msg);
}  // namespace ark::ets

#endif
