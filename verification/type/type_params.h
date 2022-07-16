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

#ifndef _PANDA_TYPE_PARAMS_HPP__
#define _PANDA_TYPE_PARAMS_HPP__

#include "type_param.h"

namespace panda::verifier {
class TypeSystem;

class TypeParams : public TypeParamsIdx {
    friend class Type;
    friend class TypeParam;
    friend class ParametricType;

public:
    TypeParams(TypeSystemKind kind, ThreadNum threadnum, const TypeParamsIdx &params = {})
        : TypeParamsIdx {params}, kind_ {kind}, threadnum_ {threadnum}
    {
    }

    TypeParams() = default;
    TypeParams(const TypeParams &) = default;
    TypeParams(TypeParams &&) = default;
    TypeParams &operator=(const TypeParams &) = default;
    TypeParams &operator=(TypeParams &&) = default;
    ~TypeParams() = default;

    bool operator<=(const TypeParams &rhs) const;

    TypeParams &operator>>(const TypeParam &p);

    template <typename Handler>
    void ForEach(Handler &&handler) const
    {
        for (const auto &p : *this) {
            handler(TypeParam {kind_, threadnum_, p});
        }
    }

    TypeSystem &GetTypeSystem() const;

private:
    TypeSystemKind kind_;
    ThreadNum threadnum_;
};
}  // namespace panda::verifier

#endif  // !_PANDA_TYPE_PARAMS_HPP__
