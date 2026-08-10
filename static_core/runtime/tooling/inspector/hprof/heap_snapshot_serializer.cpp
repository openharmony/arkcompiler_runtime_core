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

#include "hprof/heap_snapshot_serializer.h"

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ark::tooling::inspector {
namespace {

constexpr size_t NODE_FIELDS = 7;
constexpr size_t CHUNK_SIZE = 65536;

constexpr uint64_t NODE_TYPE_ARRAY = 1;
constexpr uint64_t NODE_TYPE_STRING = 2;
constexpr uint64_t NODE_TYPE_OBJECT = 3;
constexpr uint64_t NODE_TYPE_SYNTHETIC = 9;

constexpr uint64_t EDGE_TYPE_ELEMENT = 1;
constexpr uint64_t EDGE_TYPE_PROPERTY = 2;
constexpr uint64_t EDGE_TYPE_WEAK = 6;

constexpr std::string_view META_SCHEMA_PREFIX =
    R"HS({"snapshot":{"meta":{)HS"
    R"HS("node_fields":["type","name","id","self_size","edge_count","trace_node_id","detachedness"],)HS"
    R"HS("node_types":[["hidden","array","string","object","code","closure","regexp","number",)HS"
    R"HS("native","synthetic","concatenated string","sliced string","symbol","bigint","object shape"],)HS"
    R"HS("string","number","number","number","number","number"],)HS"
    R"HS("edge_fields":["type","name_or_index","to_node"],)HS"
    R"HS("edge_types":[["context","element","property","internal","hidden","shortcut","weak"],)HS"
    R"HS("string_or_number","node"],)HS"
    R"HS("trace_function_info_fields":["function_id","name","script_name","script_id","line","column"],)HS"
    R"HS("trace_node_fields":["id","function_info_index","count","size","children"],)HS"
    R"HS("sample_fields":["timestamp_us","last_assigned_id"],)HS"
    R"HS("location_fields":["object_index","script_id","line","column"]},)HS";

constexpr std::array<std::string_view, static_cast<size_t>(RootCategory::ROOT_CATEGORY_COUNT)> CATEGORY_NAMES = {
    "Class roots", "Frame roots", "Global roots", "VM roots", "String roots", "Other roots"};

uint64_t NodeTypeIndex(arkplatform::StaticNodeType t)
{
    switch (t) {
        case arkplatform::StaticNodeType::ARRAY:
            return NODE_TYPE_ARRAY;
        case arkplatform::StaticNodeType::STRING:
            return NODE_TYPE_STRING;
        case arkplatform::StaticNodeType::CLASS:
        case arkplatform::StaticNodeType::OBJECT:
        default:
            return NODE_TYPE_OBJECT;
    }
}

}  // namespace

class HeapSnapshotSerializer::StreamWriter {
public:
    explicit StreamWriter(const ChunkWriteFn &writeChunk) : writeChunk_(writeChunk)
    {
        buf_.reserve(CHUNK_SIZE + 64U);
    }

    void Append(std::string_view str)
    {
        buf_.append(str);
        MaybeFlush();
    }

    void Append(char c)
    {
        buf_.push_back(c);
        MaybeFlush();
    }

    void AppendNumber(uint64_t number)
    {
        buf_.append(std::to_string(number));
        MaybeFlush();
    }

    void AppendEscaped(const std::string &str)
    {
        buf_.push_back('"');
        size_t i = 0;
        size_t n = str.size();
        while (i < n) {
            auto c = static_cast<uint8_t>(str[i]);
            if (c == '"' || c == '\\') {
                buf_.push_back('\\');
                buf_.push_back(static_cast<char>(c));
                i++;
            } else if (c == '\n') {
                buf_.append("\\n");
                i++;
            } else if (c == '\r') {
                buf_.append("\\r");
                i++;
            } else if (c == '\t') {
                buf_.append("\\t");
                i++;
            } else if (c == '\b') {
                buf_.append("\\b");
                i++;
            } else if (c == '\f') {
                buf_.append("\\f");
                i++;
            } else if (c < 0x20U) {
                AppendUnicodeEscape(c);
                i++;
            } else if (c < 0x80U) {
                buf_.push_back(static_cast<char>(c));
                i++;
            } else {
                i += AppendUtf8OrReplace(str, i, n);
            }
        }
        buf_.push_back('"');
        MaybeFlush();
    }

    void Flush()
    {
        if (!buf_.empty()) {
            writeChunk_(buf_);
            buf_.clear();
        }
    }

private:
    void MaybeFlush()
    {
        if (buf_.size() >= CHUNK_SIZE) {
            Flush();
        }
    }

    void AppendUnicodeEscape(uint8_t byte)
    {
        // CC-OFFNXT(G.NAM.03-CPP) project code style
        static constexpr char HEX[] = "0123456789abcdef";
        buf_.append("\\u00");
        buf_.push_back(HEX[byte >> 4U]);
        buf_.push_back(HEX[byte & 0xFU]);
    }

    size_t AppendUtf8OrReplace(const std::string &str, size_t i, size_t strSize)
    {
        auto c = static_cast<uint8_t>(str[i]);
        size_t len = 0;
        if ((c & 0xE0U) == 0xC0U) {
            len = 2U;
        } else if ((c & 0xF0U) == 0xE0U) {
            len = 3U;
        } else if ((c & 0xF8U) == 0xF0U) {
            len = 4U;
        }
        if (len == 0 || i + len > strSize) {
            buf_.push_back('?');
            return 1;
        }
        for (size_t k = 1; k < len; ++k) {
            if ((static_cast<uint8_t>(str[i + k]) & 0xC0U) != 0x80U) {
                buf_.push_back('?');
                return 1;
            }
        }

        if (c == 0xC0U || c == 0xC1U || (c == 0xEDU && static_cast<uint8_t>(str[i + 1]) >= 0xA0U)) {
            buf_.push_back('?');
            return len;
        }
        buf_.append(str, i, len);
        return len;
    }

    const ChunkWriteFn &writeChunk_;
    std::string buf_;
};

uint32_t HeapSnapshotSerializer::GetStringId(const std::string &str)
{
    auto it = strIndex_.find(str);
    if (it != strIndex_.end()) {
        return it->second;
    }
    auto idx = static_cast<uint32_t>(strings_.size());
    strings_.push_back(str);
    strIndex_.emplace(str, idx);
    return idx;
}

void HeapSnapshotSerializer::IndexNodes()
{
    const auto &objNodes = model_.nodes;
    size_t objCount = objNodes.size();

    nodeIndexByAddr_.reserve(objCount * 2U);
    for (size_t i = 0; i < objCount; ++i) {
        nodeIndexByAddr_.emplace(objNodes[i].addr, i);
    }

    std::unordered_set<size_t> seenRoots;
    for (const RootInfo &rootInfo : model_.roots) {
        auto it = nodeIndexByAddr_.find(rootInfo.addr);
        if (it == nodeIndexByAddr_.end()) {
            continue;
        }
        size_t nodeIndex = it->second;
        bool isNewRoot = seenRoots.insert(nodeIndex).second;
        if (!isNewRoot) {
            continue;
        }
        rootsByCategory_[static_cast<size_t>(rootInfo.category)].push_back(nodeIndex);
    }

    for (size_t k = 0; k < rootsByCategory_.size(); ++k) {
        if (!rootsByCategory_[k].empty()) {
            nonEmptyRootCategories_.push_back(k);
        }
    }
    size_t rootCategoryCount = nonEmptyRootCategories_.size();
    syntheticNodeCount_ = rootCategoryCount > 0 ? (1U + rootCategoryCount) : 1U;
    nodeCount_ = syntheticNodeCount_ + objCount;
}

void HeapSnapshotSerializer::CollectEdges()
{
    size_t objCount = model_.nodes.size();
    size_t rootCategoryCount = nonEmptyRootCategories_.size();
    edgesByNode_.assign(nodeCount_, {});

    if (rootCategoryCount > 0) {
        for (size_t i = 0; i < rootCategoryCount; ++i) {
            // synthetic root node at index 0, need to link to each category node
            edgesByNode_[0].push_back({EDGE_TYPE_ELEMENT, i, 1U + i});
        }
        for (size_t i = 0; i < rootCategoryCount; ++i) {
            const auto &objs = rootsByCategory_[nonEmptyRootCategories_[i]];
            for (size_t j = 0; j < objs.size(); ++j) {
                // collect edges from each category node
                edgesByNode_[1U + i].push_back({EDGE_TYPE_ELEMENT, j, syntheticNodeCount_ + objs[j]});
            }
        }
    } else {
        edgesByNode_[0].reserve(objCount);
        for (size_t i = 0; i < objCount; ++i) {
            edgesByNode_[0].push_back({EDGE_TYPE_ELEMENT, static_cast<uint64_t>(i), syntheticNodeCount_ + i});
        }
    }

    for (const auto &edge : model_.edges) {
        auto fromIt = nodeIndexByAddr_.find(edge.fromAddr);
        auto toIt = nodeIndexByAddr_.find(edge.toAddr);
        if (fromIt == nodeIndexByAddr_.end() || toIt == nodeIndexByAddr_.end()) {
            continue;
        }
        uint64_t edgeType;
        uint64_t nameOrIndex;
        switch (edge.edgeType) {
            case arkplatform::StaticEdgeType::ELEMENT:
                edgeType = EDGE_TYPE_ELEMENT;
                nameOrIndex = edge.index;
                break;
            case arkplatform::StaticEdgeType::WEAK:
                edgeType = EDGE_TYPE_WEAK;
                nameOrIndex = GetStringId(edge.name);
                break;
            case arkplatform::StaticEdgeType::PROPERTY:
            default:
                edgeType = EDGE_TYPE_PROPERTY;
                nameOrIndex = GetStringId(edge.name);
                break;
        }
        edgesByNode_[syntheticNodeCount_ + fromIt->second].push_back(
            {edgeType, nameOrIndex, syntheticNodeCount_ + toIt->second});
    }
}

void HeapSnapshotSerializer::SerializeNodes(StreamWriter &w)
{
    auto writeNode = [&w](bool first, uint64_t type, uint64_t name, uint64_t id, uint64_t selfSize,
                          uint64_t edgeCount) {
        if (!first) {
            w.Append(',');
        }
        w.AppendNumber(type);
        w.Append(',');
        w.AppendNumber(name);
        w.Append(',');
        w.AppendNumber(id);
        w.Append(',');
        w.AppendNumber(selfSize);
        w.Append(',');
        w.AppendNumber(edgeCount);
        w.Append(",0,0");
    };

    uint32_t rootNameIdx = GetStringId("(GC roots)");

    writeNode(true, NODE_TYPE_SYNTHETIC, rootNameIdx, 1U, 0U, edgesByNode_[0].size());
    size_t rootCategoryCount = nonEmptyRootCategories_.size();
    if (rootCategoryCount > 0) {
        for (size_t i = 0; i < rootCategoryCount; ++i) {
            size_t idx = 1U + i;
            writeNode(false, NODE_TYPE_SYNTHETIC, GetStringId(std::string(CATEGORY_NAMES[nonEmptyRootCategories_[i]])),
                      2U * idx + 1U, 0U, edgesByNode_[idx].size());
        }
    }
    size_t objCount = model_.nodes.size();
    for (size_t i = 0; i < objCount; ++i) {
        const auto &nInfo = model_.nodes[i];
        size_t idx = syntheticNodeCount_ + i;
        writeNode(false, NodeTypeIndex(nInfo.nodeType), GetStringId(nInfo.name), 2U * idx + 1U, nInfo.size,
                  edgesByNode_[idx].size());
    }
}

void HeapSnapshotSerializer::SerializeEdges(StreamWriter &w)
{
    bool firstEdge = true;
    for (const auto &edges : edgesByNode_) {
        for (const auto &edge : edges) {
            if (!firstEdge) {
                w.Append(',');
            }
            firstEdge = false;
            w.AppendNumber(edge.type);
            w.Append(',');
            w.AppendNumber(edge.nameOrIndex);
            w.Append(',');
            w.AppendNumber(edge.toNode * NODE_FIELDS);
        }
    }
}

void HeapSnapshotSerializer::Serialize(const HeapSnapshotModel &model, const ChunkWriteFn &writeChunk)
{
    HeapSnapshotSerializer serializer(model);
    serializer.IndexNodes();
    serializer.CollectEdges();

    size_t totalEdges = 0;
    for (const auto &edges : serializer.edgesByNode_) {
        totalEdges += edges.size();
    }

    StreamWriter w(writeChunk);

    w.Append(META_SCHEMA_PREFIX);
    w.Append(R"("node_count":)");
    w.AppendNumber(serializer.nodeCount_);
    w.Append(R"(,"edge_count":)");
    w.AppendNumber(totalEdges);
    w.Append(R"(,"trace_function_count":0},)");

    w.Append(R"("nodes":[)");
    serializer.SerializeNodes(w);
    w.Append(R"(],)");

    w.Append(R"("edges":[)");
    serializer.SerializeEdges(w);
    w.Append(R"(],)");

    w.Append(R"("trace_function_infos":[],"trace_tree":[],"samples":[],"locations":[],)");

    w.Append(R"("strings":[)");
    for (size_t i = 0; i < serializer.strings_.size(); ++i) {
        if (i != 0) {
            w.Append(',');
        }
        w.AppendEscaped(serializer.strings_[i]);
    }
    w.Append("]}");

    w.Flush();
}

}  // namespace ark::tooling::inspector
