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

#ifndef PANDA_TYPE_TAGS_H_
#define PANDA_TYPE_TAGS_H_

#include <cstdlib>

#include "verification/util/enum_tag.h"
#include "verification/util/int_tag.h"

#include "runtime/include/mem/panda_containers.h"

namespace panda::verifier {
using TypeNum = size_t;
using VectorNum = PandaVector<TypeNum>;

enum class TypeSystemKind { PANDA, JAVA };
using TypeSystemKindTag = TagForEnum<TypeSystemKind, TypeSystemKind::PANDA, TypeSystemKind::JAVA>;

using ThreadNum = size_t;
constexpr size_t MAX_THREADS = 64;
using ThreadNumTag = TagForInt<ThreadNum, 0ULL, MAX_THREADS - 1ULL>;

enum class TypeVariance { INVARIANT, COVARIANT, CONTRVARIANT };
using TypeVarianceTag =
    TagForEnum<TypeVariance, TypeVariance::INVARIANT, TypeVariance::COVARIANT, TypeVariance::CONTRVARIANT>;
}  // namespace panda::verifier

#endif  // !PANDA_TYPE_TAGS_H_
