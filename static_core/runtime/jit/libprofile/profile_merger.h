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

#ifndef PGO_PROFILE_MERGER_H
#define PGO_PROFILE_MERGER_H

#include "aot_profiling_data.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace ark::pgo {

struct MergedProfile {
    // Owns panda file strings referenced by MergedProfile::data (AotProfilingData stores string_view).
    PandaVector<PandaString> pandaFilesStorage;
    AotProfilingData data;
};

class ProfileMerger {
public:
    bool Merge(const PandaVector<const AotProfilingData *> &inputs, MergedProfile &output, std::string *error) const;
};

}  // namespace ark::pgo

#endif  // PGO_PROFILE_MERGER_H
