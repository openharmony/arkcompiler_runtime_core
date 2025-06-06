/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef COMMON_INTERFACES_HEAP_VISITOR_H
#define COMMON_INTERFACES_HEAP_VISITOR_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include "base/common.h"
#include "objects/ref_field.h"

namespace common {
class BaseObject;
using CommonRootVisitor = void (*)(void *root);
using RefFieldVisitor = std::function<void(RefField<> &)>;
using WeakRefFieldVisitor = std::function<bool(RefField<> &)>;

void VisitRoots(const RefFieldVisitor &visitor, bool isMark);
void VisitWeakRoots(const WeakRefFieldVisitor &visitorFunc);

// Static VM Roots scanning
void VisitStaticRoots(const RefFieldVisitor &visitor);
}  // namespace common
#endif  // COMMON_INTERFACES_HEAP_VISITOR_H
