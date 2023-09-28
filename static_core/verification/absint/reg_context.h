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

#ifndef PANDA_VERIFIER_ABSINT_REG_CONTEXT_HPP_
#define PANDA_VERIFIER_ABSINT_REG_CONTEXT_HPP_

#include "value/abstract_typed_value.h"

#include "util/index.h"
#include "util/lazy.h"
#include "util/str.h"
#include "util/shifted_vector.h"

#include "macros.h"

#include <algorithm>

namespace panda::verifier {
/*
Design decisions:
1. regs - unordered map, for speed (compared to map) and space efficiency (compared to vector)
   after implementing sparse vectors - rebase on them (taking into consideration immutability, see immer)
*/

// TODO(vdyadov): correct handling of values origins during LUB operation

class RegContext {
public:
    explicit RegContext() = default;
    explicit RegContext(size_t size) : regs_(size) {}
    ~RegContext() = default;

    DEFAULT_MOVE_SEMANTIC(RegContext);
    DEFAULT_COPY_SEMANTIC(RegContext);

    bool UnionWith(RegContext const *rhs, TypeSystem *tsys)
    {
        bool updated = false;
        auto lhs_it = regs_.begin();
        auto rhs_it = rhs->regs_.begin();
        while (lhs_it != regs_.end() && rhs_it != rhs->regs_.end()) {
            if (!(*lhs_it).IsNone() && !(*rhs_it).IsNone()) {
                auto before = *lhs_it;
                auto result = AtvJoin(&*lhs_it, &*rhs_it, tsys);
                updated |= (result.GetAbstractType() != before.GetAbstractType());
                *lhs_it = result;
            } else {
                updated |= !(*lhs_it).IsNone();
                *lhs_it = AbstractTypedValue {};
            }
            ++lhs_it;
            ++rhs_it;
        }
        while (lhs_it != regs_.end()) {
            updated = true;
            *lhs_it = AbstractTypedValue {};
            ++lhs_it;
        }
        return updated;
    }
    void ChangeValuesOfSameOrigin(int idx, const AbstractTypedValue &atv)
    {
        if (!regs_.InValidRange(idx)) {
            regs_[idx] = atv;
            return;
        }
        auto old_atv = regs_[idx];
        if (old_atv.IsNone()) {
            regs_[idx] = atv;
            return;
        }
        const auto &old_origin = old_atv.GetOrigin();
        if (!old_origin.IsValid()) {
            regs_[idx] = atv;
            return;
        }
        auto it = regs_.begin();
        while (it != regs_.end()) {
            if (!(*it).IsNone()) {
                const auto &origin = (*it).GetOrigin();
                if (origin.IsValid() && origin == old_origin) {
                    *it = atv;
                }
            }
            ++it;
        }
    }
    AbstractTypedValue &operator[](int idx)
    {
        if (!regs_.InValidRange(idx)) {
            regs_.ExtendToInclude(idx);
        }
        return regs_[idx];
    }
    AbstractTypedValue operator[](int idx) const
    {
        ASSERT(IsRegDefined(idx) && regs_.InValidRange(idx));
        return regs_[idx];
    }
    size_t Size() const
    {
        size_t size = 0;
        EnumerateAllRegs([&size](auto, auto) {
            ++size;
            return true;
        });
        return size;
    }
    template <typename Callback>
    void EnumerateAllRegs(Callback cb) const
    {
        for (int idx = regs_.BeginIndex(); idx < regs_.EndIndex(); ++idx) {
            if (!regs_[idx].IsNone()) {
                const auto &atv = regs_[idx];
                if (!cb(idx, atv)) {
                    return;
                }
            }
        }
    }
    template <typename Callback>
    void EnumerateAllRegs(Callback cb)
    {
        for (int idx = regs_.BeginIndex(); idx < regs_.EndIndex(); ++idx) {
            if (!regs_[idx].IsNone()) {
                auto &atv = regs_[idx];
                if (!cb(idx, atv)) {
                    return;
                }
            }
        }
    }
    bool HasInconsistentRegs() const
    {
        bool result = false;

        EnumerateAllRegs([&](int, const auto &av) {
            if (!av.IsConsistent()) {
                result = true;
                return false;
            }
            return true;
        });
        return result;
    }
    auto InconsistentRegsNums() const
    {
        PandaVector<int> result;
        EnumerateAllRegs([&](int num, const auto &av) {
            if (!av.IsConsistent()) {
                result.push_back(num);
            }
            return true;
        });
        return result;
    }
    bool IsRegDefined(int num) const
    {
        return regs_.InValidRange(num) && !regs_[num].IsNone();
    }
    bool IsValid(int num) const
    {
        return regs_.InValidRange(num);
    }
    bool WasConflictOnReg(int num) const
    {
        return conflicting_regs_.count(num) > 0;
    }
    void Clear()
    {
        regs_.clear();
        conflicting_regs_.clear();
    }
    void RemoveInconsistentRegs()
    {
        EnumerateAllRegs([this](int num, auto &atv) {
            if (!atv.IsConsistent()) {
                conflicting_regs_.insert(num);
                atv = AbstractTypedValue {};
            } else {
                conflicting_regs_.erase(num);
            }
            return true;
        });
    }
    PandaString DumpRegs(TypeSystem const *tsys) const
    {
        PandaString log_string {};
        bool comma = false;
        EnumerateAllRegs([&comma, &log_string, &tsys](int num, const auto &abs_type_val) {
            PandaString result {num == -1 ? "acc" : "v" + NumToStr(num)};
            result += " : ";
            result += abs_type_val.ToString(tsys);
            if (comma) {
                log_string += ", ";
            }
            log_string += result;
            comma = true;
            return true;
        });
        return log_string;
    }

private:
    ShiftedVector<1, AbstractTypedValue> regs_;

    // TODO(vdyadov): After introducing sparse bit-vectors, change ConflictingRegs_ type.
    PandaUnorderedSet<int> conflicting_regs_;

    friend RegContext RcUnion(RegContext const *lhs, RegContext const *rhs, TypeSystem * /* tsys */);
};

inline RegContext RcUnion(RegContext const *lhs, RegContext const *rhs, TypeSystem *tsys)
{
    RegContext result(std::max(lhs->regs_.size(), rhs->regs_.size()));
    auto result_it = result.regs_.begin();
    auto lhs_it = lhs->regs_.begin();
    auto rhs_it = rhs->regs_.begin();
    while (lhs_it != lhs->regs_.end() && rhs_it != rhs->regs_.end()) {
        if (!(*lhs_it).IsNone() && !(*rhs_it).IsNone()) {
            *result_it = AtvJoin(&*lhs_it, &*rhs_it, tsys);
        }
        ++lhs_it;
        ++rhs_it;
        ++result_it;
    }
    return result;
}

}  // namespace panda::verifier

#endif  // !PANDA_VERIFIER_ABSINT_REG_CONTEXT_HPP_
