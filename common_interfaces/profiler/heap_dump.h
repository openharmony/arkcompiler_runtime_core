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

#ifndef COMMON_INTERFACES_PROFILER_HEAP_DUMP_H
#define COMMON_INTERFACES_PROFILER_HEAP_DUMP_H

#include <cstdint>
#include <functional>
#include <string>

namespace common::dump {

enum class DumpExecutionMode : uint8_t {
    IN_PROCESS,
    FORK_ONCE,
};

enum class DumpScope : uint8_t {
    VM,
    PROCESS,
};

enum class DumpReason : uint8_t {
    NORMAL,
    DYNAMIC_LOCAL_OOM,
    DYNAMIC_SHARED_OOM,
    DYNAMIC_SHARED_GC_OOM,
    STATIC_OOM,
};

struct DumpIdentity {
    static constexpr int32_t UNSPECIFIED_ID = -1;

    constexpr DumpIdentity() = default;
    constexpr DumpIdentity(int32_t pid, int32_t tid, uint64_t timestampMillis)
        : pid_(pid), tid_(tid), timestampMillis_(timestampMillis)
    {
    }

    bool IsValid() const
    {
        return pid_ > 0 && tid_ > 0 && timestampMillis_ > 0;
    }

    int32_t GetPid() const
    {
        return pid_;
    }

    int32_t GetTid() const
    {
        return tid_;
    }

    uint64_t GetTimestampMillis() const
    {
        return timestampMillis_;
    }

private:
    int32_t pid_ = UNSPECIFIED_ID;
    int32_t tid_ = UNSPECIFIED_ID;
    // System-clock milliseconds shared by every output of one dump request.
    uint64_t timestampMillis_ = 0;
};

struct DumpPolicy {
    DumpExecutionMode executionMode = DumpExecutionMode::FORK_ONCE;
    DumpScope scope = DumpScope::VM;
    bool triggerGC = true;
};

struct DumpOutputOptions {
    std::string dynamicPath;
    std::string staticPath;
};

struct OOMContext {
    std::string spaceType;
    std::string heapType;
};

struct DumpRequest {
    DumpReason reason = DumpReason::NORMAL;
    DumpPolicy policy {};
    OOMContext oom {};
    DumpIdentity identity {};
    DumpOutputOptions output {};
    std::function<void(uint8_t)> completionCallback;
};

struct DumpStatistics {
    uint64_t objectCount = 0;
    uint64_t classCount = 0;
};

struct DumpResult {
    DumpStatistics statistics {};
    bool success = false;
};

/** Minimal output contract needed by the session coordinator. */
class DumpOutput {
public:
    DumpOutput() = default;
    virtual ~DumpOutput() = default;
    DumpOutput(const DumpOutput &) = delete;
    DumpOutput &operator=(const DumpOutput &) = delete;
    DumpOutput(DumpOutput &&) = delete;
    DumpOutput &operator=(DumpOutput &&) = delete;

    virtual bool Flush() = 0;
};

/**
 * Runtime-neutral participant in one heap dump session.
 *
 * Each runtime owns its suspension scope, descriptor and writer lifetime. The
 * session coordinator only invokes the lifecycle in the required order.
 */
class AbstractDumper {
public:
    AbstractDumper() = default;
    virtual ~AbstractDumper() = default;
    AbstractDumper(const AbstractDumper &) = delete;
    AbstractDumper &operator=(const AbstractDumper &) = delete;
    AbstractDumper(AbstractDumper &&) = delete;
    AbstractDumper &operator=(AbstractDumper &&) = delete;

    virtual void TriggerGC() = 0;
    virtual void PrepareSession() = 0;
    virtual DumpResult Dump() = 0;

    /**
     * Acquire the runtime-specific output. The operation must be idempotent:
     * the coordinator may call it before execution, while each dumper also
     * calls it immediately before writing.
     */
    virtual bool AcquireOutput() = 0;

    virtual int GetOutputFd() const
    {
        return -1;
    }

    virtual void CompleteCrossRuntimeGC() {}

    virtual void *GetCurrentVM()
    {
        return nullptr;
    }

    virtual void PrepareForkChild() {}

    virtual uint32_t GetNodeId([[maybe_unused]] uint64_t addr) const
    {
        return 0;
    }

    virtual DumpOutput *GetOutput()
    {
        return nullptr;
    }
};

}  // namespace common::dump

#endif  // COMMON_INTERFACES_PROFILER_HEAP_DUMP_H
