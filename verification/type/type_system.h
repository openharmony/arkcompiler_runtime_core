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

#ifndef _PANDA_TYPE_SYSTEM_HPP
#define _PANDA_TYPE_SYSTEM_HPP

#include "verification/util/lazy.h"
#include "verification/util/relation.h"

#include "subtyping_closure.h"
#include "type_sort.h"
#include "type_index.h"
#include "type_info.h"
#include "type_param.h"
#include "type_params.h"
#include "type_parametric.h"
#include "type_set.h"
#include "type_systems.h"
#include "type_tags.h"
#include "type_type.h"

#include "runtime/include/mem/panda_containers.h"

#include "libpandabase/os/mutex.h"

#include "macros.h"

#include <memory>
#include <variant>
#include <functional>
#include <algorithm>

// TODO(vdyadov): refactor this awful code into more accurate pieces

/*
todo:
3. lazy computation of parameters subtyping
4. unify TypeParam/TypeParams with TypeParamIdx/TypeParamsIdx
6. TypeParamsIdx - separate Variance from Indices
7. Check parameters variance of types from type family (same sort + same arity),
   upon new type addition. Or separate parameters variance from parameters, make
   it independent entity, which is defined for type family prior any of type addition

Q: mutualy recursive types?
*/

namespace panda::verifier {
/*
Design decisions:
2. Subtyping relation is kept flat during types construction
3. Subtyping relation closing may be either incremental/implicit during types construction or explicit.
4. Sorts are abstracted in the form of indices (of type size_t)
5. Types internally represented as indices (size_t)
6. There are special initial and final types, named as Bot and Top, and all types are implicitly related
   as Bot <: type <: Top
*/

class TypeSystem {
public:
    TypeSystem(SortIdx bot, SortIdx top, ThreadNum threadnum = 0, TypeSystemKind kind = TypeSystemKind::PANDA)
        : kind_ {kind},
          threadnum_ {threadnum},
          BotNum_ {FindNumOrCreate({bot, {}})},
          TopNum_ {FindNumOrCreate({top, {}})}
    {
    }

    NO_COPY_SEMANTIC(TypeSystem);
    DEFAULT_MOVE_SEMANTIC(TypeSystem);
    ~TypeSystem() = default;

    using TypeUniverse = PandaVector<TypeInfo>;
    using MappingToNum = PandaUnorderedMap<TypeInfo, TypeNum>;
    // sort -> arity -> types
    using TypeClasses = PandaVector<PandaUnorderedMap<size_t, VectorNum>>;

    Relation TypingRel_ = {};
    PandaVector<PandaUnorderedSet<TypeNum>> ParameterOf_ = {};
    TypeUniverse Universe_ = {};
    MappingToNum InfoToNum_ = {};
    mutable TypeClasses TypeClasses_ = {};
    SubtypingClosureInfo SubtypingClosureCurrent_;
    SubtypingClosureInfo SubtypingClosureNext_;
    bool IncrementalSubtypingClosure_ = true;
    bool DeferIncrementalSubtypingClosure_ = false;

    TypeSystemKind kind_ = TypeSystemKind::PANDA;
    ThreadNum threadnum_ = 0;

    Index<TypeNum> FindNum(const TypeInfo &ti) const
    {
        auto it = InfoToNum_.find(ti);
        if (it != InfoToNum_.end()) {
            return it->second;
        }
        return {};
    }

    TypeNum FindNumOrCreate(const TypeInfo &ti)
    {
        Index<TypeNum> existingnum = FindNum(ti);
        if (existingnum.IsValid()) {
            return existingnum;
        }
        size_t num = Universe_.size();
        TypingRel_.EnsureMinSize(num);
        Universe_.push_back(ti);
        ParameterOf_.push_back({});
        const auto &params = ti.ParamsIdx();
        for (const auto &param : params) {
            ParameterOf_[param].insert(num);
        }
        InfoToNum_[ti] = num;
        SortIdx sort = ti.Sort();
        size_t arity = params.size();
        if (sort >= TypeClasses_.size()) {
            TypeClasses_.resize(sort + 1);
        }
        TypeClasses_[sort][arity].push_back(num);
        Relate(num, num);
        return num;
    }

