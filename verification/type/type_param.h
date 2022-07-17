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

#ifndef _PANDA_TYPE_PARAM_HPP__
#define _PANDA_TYPE_PARAM_HPP__

#include "type_index.h"
#include "type_systems.h"
#include "type_type.h"

namespace panda::verifier {
class TypeParams;

class TypeParam : public TypeParamIdx {
public:
    TypeParam() = delete;
    TypeParam(TypeParam &&) = default;
    TypeParam(const TypeParam &) = default;
    TypeParam &operator=(TypeParam &&) = default;
    TypeParam &operator=(const TypeParam &) = default;
    ~TypeParam() = default;

    TypeParams operator>>(const TypeParam &p) const;
    operator TypeParams() const;
    operator Type() const;

private:
    TypeSystemKind kind_;
    ThreadNum threadnum_;
    TypeParam(TypeVariance v, const Type &t);
    TypeParam(TypeSystemKind kind, ThreadNum threadnum, const TypeParamIdx &p);
    friend class Type;
    friend class TypeParams;
};
}  // namespace panda::verifier

#endif  // !_PANDA_TYPE_PARAM_HPP__
