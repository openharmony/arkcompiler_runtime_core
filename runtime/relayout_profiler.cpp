/*
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

#include "runtime/include/relayout_profiler.h"

#include "libpandabase/os/filesystem.h"
#include "runtime/include/runtime.h"

namespace panda {
void RelayoutProfiler::WriteProfileData()
{
    LOG(INFO, RUNTIME) << "start writing profile data!";
    LOG(INFO, RUNTIME) << "relayout profile location is: "
                       << Runtime::GetCurrent()->GetOptions().GetAbcLayoutProfileLocation();
    PandaString profile_record_root(Runtime::GetCurrent()->GetOptions().GetAbcLayoutProfileLocation());

    for (auto &file : relayoutItems_) {
        auto pos = file.first.find_last_of('/') + 1;
        PandaString file_name = file.first.substr(pos, file.first.length() - pos);
        PandaString profile_file_name = profile_record_root + file_name + ".txt";

        panda::os::unix::file::File profile_file =
            panda::os::file::Open(profile_file_name, panda::os::file::Mode::READWRITECREATE);
        if (!profile_file.IsValid()) {
            LOG(ERROR, RUNTIME) << "Cannot open the profile file " << profile_file_name;
            return;
        }
        panda::os::file::FileHolder fholder(profile_file);

        for (auto &item : file.second[RelayoutProfiler::RelayoutItemType::CLASS_ITEM]) {
            // write classItem
            PandaString item_with_line = PandaString("class_item:") + item + "\n";
            auto ret = profile_file.Write(item_with_line.c_str(), item_with_line.size());
            if (!ret) {
                LOG(ERROR, RUNTIME) << "record " << item_with_line << " failed! " << ret.Error().ToString();
            }
        }
        for (auto &item : file.second[RelayoutProfiler::RelayoutItemType::STRING_ITEM]) {
            // write StringItem
            PandaString item_with_line = PandaString("string_item:") + item + "\n";
            auto ret = profile_file.Write(item_with_line.c_str(), item_with_line.size());
            if (!ret) {
                LOG(ERROR, RUNTIME) << "record " << item_with_line << " failed! " << ret.Error().ToString();
            }
        }
        for (auto &item : file.second[RelayoutProfiler::RelayoutItemType::CODE_ITEM]) {
            // write CodeItem
            PandaString item_with_line = PandaString("code_item:") + item + "\n";
            auto ret = profile_file.Write(item_with_line.c_str(), item_with_line.size());
            if (!ret) {
                LOG(ERROR, RUNTIME) << "record " << item_with_line << " failed! " << ret.Error().ToString();
            }
        }
    }
}

void RelayoutProfiler::AddProfileItem(const std::string &file_name, const std::string &item, RelayoutItemType type)
{
    auto panda_file_name = ConvertToString(file_name);
    auto panda_item = ConvertToString(item);

    if (relayoutItems_.count(panda_file_name) == 0) {
        PandaUnorderedMap<RelayoutItemType, PandaSet<PandaString>, std::hash<int>> items;
        relayoutItems_.insert(std::make_pair(panda_file_name, std::move(items)));
    }

    relayoutItems_[panda_file_name][type].insert(panda_item);
}

void RelayoutProfiler::AddProfileClassItem(Class *klass)
{
    std::string file_name =
        klass->GetPandaFile() == nullptr ? std::string("anonymous") : klass->GetPandaFile()->GetFilename();
    std::string item = klass->GetName();
    AddProfileItem(file_name, item, RelayoutProfiler::RelayoutItemType::CLASS_ITEM);
}

void RelayoutProfiler::AddProfileCodeItem(Method *method)
{
    std::string file_name = method->GetPandaFile()->GetFilename();
    std::string item = PandaStringToStd(method->GetFullName());
    AddProfileItem(file_name, item, RelayoutProfiler::RelayoutItemType::CODE_ITEM);
}

ProfileDataType *RelayoutProfiler::GetProfileData()
{
    return &relayoutItems_;
}
}  // namespace panda
