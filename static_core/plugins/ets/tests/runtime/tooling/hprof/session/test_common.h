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
 * WITHOUT WARRANTIES OR CONDITIONS of ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_PLUGINS_ETS_TESTS_RUNTIME_TOOLING_HPROF_SESSION_TEST_COMMON_H
#define PANDA_PLUGINS_ETS_TESTS_RUNTIME_TOOLING_HPROF_SESSION_TEST_COMMON_H

#include "libarkbase/os/thread.h"
#include "profiler/heap_dump.h"
#include "plugins/ets/runtime/tooling/hprof/session/abstract_writer.h"
#include "plugins/ets/runtime/tooling/hprof/session/common_writer.h"
#include "plugins/ets/runtime/tooling/hprof/session/object_id_map.h"
#include "plugins/ets/runtime/tooling/hprof/session/output_stream.h"
#include "plugins/ets/runtime/tooling/hprof/session/string_id_pool.h"
#include "plugins/ets/runtime/tooling/hprof/heap_dump_coordinator.h"

#if defined(PANDA_JS_ETS_HYBRID_MODE)
#include "hybrid/sts_vm_interface.h"
#endif

#include <gtest/gtest.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <limits>
#include <memory>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <unistd.h>

namespace ark::tooling::hprof::test {

// Test data constants
inline constexpr uint64_t DEFAULT_DUMPER_OBJECT_COUNT = 10U;
inline constexpr uint64_t DEFAULT_DUMPER_CLASS_COUNT = 3U;
inline constexpr int FROZEN_READ_ENTRY_COUNT = 100;
inline constexpr int FROZEN_READ_THREAD_COUNT = 8;
inline constexpr int MOCK_DUMPER_DELAY_MS = 10;

using ScopedFile = std::unique_ptr<FILE, decltype(&fclose)>;

inline ScopedFile OpenWriteOnlyFile(const std::string &path)
{
    return {fopen(path.c_str(), "we"), &fclose};
}

// ===========================================================================
// Temp-file helpers - create temp path, read back binary content
// ===========================================================================

inline std::string CreateTempPath()
{
    std::string path = "panda_dump_test_XXXXXX";
    int fd = mkstemp(path.data());
    close(fd);
    return path;
}

inline std::vector<uint8_t> ReadFileBack(const std::string &path)
{
    std::ifstream ifs(path, std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return data;
}

inline void RemoveTempFile(const std::string &path)
{
    unlink(path.c_str());
}

// ===========================================================================
// Little-endian read helpers
// ===========================================================================

template <typename T>
inline T ReadUnsignedLE(const std::vector<uint8_t> &data, size_t offset)
{
    static_assert(std::is_unsigned_v<T>);
    uint64_t value = 0;
    for (size_t i = 0; i < sizeof(T); i++) {
        value |= static_cast<uint64_t>(data.at(offset + i)) << (i * std::numeric_limits<uint8_t>::digits);
    }
    return static_cast<T>(value);
}

inline uint16_t ReadU16LE(const std::vector<uint8_t> &data, size_t offset)
{
    return ReadUnsignedLE<uint16_t>(data, offset);
}

inline uint32_t ReadU32LE(const std::vector<uint8_t> &data, size_t offset)
{
    return ReadUnsignedLE<uint32_t>(data, offset);
}

inline uint64_t ReadU64LE(const std::vector<uint8_t> &data, size_t offset)
{
    return ReadUnsignedLE<uint64_t>(data, offset);
}

// ===========================================================================
// Record parsing helper - find a record by tag starting after V3 header
// ===========================================================================

struct RecordInfo {
    uint8_t tag = 0;
    uint32_t bodySize = 0;
    uint32_t count = 0;
    size_t bodyStart = 0;
    size_t nextOffset = 0;
};

RecordInfo ParseRecord(const std::vector<uint8_t> &data, size_t offset);

RecordInfo FindRecordAfterHeader(const std::vector<uint8_t> &data, uint8_t targetTag);

// ===========================================================================
// Mock dumpers - retain lifecycle observations after participant cleanup
// ===========================================================================

struct FactoryState {
    bool completeCrossRuntimeGCCalled = false;
    bool triggerGCCalled = false;
    bool prepareSessionCalled = false;
    bool dumpCalled = false;
};

class MockDumperBehavior {
public:
    explicit MockDumperBehavior(FactoryState *state = nullptr) : state_(state) {}

    void TriggerGC()
    {
        if (state_ != nullptr) {
            state_->triggerGCCalled = true;
        }
    }

    void PrepareSession()
    {
        if (state_ != nullptr) {
            state_->prepareSessionCalled = true;
        }
    }

    DumpResult Dump()
    {
        if (state_ != nullptr) {
            state_->dumpCalled = true;
        }
        os::thread::NativeSleep(MOCK_DUMPER_DELAY_MS);
        return {{DEFAULT_DUMPER_OBJECT_COUNT, DEFAULT_DUMPER_CLASS_COUNT}, dumpResult_};
    }

private:
    FactoryState *state_ = nullptr;
    bool dumpResult_ = true;
};

class MockDynamicDumper : public AbstractDumper {
public:
    explicit MockDynamicDumper(FactoryState *state = nullptr) : behavior_(state), state_(state) {}

    void TriggerGC() override
    {
        behavior_.TriggerGC();
    }

    void CompleteCrossRuntimeGC() override
    {
        if (state_ != nullptr) {
            state_->completeCrossRuntimeGCCalled = true;
        }
    }

    void PrepareSession() override
    {
        behavior_.PrepareSession();
    }

    bool AcquireOutput() override
    {
        return true;
    }

    DumpResult Dump() override
    {
        return behavior_.Dump();
    }

    void *GetCurrentVM() override
    {
        return nullptr;
    }

    void PrepareForkChild() override {}

    uint32_t GetNodeId([[maybe_unused]] uint64_t address) const override
    {
        return 0;
    }

private:
    MockDumperBehavior behavior_;
    FactoryState *state_ = nullptr;
};

class MockStaticDumper : public AbstractDumper {
public:
    explicit MockStaticDumper(FactoryState *state = nullptr) : behavior_(state) {}

    void TriggerGC() override
    {
        behavior_.TriggerGC();
    }

    void PrepareSession() override
    {
        behavior_.PrepareSession();
    }

    bool AcquireOutput() override
    {
        return true;
    }

    DumpResult Dump() override
    {
        return behavior_.Dump();
    }

    common::dump::DumpOutput *GetOutput() override
    {
        return nullptr;
    }

private:
    MockDumperBehavior behavior_;
};

#if defined(PANDA_JS_ETS_HYBRID_MODE)
class MockSTSVMInterface : public arkplatform::STSVMInterface {
public:
    void MarkFromObject(void *) override {}
    void OnVMAttach() override {}
    void OnVMDetach() override {}
    bool StartXGCBarrier(const NoWorkPred &) override
    {
        return true;
    }
    bool WaitForConcurrentMark(const NoWorkPred &) override
    {
        return true;
    }
    void RemarkStartBarrier() override {}
    bool WaitForRemark(const NoWorkPred &) override
    {
        return true;
    }
    void FinishXGCBarrier() override {}
    bool TriggerXGC() override
    {
        return true;
    }
    void NotifyWaiters() override {}
    bool GetStaticFrameInfo(const void *, arkplatform::HybridFrameInfo &) override
    {
        return false;
    }
    bool TriggerXGCAndWait() override
    {
        xgcTriggered_ = true;
        return xgcResult_;
    }
    void EtsForceFullGC() override {}
    void SuspendEtsThreads() override {}
    void ResumeEtsThreads() override {}
    std::vector<arkplatform::NodeInfo> GetEtsVMRoots() override
    {
        return {};
    }
    void GetEtsNodeEdges(uint64_t, std::vector<arkplatform::EdgeInfo> &, bool, bool) override {}
    arkplatform::NodeInfo GetEtsNodeInfo(uint64_t) override
    {
        return {};
    }
    std::vector<arkplatform::NodeInfo> GetAllEtsObjects() override
    {
        return {};
    }
    void IterateEtsObjects(const std::function<void(uint64_t)> &) override {}
    void GetXRefMaps(uintptr_t ecmaVM, XRefMap &jsToEts, XRefMap &etsToJs) override
    {
        (void)ecmaVM;
        xrefsCollected_ = true;
        if (onXRefsCollected_) {
            onXRefsCollected_();
        }
        jsToEts = jsToEtsMappings_;
        etsToJs = etsToJsMappings_;
    }
    bool AttachCurrentThread() override
    {
        return true;
    }
    bool DetachCurrentThread() override
    {
        return true;
    }
    bool IsCurrentThreadAttached() override
    {
        return true;
    }
    bool ExecuteHeapDump(const common::dump::DumpRequest &, arkplatform::EcmaVMInterface *, bool) override
    {
        return false;
    }
    bool UnionStackIsEmpty(bool *isEmpty) override
    {
        if (isEmpty != nullptr) {
            *isEmpty = true;
        }
        return true;
    }
    bool ForEachFrameInUnionStack(const std::function<void(const void *, bool)> &) override
    {
        return true;
    }

    bool xgcTriggered_ = false;
    bool xgcResult_ = true;
    bool xrefsCollected_ = false;
    XRefMap jsToEtsMappings_;
    XRefMap etsToJsMappings_;
    std::function<void()> onXRefsCollected_;
};
#endif

}  // namespace ark::tooling::hprof::test

#endif  // PANDA_PLUGINS_ETS_TESTS_RUNTIME_TOOLING_HPROF_SESSION_TEST_COMMON_H