    const VectorNum &TypeClassNum(TypeNum type) const
    {
        const auto &info = Universe_[type];
        const auto &params = info.ParamsIdx();
        return TypeClasses_[info.Sort()][params.size()];
    }

    void PerformClosingCurrentRelation()
    {
        auto addToNext = [this](TypeNum type) {
            const auto &info = Universe_[type];
            SubtypingClosureNext_.AddType(info.Sort(), type, info.Arity());
            return true;
        };
        while (!SubtypingClosureCurrent_.Empty()) {
            SubtypingClosureCurrent_.ForAllTypeClasses([this, &addToNext](auto &types) {
                for (auto type_lhs : types) {
                    for (auto type_rhs : types) {
                        bool in_direct_relation = TypingRel_.IsInDirectRelation(type_lhs, type_rhs);
                        if (!in_direct_relation && CheckIfLhsSubtypeOfRhs(type_lhs, type_rhs)) {
                            addToNext(type_lhs);
                            addToNext(type_rhs);
                            TypingRel_ += {type_lhs, type_rhs};
                            for (const auto &type : ParameterOf_[type_lhs]) {
                                addToNext(type);
                                TypingRel_.ForAllTo(type, addToNext);
                                TypingRel_.ForAllFrom(type, addToNext);
                            }
                            for (const auto &type : ParameterOf_[type_rhs]) {
                                addToNext(type);
                                TypingRel_.ForAllTo(type, addToNext);
                                TypingRel_.ForAllFrom(type, addToNext);
                            }
                        }
                    }
                }
            });
            SubtypingClosureCurrent_.swap(SubtypingClosureNext_);
            SubtypingClosureNext_.Clear();
        }
    }

    void Relate(TypeNum lhs, TypeNum rhs)
    {
        if (TypingRel_.IsInDirectRelation(lhs, rhs)) {
            return;
        }
        TypingRel_ += {lhs, rhs};
        if (IncrementalSubtypingClosure_) {
            auto processType = [this](TypeNum type) {
                auto addToCurrent = [this](TypeNum type1) {
                    const auto &info = Universe_[type1];
                    SubtypingClosureCurrent_.AddType(info.Sort(), type1, info.Arity());
                    return true;
                };
                for (const auto &typenum : TypeClassNum(type)) {
                    addToCurrent(typenum);
                }
                for (const auto &type_idx : ParameterOf_[type]) {
                    addToCurrent(type_idx);
                    TypingRel_.ForAllTo(type_idx, addToCurrent);
                    TypingRel_.ForAllFrom(type_idx, addToCurrent);
                }
            };
            processType(lhs);
            if (lhs != rhs) {
                processType(rhs);
            }
            if (!DeferIncrementalSubtypingClosure_) {
                PerformClosingCurrentRelation();
            }
        }
    }

    bool CheckIfLhsParamsSubtypeOfRhs(const TypeParamsIdx &lhs, const TypeParamsIdx &rhs) const
    {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        auto lhs_it = lhs.cbegin();
        auto rhs_it = rhs.cbegin();
        for (; lhs_it != lhs.cend(); ++lhs_it, ++rhs_it) {
            switch (lhs_it->Variance()) {
                case TypeVariance::INVARIANT:
                    if (!TypingRel_.IsInIsoRelation(*lhs_it, *rhs_it)) {
                        return false;
                    }
                    break;
                case TypeVariance::COVARIANT:
                    if (!TypingRel_.IsInDirectRelation(*lhs_it, *rhs_it)) {
                        return false;
                    }
                    break;
                case TypeVariance::CONTRVARIANT:
                    if (!TypingRel_.IsInInverseRelation(*lhs_it, *rhs_it)) {
                        return false;
                    }
                    break;
                default:
                    break;
            }
        }
        return true;
    }

