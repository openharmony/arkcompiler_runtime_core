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

#include "cflow_check.h"
#include "cflow_common.h"
#include "cflow_iterate_inl.h"
#include "runtime/include/method-inl.h"
#include "utils/logger.h"
#include "verification/jobs/job.h"
#include "verification/util/str.h"
#include "verifier_messages.h"

#include <iomanip>
#include <optional>

namespace panda::verifier {

static VerificationStatus CheckCode(Method const *method, CflowMethodInfo const *cflow_info)
{
    uint8_t const *method_start = method->GetInstructions();
    uint8_t const *method_end =
        method_start + method->GetCodeSize();  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic
    auto handler_starts = cflow_info->GetHandlerStartAddresses();
    size_t nandler_index = (*handler_starts)[0] == method_start ? 0 : -1;
    return IterateOverInstructions(
        method_start, method_start, method_end,
        [&](auto type, uint8_t const *pc, size_t size, [[maybe_unused]] bool exception_source,
            auto target) -> std::optional<VerificationStatus> {
            if (target != nullptr) {  // a jump
                if (!cflow_info->IsAddrValid(target)) {
                    LOG_VERIFIER_CFLOW_INVALID_JUMP_OUTSIDE_METHOD_BODY(
                        method->GetFullName(), OffsetAsHexStr(method_start, target), OffsetAsHexStr(method_start, pc));
                    return VerificationStatus::ERROR;
                }
                if (!cflow_info->IsFlagSet(target, CflowMethodInfo::INSTRUCTION)) {
                    LOG_VERIFIER_CFLOW_INVALID_JUMP_INTO_MIDDLE_OF_INSTRUCTION(
                        method->GetFullName(), OffsetAsHexStr(method_start, target), OffsetAsHexStr(method_start, pc));
                    return VerificationStatus::ERROR;
                }
                if (cflow_info->IsFlagSet(target, CflowMethodInfo::EXCEPTION_HANDLER)) {
                    if (!cflow_info->IsFlagSet(pc, CflowMethodInfo::EXCEPTION_HANDLER)) {
                        // - jumps into body of exception handler from code is prohibited by Panda compiler.
                        LOG_VERIFIER_CFLOW_INVALID_JUMP_INTO_EXC_HANDLER(
                            method->GetFullName(), (OffsetAsHexStr(method->GetInstructions(), pc)));
                        return VerificationStatus::ERROR;
                    }
                    // Jump from handler to handler; need to make sure it's the same one.
                    if (target < (*handler_starts)[nandler_index] || target >= (*handler_starts)[nandler_index + 1]) {
                        LOG_VERIFIER_CFLOW_INVALID_JUMP_INTO_EXC_HANDLER(
                            method->GetFullName(), (OffsetAsHexStr(method->GetInstructions(), pc)));
                        return VerificationStatus::ERROR;
                    }
                }
            }
            uint8_t const *next_inst_pc = &pc[size];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic
            if (next_inst_pc == method_end) {
                if (type != InstructionType::THROW && type != InstructionType::JUMP &&
                    type != InstructionType::RETURN) {
                    LOG_VERIFIER_CFLOW_INVALID_LAST_INSTRUCTION(method->GetFullName());
                    return VerificationStatus::ERROR;
                }
                return VerificationStatus::OK;
            }
            if (next_inst_pc == (*handler_starts)[nandler_index + 1]) {
                if (type != InstructionType::JUMP && type != InstructionType::RETURN &&
                    type != InstructionType::THROW) {
                    // - fallthrough on beginning of exception handler is prohibited by Panda compiler
                    LOG_VERIFIER_CFLOW_BODY_FALL_INTO_EXC_HANDLER(method->GetFullName(),
                                                                  (OffsetAsHexStr(method->GetInstructions(), pc)));
                    return VerificationStatus::ERROR;
                }
                nandler_index++;
            }
            return std::nullopt;
        });
}

PandaUniquePtr<CflowMethodInfo> CheckCflow(Method const *method)
{
    auto cflow_info = GetCflowMethodInfo(method);
    if (!cflow_info) {
        return {};
    }

    if (CheckCode(method, cflow_info.get()) == VerificationStatus::ERROR) {
        return {};
    }

    return cflow_info;
}

}  // namespace panda::verifier
