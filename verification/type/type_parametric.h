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

#ifndef _PANDA_TYPE_PARAMETRIC_HPP__
#define _PANDA_TYPE_PARAMETRIC_HPP__

#include "type_params.h"
#include "type_sort.h"
#include "type_tags.h"
#include "type_type.h"

namespace panda::verifier {
class TypeSystem;
class TypeParams;

class ParametricType {
public:
    TypeSystemKind kind_;
    ThreadNum threadnum_;
    SortIdx Sort_;
    ParametricType(TypeSystemKind kind, ThreadNum threadnum, SortIdx sort)
        : kind_ {kind}, threadnum_ {threadnum}, Sort_(sort)
    {
    }
    friend class TypeSystem;

    ParametricType() = delete;
    ParametricType(const ParametricType &) = default;
    ParametricType(ParametricType &&) = default;
    ParametricType &operator=(const ParametricType &) = default;
    ParametricType &operator=(ParametricType &&) = default;
    ~ParametricType() = default;

    TypeSystem &GetTypeSystem() const;
    bool operator[](TypeParamsIdx params) const;
    Type operator()(TypeParamsIdx params = {}) const;
    bool operator[](const TypeParams &params) const;
    Type operator()(const TypeParams &params) const;

    template <typename Handler>
    void ForAll(Handler &&handler) const;
};
}  // namespace panda::verifier

#endif  // !_PANDA_TYPE_PARAMETRIC_HPP__
