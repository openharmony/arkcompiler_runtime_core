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

#include "combined_event_loop.h"

namespace panda::tooling::inspector::test {
bool CombinedEventLoop::Poll()
{
    if (!RunOne()) {
        return false;
    }

    while (RunOne()) {
    }
    return true;
}

bool CombinedEventLoop::RunOne()
{
    for (unsigned count = 2; count-- != 0;) {
        switch (state_) {
            case LEFT:
                state_ = RIGHT;
                if (left_.RunOne()) {
                    return true;
                }
                break;

            case RIGHT:
                state_ = LEFT;
                if (right_.RunOne()) {
                    return true;
                }
                break;
        }
    }

    return false;
}
}  // namespace panda::tooling::inspector::test
