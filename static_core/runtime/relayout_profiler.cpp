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

#include "runtime/include/relayout_profiler.h"

#include "libpandabase/os/filesystem.h"
#include "runtime/include/runtime.h"

namespace ark {
void RelayoutProfiler::WriteProfileData()
{
    LOG(INFO, RUNTIME) << "start writing profile data!";
    LOG(INFO, RUNTIME) << "relayout profile location is: "
                       << Runtime::GetCurrent()->GetOptions().GetAbcLayoutProfileLocation();
    PandaString profileRecordRoot(Runtime::GetCurrent()->GetOptions().GetAbcLayoutProfileLocation());

    for (auto &file : relayoutItems_) {
        auto pos = file.first.find_last_of('/') + 1;
        PandaString fileName = file.first.substr(pos, file.first.length() - pos);
        PandaString profileFileName = profileRecordRoot + fileName + ".txt";

        ark::os::unix::file::File profileFile =
            ark::os::file::Open(profileFileName, ark::os::file::Mode::READWRITECREATE);
        if (!profileFile.IsValid()) {
            LOG(ERROR, RUNTIME) << "Cannot open the profile file " << profileFileName;
            return;
        }
        ark::os::file::FileHolder fholder(profileFile);

        for (auto &item : file.second[RelayoutProfiler::RelayoutItemType::CLASS_ITEM]) {
            // write classItem
            PandaString itemWithLine = PandaString("class_item:") + item + "\n";
            auto ret = profileFile.Write(itemWithLine.c_str(), itemWithLine.size());
            if (!ret) {
                LOG(ERROR, RUNTIME) << "record " << itemWithLine << " failed! " << ret.Error().ToString();
            }
        }
        for (auto &item : file.second[RelayoutProfiler::RelayoutItemType::STRING_ITEM]) {
            // write StringItem
            PandaString itemWithLine = PandaString("string_item:") + item + "\n";
            auto ret = profileFile.Write(itemWithLine.c_str(), itemWithLine.size());
            if (!ret) {
                LOG(ERROR, RUNTIME) << "record " << itemWithLine << " failed! " << ret.Error().ToString();
            }
        }
        for (auto &item : file.second[RelayoutProfiler::RelayoutItemType::CODE_ITEM]) {
            // write CodeItem
            PandaString itemWithLine = PandaString("code_item:") + item + "\n";
            auto ret = profileFile.Write(itemWithLine.c_str(), itemWithLine.size());
            if (!ret) {
                LOG(ERROR, RUNTIME) << "record " << itemWithLine << " failed! " << ret.Error().ToString();
            }
        }
    }
}

void RelayoutProfiler::AddProfileItem(const std::string &fileName, const std::string &item, RelayoutItemType type)
{
    auto pandaFileName = ConvertToString(fileName);
    auto pandaItem = ConvertToString(item);

    if (relayoutItems_.count(pandaFileName) == 0) {
        PandaUnorderedMap<RelayoutItemType, PandaSet<PandaString>, std::hash<int>> items;
        relayoutItems_.insert(std::make_pair(pandaFileName, std::move(items)));
    }

    relayoutItems_[pandaFileName][type].insert(pandaItem);
}

void RelayoutProfiler::AddProfileClassItem(Class *klass)
{
    std::string fileName =
        klass->GetPandaFile() == nullptr ? std::string("anonymous") : klass->GetPandaFile()->GetFilename();
    std::string item = klass->GetName();
    AddProfileItem(fileName, item, RelayoutProfiler::RelayoutItemType::CLASS_ITEM);
}

void RelayoutProfiler::AddProfileCodeItem(Method *method)
{
    std::string fileName = method->GetPandaFile()->GetFilename();
    std::string item = PandaStringToStd(method->GetFullName());
    AddProfileItem(fileName, item, RelayoutProfiler::RelayoutItemType::CODE_ITEM);
}

ProfileDataType *RelayoutProfiler::GetProfileData()
{
    return &relayoutItems_;
}
}  // namespace ark
