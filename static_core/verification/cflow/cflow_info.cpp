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

#include "bytecode_instruction-inl.h"
#include "file_items.h"
#include "macros.h"
#include "include/runtime.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/method-inl.h"

#include "utils/logger.h"

#include "util/str.h"

#include "cflow_info.h"

#include <iomanip>

#include "cflow_iterate_inl.h"

#include "verification/jobs/job.h"
#include "verification/cflow/cflow_common.h"
#include "verification/public_internal.h"
#include "verification/verification_status.h"

#include "verifier_messages.h"

namespace panda::verifier {

VerificationStatus CflowMethodInfo::FillCodeMaps(Method const *method)
{
    auto status = IterateOverInstructions(
        addr_start_, addr_start_, addr_end_,
        [this, method]([[maybe_unused]] auto typ, uint8_t const *pc, size_t sz, bool exception_source,
                       [[maybe_unused]] auto tgt) -> std::optional<VerificationStatus> {
            SetFlag(pc, INSTRUCTION);
            if (exception_source) {
                SetFlag(pc, EXCEPTION_SOURCE);
            }
            if (tgt != nullptr) {  // a jump
                if (!IsAddrValid(tgt)) {
                    LOG_VERIFIER_CFLOW_INVALID_JUMP_OUTSIDE_METHOD_BODY(
                        method->GetFullName(), OffsetAsHexStr(addr_start_, tgt), OffsetAsHexStr(addr_start_, pc));
                    return VerificationStatus::ERROR;
                }
                SetFlag(tgt, JUMP_TARGET);
            }
            uint8_t const *next_inst_pc = &pc[sz];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (next_inst_pc == addr_end_) {
                return VerificationStatus::OK;
            }
            if (next_inst_pc > addr_end_) {
                LOG_VERIFIER_CFLOW_INVALID_INSTRUCTION(OffsetAsHexStr(addr_start_, pc));
                return VerificationStatus::ERROR;
            }
            return std::nullopt;
        });
    return status;
}

VerificationStatus CflowMethodInfo::ProcessCatchBlocks(Method const *method)
{
    using CatchBlock = panda_file::CodeDataAccessor::CatchBlock;

    LOG(DEBUG, VERIFIER) << "Tracing exception handlers.";

    auto status = VerificationStatus::OK;
    method->EnumerateCatchBlocks([&]([[maybe_unused]] uint8_t const *try_start_pc,
                                     [[maybe_unused]] uint8_t const *try_end_pc, CatchBlock const &catch_block) {
        auto catch_block_start = reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(addr_start_) +
                                                                   static_cast<uintptr_t>(catch_block.GetHandlerPc()));
        auto catch_block_end =
            &catch_block_start[catch_block.GetCodeSize()];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (catch_block_start > catch_block_end || catch_block_start >= addr_end_ || catch_block_end < addr_start_) {
            LOG_VERIFIER_CFLOW_BAD_CATCH_BLOCK_BOUNDARIES(OffsetAsHexStr(addr_start_, catch_block_start),
                                                          OffsetAsHexStr(addr_start_, catch_block_end));
            status = VerificationStatus::ERROR;
            return false;
        }
        if (catch_block_end == catch_block_start) {
            // special case, no need to iterate over instructions.
            return true;
        }

        handler_start_addresses_.push_back(catch_block_start);

        auto result = IterateOverInstructions(
            catch_block_start, addr_start_, addr_end_,
            [this, catch_block_end]([[maybe_unused]] auto typ, uint8_t const *pc, size_t sz,
                                    [[maybe_unused]] bool exception_source,
                                    [[maybe_unused]] auto tgt) -> std::optional<VerificationStatus> {
                SetFlag(pc, EXCEPTION_HANDLER);
                uint8_t const *next_inst_pc = &pc[sz];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                if (next_inst_pc == catch_block_end) {
                    return VerificationStatus::OK;
                }
                if (next_inst_pc > catch_block_end) {
                    LOG_VERIFIER_CFLOW_INVALID_INSTRUCTION(OffsetAsHexStr(addr_start_, pc));
                    return VerificationStatus::ERROR;
                }
                return std::nullopt;
            });
        status = std::max(status, result);
        return true;
    });

    // Serves as a barrier
    auto end_code = reinterpret_cast<uint8_t const *>(reinterpret_cast<uintptr_t>(method->GetInstructions()) +
                                                      method->GetCodeSize());
    handler_start_addresses_.push_back(end_code);
    std::sort(handler_start_addresses_.begin(), handler_start_addresses_.end());

    return status;
}

PandaUniquePtr<CflowMethodInfo> GetCflowMethodInfo(Method const *method)
{
    const uint8_t *method_pc_start_ptr = method->GetInstructions();
    size_t code_size = method->GetCodeSize();

    auto cflow_info = MakePandaUnique<CflowMethodInfo>(method_pc_start_ptr, code_size);

    LOG(DEBUG, VERIFIER) << "Build control flow info for method " << method->GetFullName();

    // 1. fill instructions map
    LOG(DEBUG, VERIFIER) << "Build instructions map.";
    if (cflow_info->FillCodeMaps(method) == VerificationStatus::ERROR) {
        return {};
    }

    // 2. Mark exception handlers.
    if (cflow_info->ProcessCatchBlocks(method) == VerificationStatus::ERROR) {
        return {};
    }

    return cflow_info;
}

}  // namespace panda::verifier
