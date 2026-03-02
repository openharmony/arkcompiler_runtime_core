/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef LIBABCKIT_SRC_WRAPPERS_GRAPH_WRAPPER_H
#define LIBABCKIT_SRC_WRAPPERS_GRAPH_WRAPPER_H

#include "libabckit/c/metadata_core.h"
#include "libabckit/c/statuses.h"
#include "libabckit/src/ir_impl.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "libabckit/src/wrappers/abcfile_wrapper.h"
#include "libabckit/src/wrappers/pandasm_wrapper.h"

namespace libabckit {

class GraphWrapper {
public:
    using LineColumnPair = std::pair<size_t, uint32_t>;
    static void CreateGraphWrappers(AbckitGraph *graph);
    static AbckitGraph *BuildGraphDynamic(FileWrapper *pf, AbckitIrInterface *irInterface, AbckitFile *file,
                                          uint32_t methodOffset);
    static void *BuildCodeDynamic(AbckitGraph *graph, const std::string &funcName,
                                  std::unordered_map<size_t, LineColumnPair> *pcToLineColumn = nullptr);
    static void BuildPcToLineColumnMap(AbckitGraph *graph, const std::vector<LineColumnPair> &lineColumnPerIns,
                                       std::unordered_map<size_t, LineColumnPair> &outMap);
    static void DestroyGraphDynamic(AbckitGraph *graph);
};

}  // namespace libabckit

#endif  // LIBABCKIT_SRC_WRAPPERS_GRAPH_WRAPPER_H
