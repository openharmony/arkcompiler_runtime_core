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

#ifndef PANDA_TOOLING_INSPECTOR_HPROF_HEAP_SNAPSHOT_SERIALIZER_H
#define PANDA_TOOLING_INSPECTOR_HPROF_HEAP_SNAPSHOT_SERIALIZER_H

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "hybrid/hybrid_heap_snapshot_info.h"

namespace ark::tooling::inspector {

enum class RootCategory : uint8_t { CLASS, FRAME, GLOBAL, VM, STRING, OTHER, ROOT_CATEGORY_COUNT };

struct RootInfo {
    uint64_t addr;  // ObjectHeader* of the root object
    RootCategory category;
};

struct HeapSnapshotModel {
    std::vector<arkplatform::NodeInfo> nodes;
    std::vector<arkplatform::EdgeInfo> edges;
    std::vector<RootInfo> roots;
};

class HeapSnapshotSerializer {
public:
    using ChunkWriteFn = std::function<void(std::string_view)>;

    static void Serialize(const HeapSnapshotModel &model, const ChunkWriteFn &writeChunk);

private:
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr size_t CATEGORY_COUNT = static_cast<size_t>(RootCategory::ROOT_CATEGORY_COUNT);

    class StreamWriter;
    struct SerializedEdge {
        uint64_t type;
        uint64_t nameOrIndex;
        uint64_t toNode;
    };

    explicit HeapSnapshotSerializer(const HeapSnapshotModel &model) : model_(model) {}

    void IndexNodes();
    void CollectEdges();
    void SerializeNodes(StreamWriter &w);
    void SerializeEdges(StreamWriter &w);
    uint32_t GetStringId(const std::string &str);

    const HeapSnapshotModel &model_;
    std::unordered_map<uint64_t, size_t> nodeIndexByAddr_;
    std::array<std::vector<size_t>, CATEGORY_COUNT> rootsByCategory_;
    std::vector<size_t> nonEmptyRootCategories_;
    std::vector<std::vector<SerializedEdge>> edgesByNode_;
    std::vector<std::string> strings_;
    std::unordered_map<std::string, uint32_t> strIndex_;
    size_t syntheticNodeCount_ {1U};
    size_t nodeCount_ {0U};
};

}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_HPROF_HEAP_SNAPSHOT_SERIALIZER_H
