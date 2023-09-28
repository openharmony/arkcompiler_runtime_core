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

#ifndef PANDA_VERIFIER_ABSINT_EXEC_CONTEXT_HPP_
#define PANDA_VERIFIER_ABSINT_EXEC_CONTEXT_HPP_

#include "reg_context.h"

#include "util/addr_map.h"
#include "util/hash.h"

#include "include/mem/panda_containers.h"

namespace panda::verifier {

enum class EntryPointType : size_t { METHOD_BODY, EXCEPTION_HANDLER, LAST = EXCEPTION_HANDLER };

class ExecContext {
public:
    enum class Status { OK, ALL_DONE, NO_ENTRY_POINTS_WITH_CONTEXT };

    bool HasContext(const uint8_t *addr) const
    {
        return reg_context_on_check_point_.count(addr) > 0;
    }

    bool IsCheckPoint(const uint8_t *addr) const
    {
        return check_point_.HasMark(addr);
    }

    void AddEntryPoint(const uint8_t *addr, EntryPointType type)
    {
        entry_point_.insert({addr, type});
    }

    template <typename Reporter>
    void StoreCurrentRegContextForAddr(const uint8_t *addr, Reporter reporter)
    {
        if (HasContext(addr)) {
            StoreCurrentRegContextForAddrIfHasContext(addr, reporter);
        } else if (IsCheckPoint(addr)) {
            reg_context_on_check_point_[addr] = current_reg_context_;
        }
    }

    template <typename Reporter>
    void StoreCurrentRegContextForAddrIfHasContext(const uint8_t *addr, Reporter reporter)
    {
        RegContext &ctx = reg_context_on_check_point_[addr];
        auto lub = RcUnion(&ctx, &current_reg_context_, type_system_);

        if (lub.HasInconsistentRegs()) {
            for (int reg_idx : lub.InconsistentRegsNums()) {
                if (!reporter(reg_idx, current_reg_context_[reg_idx], ctx[reg_idx])) {
                    break;
                }
            }
        }

        ctx.UnionWith(&current_reg_context_, type_system_);

        if (ctx.HasInconsistentRegs()) {
            ctx.RemoveInconsistentRegs();
        }
    }

    void StoreCurrentRegContextForAddr(const uint8_t *addr)
    {
        if (HasContext(addr)) {
            RegContext &ctx = reg_context_on_check_point_[addr];
            ctx.UnionWith(&current_reg_context_, type_system_);
            ctx.RemoveInconsistentRegs();
        } else if (IsCheckPoint(addr)) {
            reg_context_on_check_point_[addr] = current_reg_context_;
        }
    }

    template <typename Reporter>
    void ProcessJump(const uint8_t *jmp_insn_ptr, const uint8_t *target_ptr, Reporter reporter,
                     EntryPointType code_type)
    {
        if (!processed_jumps_.HasMark(jmp_insn_ptr)) {
            processed_jumps_.Mark(jmp_insn_ptr);
            AddEntryPoint(target_ptr, code_type);
            StoreCurrentRegContextForAddr(target_ptr, reporter);
        } else {
            RegContext &target_ctx = reg_context_on_check_point_[target_ptr];
            bool type_updated = target_ctx.UnionWith(&current_reg_context_, type_system_);
            if (type_updated) {
                AddEntryPoint(target_ptr, code_type);
            }
        }
    }

    void ProcessJump(const uint8_t *jmp_insn_ptr, const uint8_t *target_ptr, EntryPointType code_type)
    {
        if (!processed_jumps_.HasMark(jmp_insn_ptr)) {
            processed_jumps_.Mark(jmp_insn_ptr);
            AddEntryPoint(target_ptr, code_type);
            StoreCurrentRegContextForAddr(target_ptr);
        } else {
            RegContext &target_ctx = reg_context_on_check_point_[target_ptr];
            bool type_updated = target_ctx.UnionWith(&current_reg_context_, type_system_);
            if (type_updated) {
                AddEntryPoint(target_ptr, code_type);
            }
        }
    }

    const RegContext &RegContextOnTarget(const uint8_t *addr) const
    {
        auto ctx = reg_context_on_check_point_.find(addr);
        ASSERT(ctx != reg_context_on_check_point_.cend());
        return ctx->second;
    }

    Status GetEntryPointForChecking(const uint8_t **entry, EntryPointType *entry_type)
    {
        for (auto [addr, type] : entry_point_) {
            if (HasContext(addr)) {
                *entry = addr;
                *entry_type = type;
                current_reg_context_ = RegContextOnTarget(addr);
                entry_point_.erase({addr, type});
                return Status::OK;
            }
        }
        if (entry_point_.empty()) {
            return Status::ALL_DONE;
        }
        return Status::NO_ENTRY_POINTS_WITH_CONTEXT;
    }

    RegContext &CurrentRegContext()
    {
        return current_reg_context_;
    }

    const RegContext &CurrentRegContext() const
    {
        return current_reg_context_;
    }

    void SetCheckPoint(const uint8_t *addr)
    {
        check_point_.Mark(addr);
    }

    template <typename Fetcher>
    void SetCheckPoints(Fetcher fetcher)
    {
        while (auto tgt = fetcher()) {
            SetCheckPoint(*tgt);
        }
    }

    template <typename Handler>
    void ForContextsOnCheckPointsInRange(const uint8_t *from, const uint8_t *to, Handler handler)
    {
        check_point_.EnumerateMarksInScope<const uint8_t *>(from, to, [&handler, this](const uint8_t *ptr) {
            if (HasContext(ptr)) {
                return handler(ptr, reg_context_on_check_point_[ptr]);
            }
            return true;
        });
    }

    ExecContext(const uint8_t *pc_start_ptr, const uint8_t *pc_end_ptr, TypeSystem *type_system)
        : check_point_ {pc_start_ptr, pc_end_ptr},
          processed_jumps_ {pc_start_ptr, pc_end_ptr},
          type_system_ {type_system}
    {
    }

    DEFAULT_MOVE_SEMANTIC(ExecContext);
    DEFAULT_COPY_SEMANTIC(ExecContext);
    ~ExecContext() = default;

private:
    AddrMap check_point_;
    AddrMap processed_jumps_;
    // Use an ordered set to make iteration over elements reproducible.
    PandaSet<std::pair<const uint8_t *, EntryPointType>> entry_point_;
    PandaUnorderedMap<const uint8_t *, RegContext> reg_context_on_check_point_;
    TypeSystem *type_system_;
    RegContext current_reg_context_;
};
}  // namespace panda::verifier

#endif  // !PANDA_VERIFIER_ABSINT_EXEC_CONTEXT_HPP_
