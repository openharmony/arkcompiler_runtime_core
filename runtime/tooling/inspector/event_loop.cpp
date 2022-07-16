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

#include "event_loop.h"

#include "macros.h"

namespace panda::tooling::inspector {
bool EventLoop::Pause()
{
    bool wasRunning = running_;
    running_ = false;
    return wasRunning;
}

void EventLoop::Run()
{
    ASSERT_PRINT(!running_, "Event loop is already running");

    for (running_ = true; running_;) {
        RunOne();
    }

    Poll();
}
}  // namespace panda::tooling::inspector
