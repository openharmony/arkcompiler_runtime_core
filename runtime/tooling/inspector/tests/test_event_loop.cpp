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

#include "test_event_loop.h"
#include "test_logger.h"

namespace panda::tooling::inspector::test {
bool TestEventLoop::Poll()
{
    bool result = !messages_.empty();
    while (RunOne()) {
    }
    return result;
}

bool TestEventLoop::RunOne()
{
    if (messages_.empty()) {
        return false;
    }

    const auto &[connection, buffer] = messages_.front();

    auto savedPrefix = logger_.SetPrefix(name_);
    connection->read_all(buffer.data(), buffer.size());
    logger_.SetPrefix(savedPrefix);

    messages_.pop_front();
    return true;
}
}  // namespace panda::tooling::inspector::test
