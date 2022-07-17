/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_VERIFICATION_CFLOW_INFO_HPP_
#define PANDA_VERIFICATION_CFLOW_INFO_HPP_

#include "instructions_map.h"
#include "jumps_map.h"
#include "exception_source_map.h"

#include "runtime/include/mem/panda_containers.h"

#include "runtime/include/mem/panda_smart_pointers.h"

#include "verification/jobs/cache.h"

#include <cstdint>
#include <optional>

namespace panda::verifier {
enum class InstructionType { NORMAL, JUMP, COND_JUMP, RETURN, THROW };

struct CflowBlockInfo {
    const uint8_t *Start;
    const uint8_t *End;
    JumpsMap JmpsMap;
    InstructionType LastInstType;
};

struct CflowExcHandlerInfo {
    CflowBlockInfo TryBlock;
    const uint8_t *Start;
    size_t Size;  // Note. In Java the catch block size is not defined
    OptionalConstRef<LibCache::CachedClass> CachedException;
};

class CflowMethodInfo {
public:
    CflowMethodInfo() = delete;
    CflowMethodInfo(const uint8_t *addr_start, size_t code_size)
        : InstMap_ {addr_start, code_size}, JmpsMap_ {addr_start, code_size}, ExcSrcMap_ {addr_start, code_size}
    {
    }
    ~CflowMethodInfo() = default;
    const InstructionsMap &InstMap() const
    {
        return InstMap_;
    }
    const JumpsMap &JmpsMap() const
    {
        return JmpsMap_;
    }
    const ExceptionSourceMap &ExcSrcMap() const
    {
        return ExcSrcMap_;
    }
    const PandaVector<CflowBlockInfo> &BodyBlocks() const
    {
        return BodyBlocks_;
    }
    const PandaVector<CflowBlockInfo> &ExcTryBlocks() const
    {
        return ExcTryBlocks_;
    }
    const PandaVector<CflowExcHandlerInfo> &ExcHandlers() const
    {
        return ExcHandlers_;
    }

private:
    InstructionsMap InstMap_;
    JumpsMap JmpsMap_;
    ExceptionSourceMap ExcSrcMap_;
    PandaVector<CflowBlockInfo> BodyBlocks_;
    PandaVector<CflowBlockInfo> ExcTryBlocks_;
    PandaVector<CflowExcHandlerInfo> ExcHandlers_;
    friend PandaUniquePtr<CflowMethodInfo> GetCflowMethodInfo(const LibCache::CachedMethod &method, LibCache &cache);
};

PandaUniquePtr<CflowMethodInfo> GetCflowMethodInfo(const LibCache::CachedMethod &method, LibCache &cache);
}  // namespace panda::verifier

#endif  // !PANDA_VERIFICATION_CFLOW_INFO_HPP_
