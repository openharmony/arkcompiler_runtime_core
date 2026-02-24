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

#ifndef PANDA_RUNTIME_TOOLING_BACKTRACE_SYMBOL_EXTRACTOR_H
#define PANDA_RUNTIME_TOOLING_BACKTRACE_SYMBOL_EXTRACTOR_H

#include "libarkfile/file.h"
#include "runtime/tooling/backtrace/base_defs.h"

namespace ark::tooling {

class ArkSymbolExtractor final {
public:
    static ArkSymbolExtractor *Create();
    static bool Destroy(ArkSymbolExtractor *extractor);

    void CreateArkFileData(uint8_t *data, uintptr_t dataSize, uintptr_t loadOffset);
    const panda_file::File *GetArkPandaFile();
    const std::vector<MethodInfo> &GetMethodInfos();
    void SetMethodInfos(std::vector<MethodInfo> methodInfos);
    bool HasMethodInfos();
    uint8_t *GetData();
    uintptr_t GetLoadOffset();
    uintptr_t GetDataSize();

private:
    ArkSymbolExtractor() = default;

    NO_COPY_SEMANTIC(ArkSymbolExtractor);
    NO_MOVE_SEMANTIC(ArkSymbolExtractor);

    ~ArkSymbolExtractor() = default;

    void CreateArkPandaFile();

    std::vector<MethodInfo> methodInfos_;
    uintptr_t loadOffset_ {0};
    uintptr_t dataSize_ {0};
    uint8_t *data_ {nullptr};
    std::unique_ptr<const panda_file::File> arkPandaFile_ {nullptr};
};

}  // namespace ark::tooling

#endif  // PANDA_RUNTIME_TOOLING_BACKTRACE_SYMBOL_EXTRACTOR_H
