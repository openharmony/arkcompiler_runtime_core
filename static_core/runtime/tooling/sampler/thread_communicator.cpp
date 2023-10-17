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

#include "runtime/tooling/sampler/sample_info.h"
#include "runtime/tooling/sampler/thread_communicator.h"

namespace panda::tooling::sampler {

bool ThreadCommunicator::IsPipeEmpty() const
{
    ASSERT(listener_pipe_[PIPE_READ_ID] != 0 && listener_pipe_[PIPE_WRITE_ID] != 0);

    struct pollfd poll_fd = {listener_pipe_[PIPE_READ_ID], POLLIN, 0};
    return poll(&poll_fd, 1, 0) == 0;
}

bool ThreadCommunicator::SendSample(const SampleInfo &sample) const
{
    ASSERT(listener_pipe_[PIPE_READ_ID] != 0 && listener_pipe_[PIPE_WRITE_ID] != 0);

    const void *buffer = reinterpret_cast<const void *>(&sample);
    ssize_t syscall_result = write(listener_pipe_[PIPE_WRITE_ID], buffer, sizeof(SampleInfo));
    if (syscall_result == -1) {
        return false;
    }
    ASSERT(syscall_result == sizeof(SampleInfo));
    return true;
}

bool ThreadCommunicator::ReadSample(SampleInfo *sample) const
{
    ASSERT(listener_pipe_[PIPE_READ_ID] != 0 && listener_pipe_[PIPE_WRITE_ID] != 0);

    void *buffer = reinterpret_cast<void *>(sample);

    // TODO(m.strizhak): optimize by reading several samples by one call
    ssize_t syscall_result = read(listener_pipe_[PIPE_READ_ID], buffer, sizeof(SampleInfo));
    if (syscall_result == -1) {
        return false;
    }
    ASSERT(syscall_result == sizeof(SampleInfo));
    return true;
}

}  // namespace panda::tooling::sampler
