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

#ifndef PANDA_ARKPLATFORM_HYBRID_HEAP_SNAPSHOT_INFO_H
#define PANDA_ARKPLATFORM_HYBRID_HEAP_SNAPSHOT_INFO_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace arkplatform {

// Only includes types actually used by ArkTS-Sta static side.
// Mapping to ets_runtime NodeType/EdgeType is done on the dynamic side via switch-case.
enum class StaticNodeType {
    ARRAY,
    STRING,
    OBJECT,
    CLASS,
    DEFAULT = OBJECT
};

enum class StaticEdgeType {
    ELEMENT,
    PROPERTY,
    WEAK,
    DEFAULT = PROPERTY
};

struct NodeInfo {
    std::string name;
    StaticNodeType nodeType;
    size_t size;
    size_t nativeSize;
    uint64_t addr;  // ObjectHeader* cast to uint64_t, or JSTaggedType directly
};

struct EdgeInfo {
    StaticEdgeType edgeType;
    uint64_t fromAddr;
    uint64_t toAddr;
    std::string name;   // for property edges
    uint32_t index;     // for element edges
};

}  // namespace arkplatform

#endif  // PANDA_ARKPLATFORM_HYBRID_HEAP_SNAPSHOT_INFO_H
