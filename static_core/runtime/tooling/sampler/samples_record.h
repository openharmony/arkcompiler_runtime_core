/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_RECORD_H
#define PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_RECORD_H

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "libarkbase/os/mutex.h"
#include "libarkfile/file.h"
#include "sample_info.h"

namespace ark::tooling::sampler {
const int MAX_NODE_COUNT = 20000;  // 20000:the maximum size of the array

struct FrameInfo {
    int scriptId = 0;
    int lineNumber = -1;
    int columnNumber = -1;
    std::string functionName;
    std::string moduleName;
    std::string url;
    bool isStaticFrame = true;  // true for ETS (static), false for JS (dynamic)
};

struct CpuProfileNode {
    int id = 0;
    int parentId = 0;
    int hitCount = 0;
    FrameInfo codeEntry;
    std::vector<int> children;
};

struct ProfileInfo {
    uint64_t tid = 0;
    uint64_t startTime = 0;
    uint64_t stopTime = 0;
    std::array<CpuProfileNode, MAX_NODE_COUNT> nodes;
    int nodeCount = 0;
    std::vector<int> samples;
    std::vector<int> timeDeltas;
    // state time statistic
    uint64_t gcTime = 0;
    uint64_t cInterpreterTime = 0;
    uint64_t asmInterpreterTime = 0;
    uint64_t aotTime = 0;
    uint64_t builtinTime = 0;
    uint64_t napiTime = 0;
    uint64_t arkuiEngineTime = 0;
    uint64_t runtimeTime = 0;
    uint64_t otherTime = 0;
};

struct MethodKey {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes,-warnings-as-errors)
    SampleInfo::ManagedStackFrameId frameId;
    int lineNumber = -1;
    // NOLINTEND(misc-non-private-member-variables-in-classes,-warnings-as-errors)
    bool operator<(const MethodKey &methodKey) const
    {
        return std::tie(lineNumber, frameId) < std::tie(methodKey.lineNumber, methodKey.frameId);
    }
};

struct NodeKey {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes,-warnings-as-errors)
    MethodKey methodKey;
    int parentId = 0;
    // NOLINTEND(misc-non-private-member-variables-in-classes,-warnings-as-errors)
    bool operator<(const NodeKey &nodeKey) const
    {
        return std::tie(parentId, methodKey) < std::tie(nodeKey.parentId, nodeKey.methodKey);
    }
};

// Owned wrapper around SampleInfo that deep-copies ext frame data into heap memory.
// This ensures dynamic frame data survives beyond the shared plugin slot's lifetime.
struct OwnedSampleInfo {
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    SampleInfo sample;
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    std::vector<std::unique_ptr<uint8_t[]>> ownedBuffers;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    explicit OwnedSampleInfo(const SampleInfo &src) : sample(src)
    {
        for (size_t i = 0; i < src.stackInfo.managedStackSize; i++) {
            auto &frame = sample.stackInfo.managedStack[i];
            if (frame.pandaFilePtr == helpers::ToUnderlying(FrameKind::EXTERNAL_FRAME) &&
                frame.extFrameData != nullptr && frame.extFrameDataSize > 0) {
                // NOLINTNEXTLINE(modernize-avoid-c-arrays)
                auto buf = std::make_unique<uint8_t[]>(frame.extFrameDataSize);
                [[maybe_unused]] int r =
                    memcpy_s(buf.get(), frame.extFrameDataSize, frame.extFrameData, frame.extFrameDataSize);
                ASSERT(r == 0);
                frame.extFrameData = buf.get();
                ownedBuffers.push_back(std::move(buf));
            }
        }
    }
};

class SamplesRecord {
public:
    SamplesRecord() = default;
    PANDA_PUBLIC_API std::unique_ptr<std::vector<std::unique_ptr<ProfileInfo>>> GetAllThreadsProfileInfos();
    void SetThreadStartTime(uint64_t threadStartTime)
    {
        threadStartTime_ = threadStartTime;
    };
    void AddSampleInfo(uint32_t threadId, std::unique_ptr<OwnedSampleInfo> sampleInfo)
    {
        os::memory::LockHolder holder(addSamplInfoLock_);
        tidToSampleInfosMap_[threadId].emplace_back(std::move(sampleInfo));
    };

private:
    void NodeInit(ProfileInfo &profileInfo);
    using SampleInfoVector = std::vector<std::unique_ptr<OwnedSampleInfo>>;
    std::unique_ptr<ProfileInfo> GetSingleThreadProfileInfo(const SampleInfoVector &sampleInfos);
    void BuildStackInfoMap(const SampleInfo &sampleInfo);
    FrameInfo *GetFrameInfoByFrameId(const SampleInfo::ManagedStackFrameId &frameId);
    void ProcessSingleCallStackData(const SampleInfo &sampleInfo, ProfileInfo &profileInfo, uint64_t &prevTimeStamp);

    FrameInfo BuildDynamicFrameInfo(const uint8_t *buffer, size_t size);
    FrameInfo BuildStaticFrameInfo(const SampleInfo::ManagedStackFrameId &frameId);
    int GetOrAssignScriptId(const std::string &url);
    std::string GetUrlFromClassData(const panda_file::File *pfId, panda_file::File::EntityId classId);

    uint64_t threadStartTime_ = 0;
    os::memory::Mutex addSamplInfoLock_;
    std::map<NodeKey, int> nodesMap_ = {};
    std::map<std::string, size_t> scriptIdMap_ = {};
    std::map<SampleInfo::ManagedStackFrameId, FrameInfo> stackInfoMap_;
    std::unordered_map<uint32_t, SampleInfoVector> tidToSampleInfosMap_ = {};
};
}  // namespace ark::tooling::sampler

#endif  // PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLE_RECORD_H
