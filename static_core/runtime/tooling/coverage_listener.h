/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef COVERAGELISTENER_H_
#define COVERAGELISTENER_H_

#include <atomic>
#include <cstdint>
#include <chrono>
#include <optional>
#include <queue>
#include <string_view>
#include <thread>

#include "include/runtime_notification.h"
#include "os/mutex.h"

namespace std {
template <>
struct hash<std::pair<ark::panda_file::File::EntityId, uint32_t>> {
    size_t operator()(const std::pair<ark::panda_file::File::EntityId, uint32_t> &key) const
    {
        size_t hash1 = hash<ark::panda_file::File::EntityId>()(key.first);
        size_t hash2 = hash<uint32_t>()(key.second);
        return hash1 ^ (hash2 << 1);
    }
};
}  // namespace std

namespace ark::tooling {
using BytecodeCountMap = std::unordered_map<std::pair<panda_file::File::EntityId, uint32_t>, uint32_t>;
class BytecodeCounter;

class CoverageListener final : public RuntimeListener {
public:
    explicit CoverageListener(const std::string &fileName);
    ~CoverageListener();

    void BytecodePcChanged(ManagedThread *thread, Method *method, uint32_t bcOffset) override;

    void StopCoverage();

private:
    void ClearInfoFile();
    void StopDumpThread();

    void DumpThreadWorker();
    void DumpCoverageInfoToFile(BytecodeCountMap &bytecodeCountMap);

    PandaUniquePtr<BytecodeCounter> delegate_;

    std::string fileName_;
    std::ofstream outFile;
    std::thread dumpThread_;
    std::atomic<bool> stopThread_ = false;
};

class BytecodeCounter final : public RuntimeListener {
public:
    static constexpr uint32_t byteCodeDataBufferSize = 20000;
    static constexpr uint32_t maxDumpIntervalMs = 1000;
    static constexpr uint32_t queueWaitTimeoutMs = 500;

    explicit BytecodeCounter(uint32_t bufferSize = byteCodeDataBufferSize);
    ~BytecodeCounter();

    void BytecodePcChanged(ManagedThread *thread, Method *method, uint32_t bcOffset) override;

    void StopCoverage();
    std::optional<std::queue<BytecodeCountMap>> GetQueue();

private:
    NO_THREAD_SAFETY_ANALYSIS void TriggerDump();

    os::memory::Mutex mapMutex_;
    os::memory::Mutex queueMutex_;
    os::memory::ConditionVariable queueConditionalVar_;

    uint32_t bufferSize_;
    BytecodeCountMap bytecodeCountMap_ GUARDED_BY(mapMutex_);
    std::queue<BytecodeCountMap> dumpQueue_ GUARDED_BY(queueMutex_);
    std::atomic<std::chrono::system_clock::time_point> lastDumpTime_;
};

}  // namespace ark::tooling
#endif  // COVERAGELISTENER_H_
