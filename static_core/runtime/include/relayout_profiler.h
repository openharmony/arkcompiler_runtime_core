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
#ifndef PANDA_RUNTIME_RELAYOUT_PROFILER_H
#define PANDA_RUNTIME_RELAYOUT_PROFILER_H

#include <utility>
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/class.h"
#include "runtime/include/method.h"

namespace ark {
class RelayoutProfiler {
public:
    enum RelayoutItemType { CLASS_ITEM, STRING_ITEM, CODE_ITEM };
    using ProfileDataType =
        PandaUnorderedMap<PandaString, PandaUnorderedMap<RelayoutItemType, PandaSet<PandaString>, std::hash<int>>>;

    void WriteProfileData();
    void AddProfileItem(const std::string &fileName, const std::string &item, RelayoutItemType type);
    void AddProfileClassItem(Class *klass);
    void AddProfileCodeItem(Method *method);
    ProfileDataType *GetProfileData();

private:
    ProfileDataType relayoutItems_;
};

#ifdef PANDA_ENABLE_RELAYOUT_PROFILE
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ADD_PROFILE_CLASS_ITEM(...) Runtime::GetCurrent()->GetRelayoutProfiler()->AddProfileClassItem(__VA_ARGS__)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ADD_PROFILE_STRING_ITEM(...)                                          \
    Runtime::GetCurrent()->GetRelayoutProfiler()->AddProfileItem(__VA_ARGS__, \
                                                                 RelayoutProfiler::RelayoutItemType::STRING_ITEM)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ADD_PROFILE_CODE_ITEM(...) Runtime::GetCurrent()->GetRelayoutProfiler()->AddProfileCodeItem(__VA_ARGS__)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define WRITE_RELAYOUT_PROFILE_DATA() Runtime::GetCurrent()->GetRelayoutProfiler()->WriteProfileData()
#else
#define ADD_PROFILE_CLASS_ITEM(...)
#define ADD_PROFILE_STRING_ITEM(...)
#define ADD_PROFILE_CODE_ITEM(...)
#define WRITE_RELAYOUT_PROFILE_DATA()
#endif

using ProfileDataType =
    PandaUnorderedMap<PandaString,
                      PandaUnorderedMap<RelayoutProfiler::RelayoutItemType, PandaSet<PandaString>, std::hash<int>>>;
}  // namespace ark
#endif  // PANDA_RUNTIME_RELAYOUT_PROFILER_H
