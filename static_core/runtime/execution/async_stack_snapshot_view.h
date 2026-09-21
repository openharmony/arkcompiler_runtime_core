/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_EXECUTION_ASYNC_STACK_SNAPSHOT_VIEW_H
#define PANDA_RUNTIME_EXECUTION_ASYNC_STACK_SNAPSHOT_VIEW_H

#include <cstdint>
#include <string>
#include <vector>

namespace ark {

/** Native copy of one user frame stored in a language-specific async snapshot. */
struct AsyncStackFrameView {
    std::string functionName;
    std::string pandaFile;
    uint64_t methodId = 0U;
    uint32_t bytecodeOffset = 0U;
    uint64_t nativePc = 0U;
};

/** Native copy of one asynchronous stack segment. */
struct AsyncStackSegmentView {
    std::string description;
    uint64_t generation = 0U;
    std::vector<AsyncStackFrameView> frames;
};

/**
 * A thread-independent native view of an async stack snapshot.
 *
 * A language plugin creates this value while the paused mutator thread is available. Inspector code may
 * afterwards consume the native copy without interpreting managed objects.
 */
struct AsyncStackSnapshotView {
    uint64_t generation = 0U;
    std::vector<AsyncStackSegmentView> segments;
};

}  // namespace ark

#endif  // PANDA_RUNTIME_EXECUTION_ASYNC_STACK_SNAPSHOT_VIEW_H
