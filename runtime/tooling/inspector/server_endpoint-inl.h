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

#ifndef RUNTIME_TOOLING_INSPECTOR_SERVER_ENDPOINT_INL_H
#define RUNTIME_TOOLING_INSPECTOR_SERVER_ENDPOINT_INL_H

#ifndef CONFIG
#error Define CONFIG before including this header
#endif

#include "server_endpoint.h"

#include "utils/logger.h"

#include <functional>
#include <optional>
#include <utility>

namespace panda::tooling::inspector {
template <>  // NOLINTNEXTLINE(misc-definitions-in-headers)
ServerEndpoint<CONFIG>::ServerEndpoint() noexcept
{
    this->endpoint_.set_validate_handler([this](auto hdl) {
        if (Pin(hdl)) {
            return true;
        }

        this->endpoint_.get_con_from_hdl(hdl)->set_body("Another debug session is in progress");
        return false;
    });

    this->endpoint_.set_close_handler([this](auto hdl) { Unpin(hdl); });
}

template <>  // NOLINTNEXTLINE(misc-definitions-in-headers)
void ServerEndpoint<CONFIG>::Call(const char *method, std::function<void(JsonObjectBuilder &)> &&params)
{
    Endpoint<websocketpp::server<CONFIG>>::Call(std::nullopt, method, std::move(params));
}

template <>  // NOLINTNEXTLINE(misc-definitions-in-headers)
void ServerEndpoint<CONFIG>::OnCall(const char *method,
                                    std::function<void(JsonObjectBuilder &, const JsonObject &)> &&handler)
{
    Endpoint<websocketpp::server<CONFIG>>::OnCall(method, [this, handler = std::move(handler)](auto id, auto &params) {
        if (!id) {
            LOG(INFO, DEBUGGER) << "Invalid request: request has no \"id\"";
            return;
        }

        Reply(*id, std::bind(std::ref(handler), std::placeholders::_1, std::cref(params)));
    });
}
}  // namespace panda::tooling::inspector

#endif  // RUNTIME_TOOLING_INSPECTOR_SERVER_ENDPOINT_INL_H
