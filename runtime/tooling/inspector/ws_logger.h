/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_WS_LOGGER_H
#define PANDA_TOOLING_INSPECTOR_WS_LOGGER_H

#include "utils/logger.h"

#include "websocketpp/logger/levels.hpp"

#include <string>

namespace panda::tooling::inspector {
class WsLogger {
    using ChannelType = websocketpp::log::channel_type_hint::value;
    using Level = websocketpp::log::level;

public:
    WsLogger(Level static_channels, ChannelType channel_type)
        : channel_type_(channel_type), static_channels_(static_channels)
    {
    }

    void set_channels(Level channels);

    bool static_test(Level channel) const
    {
        return (channel & static_channels_) == channel;
    }

    bool dynamic_test(Level channel) const
    {
        return (channel & dynamic_channels_) == channel;
    }

    void write(Level channel, const std::string &string) const;

private:
    Logger::Level channel_log_level(Level channel) const;

    const ChannelType channel_type_;
    const Level static_channels_;
    Level dynamic_channels_ {0};
};
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_WS_LOGGER_H
