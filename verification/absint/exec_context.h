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

enum class EntryPointType : size_t { METHOD_BODY, EXCEPTION_HANDLER, __LAST__ = EXCEPTION_HANDLER };

class ExecContext {
public:
    enum class Status { OK, ALL_DONE, NO_ENTRY_POINTS_WITH_CONTEXT };

    bool HasContext(const uint8_t *addr) const
    {
        return RegContextOnCheckPoint_.count(addr) > 0;
    }

    bool IsCheckPoint(const uint8_t *addr) const
    {
        return CheckPoint_.HasMark(addr);
    }

    void AddEntryPoint(const uint8_t *addr, EntryPointType type)
    {
        EntryPoint_.insert({addr, type});
    }

    template <typename Reporter>
    void StoreCurrentRegContextForAddr(const uint8_t *addr, Reporter reporter)
    {
        if (HasContext(addr)) {
            StoreCurrentRegContextForAddrIfHasContext(addr, reporter);
        } else if (IsCheckPoint(addr)) {
            RegContextOnCheckPoint_[addr] = CurrentRegContext_;
        }
    }

    template <typename Reporter>
    void StoreCurrentRegContextForAddrIfHasContext(const uint8_t *addr, Reporter reporter)
    {
        RegContext &ctx = RegContextOnCheckPoint_[addr];
        auto lub = ctx & CurrentRegContext_;

        if (lub.HasInconsistentRegs()) {
            for (int reg_idx : lub.InconsistentRegsNums()) {
                if (!reporter(reg_idx, CurrentRegContext_[reg_idx], ctx[reg_idx])) {
                    break;
                }
            }
        }

        ctx &= CurrentRegContext_;

        if (ctx.HasInconsistentRegs()) {
            ctx.RemoveInconsistentRegs();
        }
    }

    void StoreCurrentRegContextForAddr(const uint8_t *addr)
    {
        if (HasContext(addr)) {
            RegContext &ctx = RegContextOnCheckPoint_[addr];
            ctx &= CurrentRegContext_;
            ctx.RemoveInconsistentRegs();
        } else if (IsCheckPoint(addr)) {
            RegContextOnCheckPoint_[addr] = CurrentRegContext_;
        }
    }

    template <typename Reporter>
    void ProcessJump(const uint8_t *jmpInsnPtr, const uint8_t *targetPtr, Reporter reporter, EntryPointType codeType)
    {
        if (!ProcessedJumps_.HasMark(jmpInsnPtr)) {
            ProcessedJumps_.Mark(jmpInsnPtr);
            AddEntryPoint(targetPtr, codeType);
            StoreCurrentRegContextForAddr(targetPtr, reporter);
        }
    }

    void ProcessJump(const uint8_t *jmpInsnPtr, const uint8_t *targetPtr, EntryPointType codeType)
    {
        if (!ProcessedJumps_.HasMark(jmpInsnPtr)) {
            ProcessedJumps_.Mark(jmpInsnPtr);
            AddEntryPoint(targetPtr, codeType);
            StoreCurrentRegContextForAddr(targetPtr);
        }
    }

    const RegContext &RegContextOnTarget(const uint8_t *addr) const
    {
        auto ctx = RegContextOnCheckPoint_.find(addr);
        ASSERT(ctx != RegContextOnCheckPoint_.cend());
        return ctx->second;
    }

    Status GetEntryPointForChecking(const uint8_t **entry, EntryPointType *entryType)
    {
        for (auto [addr, type] : EntryPoint_) {
            if (HasContext(addr)) {
                *entry = addr;
                *entryType = type;
                CurrentRegContext_ = RegContextOnTarget(addr);
                EntryPoint_.erase({addr, type});
                return Status::OK;
            }
        }
        if (EntryPoint_.empty()) {
            return Status::ALL_DONE;
        }
        return Status::NO_ENTRY_POINTS_WITH_CONTEXT;
    }

    RegContext &CurrentRegContext()
    {
        return CurrentRegContext_;
    }

    const RegContext &CurrentRegContext() const
    {
        return CurrentRegContext_;
    }

    void SetCheckPoint(const uint8_t *addr)
    {
        CheckPoint_.Mark(addr);
    }

    void SetTypecastPoint(const uint8_t *addr)
    {
        CheckPoint_.Mark(addr);
        TypecastPoint_.Mark(addr);
    }

    bool IsTypecastPoint(const uint8_t *addr) const
    {
        return TypecastPoint_.HasMark(addr);
    }

    template <typename Handler>
    void ForAllTypesOfRegAccordingToTypecasts(int reg, const RegContext &ctx, Handler &&handler)
    {
        if (ctx.IsRegDefined(reg)) {
            const auto &atv = ctx[reg];
            if (!handler(atv)) {
                return;
            }
            const auto &origin = atv.GetOrigin();
            if (origin.IsValid() && !origin.AtStart()) {
                uint32_t offset = origin.GetOffset();
                const uint8_t *ptr = &(TypecastPoint_.AddrStart<const uint8_t *>()[offset]);  // NOLINT
                if (IsTypecastPoint(ptr)) {
                    ForAllTypesOfRegAccordingToTypecasts(reg, RegContextOnTarget(ptr), std::move(handler));
                }
            }
        }
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
        CheckPoint_.EnumerateMarksInScope<const uint8_t *>(from, to, [&handler, this](const uint8_t *ptr) {
            if (HasContext(ptr)) {
                return handler(ptr, RegContextOnCheckPoint_[ptr]);
            }
            return true;
        });
    }

    ExecContext(const uint8_t *pcStartPtr, const uint8_t *pcEndPtr)
        : CheckPoint_ {pcStartPtr, pcEndPtr},
          ProcessedJumps_ {pcStartPtr, pcEndPtr},
          TypecastPoint_ {pcStartPtr, pcEndPtr}
    {
    }

    DEFAULT_MOVE_SEMANTIC(ExecContext);
    ExecContext(const ExecContext &) = default;
    ~ExecContext() = default;

private:
    AddrMap CheckPoint_;
    AddrMap ProcessedJumps_;
    AddrMap TypecastPoint_;
    PandaUnorderedSet<std::pair<const uint8_t *, EntryPointType>> EntryPoint_;
    PandaUnorderedMap<const uint8_t *, RegContext> RegContextOnCheckPoint_;
    RegContext CurrentRegContext_;
};
}  // namespace panda::verifier

#endif  // !PANDA_VERIFIER_ABSINT_EXEC_CONTEXT_HPP_
