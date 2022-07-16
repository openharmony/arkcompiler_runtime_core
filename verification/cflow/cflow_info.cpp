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

#include "utils/logger.h"

#include "util/str.h"

#include "cflow_status.h"
#include "cflow_info.h"

#include <iomanip>

#include "cflow_iterate_inl.h"

#include "verification/jobs/cache.h"
#include "verification/jobs/thread_pool.h"
#include "verification/cflow/cflow_common.h"

#include "verifier_messages.h"

namespace panda::verifier {

CflowStatus FillInstructionsMap(InstructionsMap *inst_map_ptr, ExceptionSourceMap *exc_src_map_ptr)
{
    auto &inst_map = *inst_map_ptr;
    auto &exc_src_map = *exc_src_map_ptr;
    auto status = IterateOverInstructions(
        inst_map.AddrStart<const uint8_t *>(), inst_map.AddrStart<const uint8_t *>(),
        inst_map.AddrEnd<const uint8_t *>(),
        [&inst_map, &exc_src_map]([[maybe_unused]] auto typ, const uint8_t *pc, size_t sz, bool exception_source,
                                  [[maybe_unused]] auto tgt) -> std::optional<CflowStatus> {
            if (!inst_map.PutInstruction(pc, sz)) {
                LOG_VERIFIER_CFLOW_INVALID_INSTRUCTION(OffsetAsHexStr(inst_map.AddrStart<void *>(), pc));
                return CflowStatus::ERROR;
            }
            if (exception_source && !exc_src_map.PutExceptionSource(pc)) {
                LOG_VERIFIER_CFLOW_INVALID_INSTRUCTION(OffsetAsHexStr(inst_map.AddrStart<void *>(), pc));
                return CflowStatus::ERROR;
            }
            const uint8_t *next_inst_pc = &pc[sz];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (next_inst_pc <= inst_map.AddrEnd<const uint8_t *>()) {
                return std::nullopt;
            }
            return CflowStatus::OK;
        });
    return status;
}

CflowStatus FillJumpsMapAndGetLastInstructionType(const InstructionsMap &inst_map, JumpsMap *jumps_map_ptr,
                                                  const uint8_t *pc_start_ptr, const uint8_t *pc_end_ptr,
                                                  InstructionType *last_inst_type_ptr)
{
    ASSERT(jumps_map_ptr != nullptr);

    JumpsMap &jumps_map = *jumps_map_ptr;
    auto result = IterateOverInstructions(
        pc_start_ptr, inst_map.AddrStart<const uint8_t *>(), inst_map.AddrEnd<const uint8_t *>(),
        [&pc_end_ptr, &inst_map, &jumps_map, last_inst_type_ptr](InstructionType typ, const uint8_t *pc, size_t sz,
                                                                 [[maybe_unused]] bool exception_source,
                                                                 const uint8_t *target) -> std::optional<CflowStatus> {
            const uint8_t *next_inst_pc = &pc[sz];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (typ == InstructionType::JUMP || typ == InstructionType::COND_JUMP) {
                if (!inst_map.CanJumpTo(target)) {
                    LOG_VERIFIER_CFLOW_INVALID_JUMP_INTO_MIDDLE_OF_INSTRUCTION(
                        OffsetAsHexStr(inst_map.AddrStart<void *>(), target),
                        OffsetAsHexStr(inst_map.AddrStart<void *>(), pc));
                    return CflowStatus::ERROR;
                }
                if (!jumps_map.PutJump(pc, target)) {
                    LOG_VERIFIER_CFLOW_INVALID_JUMP(OffsetAsHexStr(inst_map.AddrStart<void *>(), pc),
                                                    OffsetAsHexStr(inst_map.AddrStart<void *>(), target));
                    return CflowStatus::ERROR;
                }
            }
            // pc_end_ptr is expected to point to the last instruction/byte of the block
            if (next_inst_pc > pc_end_ptr) {
                // last instruction should terminate control-flow: jump, return, throw
                // conditional jumps is problem here, since condition in general could not be precisely
                // evaluated
                if (last_inst_type_ptr != nullptr) {
                    *last_inst_type_ptr = typ;
                }

                return CflowStatus::OK;
            }
            return std::nullopt;
        });
    return result;
}

CflowStatus FillCflowBlockInfo(const InstructionsMap &inst_map, CflowBlockInfo *code_block_info)
{
    return FillJumpsMapAndGetLastInstructionType(inst_map, &code_block_info->JmpsMap, code_block_info->Start,
                                                 code_block_info->End, &code_block_info->LastInstType);
}

template <class F>
static void DebugDump(const LibCache::CachedCatchBlock &catch_block, const F &get_offset)
{
    auto try_start_pc = catch_block.try_block_start;
    auto try_end_pc = catch_block.try_block_end;
    auto &exception = catch_block.exception_type;
    auto pc_start_ptr = catch_block.handler_bytecode;
    auto size = catch_block.handler_bytecode_size;

    bool catch_all = !LibCache::IsValid(exception);
    PandaString exc_name = catch_all ? PandaString {"null"} : LibCache::GetName(exception);
    auto try_range = PandaString {"[ "} + get_offset(try_start_pc) + ", " + get_offset(try_end_pc) + " ]";
    PandaString exc_handler_range;
    if (size == 0) {
        exc_handler_range = get_offset(pc_start_ptr);
    } else {
        exc_handler_range = PandaString {"[ "};
        exc_handler_range += get_offset(pc_start_ptr) + ", ";
        exc_handler_range += get_offset(&pc_start_ptr[size - 1]);  // NOLINT
        exc_handler_range += " ]";
    }
    LOG_VERIFIER_CFLOW_EXC_HANDLER_INFO(exc_handler_range, try_range, exc_name);
}

template <class F>
static bool ProcessCatchBlocks(const LibCache::CachedMethod &method, [[maybe_unused]] AddrMap *addr_map,
                               [[maybe_unused]] const F &get_offset, InstructionsMap *inst_map,
                               PandaVector<CflowExcHandlerInfo> *exc_handlers, LibCache &cache)
{
    bool result = true;

    LOG(DEBUG, VERIFIER) << "Tracing exception handlers.";

    for (const auto &catch_block : method.catch_blocks) {
        OptionalConstRef<LibCache::CachedClass> cached_class_of_exception = LibCache::Visit(
            catch_block.exception_type,
            [](const LibCache::CachedClass &cached_class) { return OptionalConstRef(cached_class); },
            [&cache, &method](const LibCache::DescriptorString &descriptor) {
                if (!descriptor.IsValid()) {
                    return OptionalConstRef<LibCache::CachedClass> {};  // catch_all
                }
                OptionalConstRef<CachedClass> cc = cache.FastAPI().ResolveAndLink(method.GetSourceLang(), descriptor);
                return cc;
            });

#ifndef NDEBUG
        DebugDump(catch_block, get_offset);
#endif
        CflowBlockInfo block_info {catch_block.try_block_start,
                                   catch_block.try_block_end,
                                   {inst_map->AddrStart<const uint8_t *>(), inst_map->AddrEnd<const uint8_t *>()},
                                   InstructionType::NORMAL};
        exc_handlers->push_back(
            {block_info, catch_block.handler_bytecode, catch_block.handler_bytecode_size, cached_class_of_exception});
        if (FillCflowBlockInfo(*inst_map, &exc_handlers->back().TryBlock) == CflowStatus::ERROR) {
            LOG_VERIFIER_CFLOW_CANNOT_FILL_JUMPS_OF_EXC_HANDLER_BLOCK();
            result = false;
            break;
        }
    }

    return result;
}

PandaUniquePtr<CflowMethodInfo> GetCflowMethodInfo(const LibCache::CachedMethod &method, LibCache &cache)
{
    const uint8_t *method_pc_start_ptr = method.bytecode;
    size_t code_size = method.bytecode_size;
    const uint8_t *method_pc_end_ptr =
        &method_pc_start_ptr[code_size - 1];  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

    auto cflow_info = MakePandaUnique<CflowMethodInfo>(method_pc_start_ptr, code_size);

    LOG(DEBUG, VERIFIER) << "Build control flow info for method " << method.GetName();

    // 1. fill instructions map
    LOG(DEBUG, VERIFIER) << "Build instructions map.";
    if (FillInstructionsMap(&(*cflow_info).InstMap_, &(*cflow_info).ExcSrcMap_) == CflowStatus::ERROR) {
        LOG_VERIFIER_CFLOW_CANNOT_FILL_INSTRUCTIONS_MAP()
        return {};
    }

    // 2. fill jumps map
    LOG(DEBUG, VERIFIER) << "Build jumps map.";
    if (FillJumpsMapAndGetLastInstructionType((*cflow_info).InstMap_, &(*cflow_info).JmpsMap_, method_pc_start_ptr,
                                              method_pc_end_ptr, nullptr) == CflowStatus::ERROR) {
        LOG_VERIFIER_CFLOW_CANNOT_FILL_JUMPS_MAP()
        return {};
    }

    // 3. get method body blocks (exception handlers are not limited to the end of method)
    //    and exception handlers blocks at once
    AddrMap addr_map {method_pc_start_ptr, method_pc_end_ptr};
    addr_map.InvertMarks();

    auto get_offset = [&addr_map](const uint8_t *ptr) { return OffsetAsHexStr(addr_map.AddrStart<void *>(), ptr); };

    bool result =
        ProcessCatchBlocks(method, &addr_map, get_offset, &cflow_info->InstMap_, &cflow_info->ExcHandlers_, cache);
    if (!result) {
        return {};
    }

    LOG(DEBUG, VERIFIER) << "Trace method body code blocks.";
    addr_map.EnumerateMarkedBlocks<const uint8_t *>(
        [&result, &cflow_info, &get_offset](const uint8_t *pc_start_ptr, const uint8_t *pc_end_ptr) {
            auto body = CflowBlockInfo {pc_start_ptr, pc_end_ptr,
                                        JumpsMap {(*cflow_info).InstMap_.AddrStart<const uint8_t *>(),
                                                  (*cflow_info).InstMap_.AddrEnd<const uint8_t *>()},
                                        InstructionType::NORMAL};
            if (FillCflowBlockInfo((*cflow_info).InstMap_, &body) == CflowStatus::ERROR) {
                LOG_VERIFIER_CFLOW_CANNOT_FILL_JUMPS_OF_CODE_BLOCK(get_offset(pc_start_ptr), get_offset(pc_end_ptr));
                return result = false;
            }
            (*cflow_info).BodyBlocks_.push_back(body);
            return true;
        });
    LOG(DEBUG, VERIFIER) << "/Trace method body code blocks.";

    if (!result) {
        return {};
    }

    return cflow_info;
}

}  // namespace panda::verifier
