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

#include "profiling_data.h"
#include "utils/logger.h"
#include "ipc/ipc_message.h"
#include "ipc/ipc_unix_socket.h"
#include "ipc/ipc_message_protocol.h"
#include "serializer/serializer.h"

namespace panda::dprof {
bool ProfilingData::SetFeatureDate(const std::string &feature_name, std::vector<uint8_t> &&data)
{
    auto it = features_data_map_.find(feature_name);
    if (it != features_data_map_.end()) {
        LOG(ERROR, DPROF) << "Feature already exists, featureName=" << feature_name;
        return false;
    }

    features_data_map_.emplace(std::pair(feature_name, std::move(data)));
    return false;
}

bool ProfilingData::DumpAndResetFeatures()
{
    os::unique_fd::UniqueFd sock(ipc::CreateUnixClientSocket());
    if (!sock.IsValid()) {
        LOG(ERROR, DPROF) << "Cannot create client socket";
        return false;
    }

    ipc::protocol::Version tmp {ipc::protocol::VERSION};

    std::vector<uint8_t> version_data;
    serializer::StructToBuffer<ipc::protocol::VERSION_FCOUNT>(tmp, version_data);

    ipc::Message msg_version(ipc::Message::Id::VERSION, std::move(version_data));
    if (!SendMessage(sock.Get(), msg_version)) {
        LOG(ERROR, DPROF) << "Cannot send version";
        return false;
    }

    ipc::protocol::AppInfo tmp2 {app_name_, hash_, pid_};
    std::vector<uint8_t> app_info_data;
    serializer::StructToBuffer<ipc::protocol::APP_INFO_FCOUNT>(tmp2, app_info_data);

    ipc::Message msg_app_info(ipc::Message::Id::APP_INFO, std::move(app_info_data));
    if (!SendMessage(sock.Get(), msg_app_info)) {
        LOG(ERROR, DPROF) << "Cannot send app info";
        return false;
    }

    // Send features data
    for (auto &kv : features_data_map_) {
        ipc::protocol::FeatureData tmp_data;
        tmp_data.name = kv.first;
        tmp_data.data = std::move(kv.second);

        std::vector<uint8_t> feature_data;
        serializer::StructToBuffer<ipc::protocol::FEATURE_DATA_FCOUNT>(tmp_data, feature_data);

        ipc::Message msg_feature_data(ipc::Message::Id::FEATURE_DATA, std::move(feature_data));
        if (!SendMessage(sock.Get(), msg_feature_data)) {
            LOG(ERROR, DPROF) << "Cannot send feature data, featureName=" << tmp_data.name;
            return false;
        }
    }

    features_data_map_.clear();
    return true;
}
}  // namespace panda::dprof
