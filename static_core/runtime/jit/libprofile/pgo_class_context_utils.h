/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef ARK_RUNTIME_JIT_LIBPROFILE_PGO_CLASS_CONTEXT_UTILS_H
#define ARK_RUNTIME_JIT_LIBPROFILE_PGO_CLASS_CONTEXT_UTILS_H

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace ark::pgo {

class PgoClassContextUtils {
public:
    static bool Parse(std::string_view ctx, std::map<std::string, std::string> &entries, std::string *error);
    static bool Merge(const std::vector<std::string_view> &contexts, std::string &merged, std::string *error);
};

}  // namespace ark::pgo

#endif  // ARK_RUNTIME_JIT_LIBPROFILE_PGO_CLASS_CONTEXT_UTILS_H