    bool CheckIfLhsSubtypeOfRhs(TypeNum lhs, TypeNum rhs) const
    {
        const TypeInfo &lhsInfo = Universe_[lhs];
        const TypeInfo &rhsInfo = Universe_[rhs];
        if (lhsInfo.Sort() != rhsInfo.Sort()) {
            return false;
        }
        const TypeParamsIdx &lhsParams = lhsInfo.ParamsIdx();
        const TypeParamsIdx &rhsParams = rhsInfo.ParamsIdx();
        return CheckIfLhsParamsSubtypeOfRhs(lhsParams, rhsParams);
    }

    bool IsInDirectRelation(TypeNum lhs, TypeNum rhs) const
    {
        return TypingRel_.IsInDirectRelation(lhs, rhs);
    }

    size_t GetSort(TypeNum t) const
    {
        return Universe_[t].Sort();
    }

    size_t GetArity(TypeNum t) const
    {
        return Universe_[t].Arity();
    }

    const TypeParamsIdx &GetParamsIdx(TypeNum t) const
    {
        return Universe_[t].ParamsIdx();
    }

    friend class Type;
    friend class TypeParams;
    friend class ParametricType;

    void SetIncrementalRelationClosureMode(bool state)
    {
        IncrementalSubtypingClosure_ = state;
    }

    void SetDeferIncrementalRelationClosure(bool state)
    {
        DeferIncrementalSubtypingClosure_ = state;
    }

    template <typename Handler>
    void ForAllTypes(Handler &&handler) const
    {
        for (size_t num = 0; num < Universe_.size(); ++num) {
            if (!handler(Type {kind_, threadnum_, num})) {
                return;
            }
        }
    }

    template <typename Handler>
    void ForAllSubtypesOf(const Type &t, Handler &&handler) const
    {
        auto num = t.Number();
        auto callback = [this, &handler](const TypeNum &t_num) {
            bool result = handler(Type {kind_, threadnum_, t_num});
            return result;
        };
        TypingRel_.ForAllTo(num, callback);
    }

    template <typename Handler>
    void ForAllSupertypesOf(const Type &t, Handler &&handler) const
    {
        auto num = t.Number();
        auto callback = [this, &handler](const TypeNum &t_num) {
            bool result = handler(Type {kind_, threadnum_, t_num});
            return result;
        };
        TypingRel_.ForAllFrom(num, callback);
    }

    const IntSet<TypeNum> &GetDirectlyRelated(TypeNum from) const
    {
        return TypingRel_.GetDirectlyRelated(from);
    }

    const IntSet<TypeNum> &GetInverselyRelated(TypeNum to) const
    {
        return TypingRel_.GetInverselyRelated(to);
    }

    void CloseSubtypingRelation()
    {
        ForAllTypes([this](const Type &type) {
            auto sort = type.Sort();
            auto number = type.Number();
            auto arity = type.Arity();
            SubtypingClosureCurrent_.AddType(sort, number, arity);
            return true;
        });
        PerformClosingCurrentRelation();
    }

    void CloseAccumulatedSubtypingRelation()
    {
        if (IncrementalSubtypingClosure_) {
            if (DeferIncrementalSubtypingClosure_) {
                PerformClosingCurrentRelation();
            }
        } else {
            CloseSubtypingRelation();
        }
    }

    ParametricType Parametric(SortIdx sort)
    {
        return {kind_, threadnum_, sort};
    }

    Type Bot() const
    {
        return {kind_, threadnum_, BotNum_};
    }

    Type Top() const
    {
        return {kind_, threadnum_, TopNum_};
    }

    TypeSystemKind GetKind() const
    {
        return kind_;
    }

    ThreadNum GetThreadNum() const
    {
        return threadnum_;
    }

private:
    TypeNum BotNum_;
    TypeNum TopNum_;
};
}  // namespace panda::verifier

#endif  // !_PANDA_TYPE_SYSTEM_HPP
