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
#include "verification/util/str.h"
#include "verifier_messages.h"

#include <iomanip>
#include <optional>

namespace panda::verifier {

template <class F>
static bool CheckCode(const CflowMethodInfo &cflow_info, const uint8_t *method_pc_end_ptr,
                      const F &report_incorrect_jump)
{
    // check method code jumps (body + exc handlers, i.e all code)
    {
        const uint8_t *pc_jump_ptr = nullptr;
        const uint8_t *pc_target_ptr = nullptr;
        if (cflow_info.JmpsMap().GetFirstConflictingJump<const uint8_t *>(cflow_info.InstMap(), &pc_jump_ptr,
                                                                          &pc_target_ptr)) {
            report_incorrect_jump(pc_jump_ptr, pc_target_ptr,
                                  "Invalid jump in the method body into middle of instruction.");
            return false;
        }
    }

    // check method body last instruction, if body spans till the method end
    {
        const auto &last_body_block = cflow_info.BodyBlocks().back();
        if (last_body_block.End == method_pc_end_ptr && last_body_block.LastInstType != InstructionType::RETURN &&
            last_body_block.LastInstType != InstructionType::THROW &&
            last_body_block.LastInstType != InstructionType::JUMP) {
            LOG(ERROR, VERIFIER) << "Invalid last instruction in method, execution beyond the method code boundary.";
            return false;
        }
    }

    return true;
}

static bool IsExceptionHanlderStart(const CflowMethodInfo &cflow_info, const uint8_t *inst)
{
    for (const CflowExcHandlerInfo &handler : cflow_info.ExcHandlers()) {
        if (handler.Start == inst) {
            return true;
        }
    }
    return false;
}

/**
 *  Checking two statements:
 *    - jumps into body of exception handler from code is prohibited
 *    - fallthrough on beginning of exception handler is prohibited
 *  The statements are not declared by Java Spec, it is a demand of Panda compiler.
 */
static bool CheckInvalidCatchBlockEnter(const LibCache::CachedMethod &method, const CflowMethodInfo &cflow_info,
                                        const MethodOptions &options)
{
    const uint8_t *method_start = method.bytecode;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const uint8_t *method_end = &method_start[method.bytecode_size - 1];
    bool status_ok = true;

    auto cflow_handler = [&cflow_info, &method, &status_ok, &options](auto type, const uint8_t *pc, size_t sz,
                                                                      [[maybe_unused]] bool exception_source,
                                                                      auto target) -> std::optional<CflowStatus> {
        if (type != InstructionType::JUMP && type != InstructionType::THROW && type != InstructionType::RETURN) {
            const uint8_t *next_inst = &pc[sz];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (IsExceptionHanlderStart(cflow_info, next_inst)) {
                LOG_VERIFIER_CFLOW_BODY_FALL_INTO_EXC_HANDLER(LibCache::GetName(method),
                                                              (OffsetAsHexStr(method.bytecode, pc)));
                status_ok = false;
                return CflowStatus::ERROR;
            }
        }

        if (type != InstructionType::JUMP && type != InstructionType::COND_JUMP) {
            return std::nullopt;
        }

        for (const CflowExcHandlerInfo &handler : cflow_info.ExcHandlers()) {
            bool isInvalidJump = (target == handler.Start && handler.Size == 0) ||
                                 // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                                 (target >= handler.Start && target < handler.Start + handler.Size);

            if (isInvalidJump && (options.Msg(VerifierMessage::CflowInvalidJumpIntoExcHandler).IsError())) {
                status_ok = false;
            }

            if (isInvalidJump) {
                LOG_VERIFIER_CFLOW_INVALID_JUMP_INTO_EXC_HANDLER(LibCache::GetName(method),
                                                                 (OffsetAsHexStr(method.bytecode, pc)));
                return CflowStatus::ERROR;
            }
        }
        return std::nullopt;
    };

    IterateOverInstructions(method_start, method_start, method_end, cflow_handler);
    return status_ok;
}

PandaUniquePtr<CflowMethodInfo> CheckCflow(const LibCache::CachedMethod &method, const MethodOptions &options,
                                           LibCache &cache)
{
    auto cflow_info = GetCflowMethodInfo(method, cache);
    if (!cflow_info) {
        return {};
    }

    const uint8_t *method_pc_start_ptr = method.bytecode;
    const uint8_t *method_pc_end_ptr =
        &method_pc_start_ptr[method.bytecode_size - 1];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    auto report_incorrect_jump = [&method, &method_pc_start_ptr](const uint8_t *jump_pc, const uint8_t *jump_target,
                                                                 const char *msg) {
        LOG_VERIFIER_CFLOW_INVALID_JUMP_TARGET(LibCache::GetName(method),
                                               (OffsetAsHexStr(method_pc_start_ptr, jump_target)),
                                               (OffsetAsHexStr(method_pc_start_ptr, jump_pc)), msg);
    };

    if (!CheckCode(*cflow_info, method_pc_end_ptr, report_incorrect_jump)) {
        return {};
    }

    if (!CheckInvalidCatchBlockEnter(method, *cflow_info, options)) {
        return {};
    }

    return cflow_info;
}

}  // namespace panda::verifier
