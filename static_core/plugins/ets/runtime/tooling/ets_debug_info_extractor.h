/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TOOLING_ETS_DEBUG_INFO_EXTRACTOR_H
#define PANDA_PLUGINS_ETS_RUNTIME_TOOLING_ETS_DEBUG_INFO_EXTRACTOR_H

#include "disassembler/disasm_backed_debug_info_extractor.h"

#include "method.h"
#include "runtime/include/typed_value.h"

#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ark::ets::tooling {
// TODO: consider removing this class
class EtsDebugInfoExtractor final : public disasm::DisasmBackedDebugInfoExtractor {
public:
    explicit EtsDebugInfoExtractor(const panda_file::File &file)
        : disasm::DisasmBackedDebugInfoExtractor(file), pf_(&file)
    {
    }
    ~EtsDebugInfoExtractor() = default;

    DEFAULT_COPY_SEMANTIC(EtsDebugInfoExtractor);
    DEFAULT_MOVE_SEMANTIC(EtsDebugInfoExtractor);

    const panda_file::LocalVariableTable &GetLocalsWithArguments(Frame *frame);

private:
    const panda_file::File *pf_ {nullptr};
    std::unordered_map<const Method *, panda_file::LocalVariableTable> localsCache_;
};
}  // namespace ark::ets::tooling

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TOOLING_ETS_DEBUG_INFO_EXTRACTOR_H
