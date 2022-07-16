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

#ifndef PANDA_TOOLING_INSPECTOR_ASIO_SERVER_H
#define PANDA_TOOLING_INSPECTOR_ASIO_SERVER_H

#include "asio_config.h"
#include "server_endpoint.h"

#include "websocketpp/uri.hpp"

#include <cstdint>

namespace panda::tooling::inspector {
class AsioServer final : public ServerEndpoint<AsioConfig> {
public:
    bool Poll() override
    {
        return endpoint_.poll() != 0;
    }

    bool RunOne() override
    {
        return endpoint_.run_one() != 0;
    }

    websocketpp::uri_ptr Start(uint32_t port = 0);
    bool Stop();

private:
    bool Initialize();

    bool initialized_ {false};
};
}  // namespace panda::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_ASIO_SERVER_H
