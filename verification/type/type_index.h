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

#ifndef _PANDA_TYPE_INDEX_HPP
#define _PANDA_TYPE_INDEX_HPP

#include "type_tags.h"

#include "verification/util/lazy.h"
#include "verification/util/relation.h"
#include "verification/util/tagged_index.h"

#include "runtime/include/mem/panda_containers.h"

namespace panda::verifier {
class TypeParamIdx : public TaggedIndex<TypeVarianceTag, TypeNum> {
    using Base = TaggedIndex<TypeVarianceTag, TypeNum>;

public:
    TypeParamIdx(TypeVariance variance, TypeNum num)
    {
        Base::SetTag<0>(variance);
        Base::SetInt(num);
    }
    ~TypeParamIdx() = default;
    TypeParamIdx &operator+()
    {
        Base::SetTag<0>(TypeVariance::COVARIANT);
        return *this;
    }
    TypeParamIdx &operator-()
    {
        Base::SetTag<0>(TypeVariance::CONTRVARIANT);
        return *this;
    }
    TypeParamIdx &operator~()
    {
        Base::SetTag<0>(TypeVariance::INVARIANT);
        return *this;
    }
    TypeVariance Variance() const
    {
        return Base::GetTag<0>();
    }
};

using TypeParamsIdx = PandaVector<TypeParamIdx>;
}  // namespace panda::verifier

#endif  // !_PANDA_TYPE_INDEX_HPP
