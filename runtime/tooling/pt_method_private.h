/*
 * Copyright (c) 2019-2022 Huawei Device Co., Ltd.
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

#ifndef RUNTIME_TOOLING_PT_METHOD_PRIVATE_H
#define RUNTIME_TOOLING_PT_METHOD_PRIVATE_H

#include "runtime/include/tooling/debug_interface.h"

// TODO(maksenov): remove this file after refactoring ets_runtime
namespace panda {
class Method;
}  // namespace panda

namespace panda::tooling {
inline PtMethod MethodToPtMethod([[maybe_unused]] Method *method)
{
    return PtMethod();
}
}  // namespace panda::tooling

#endif  // RUNTIME_TOOLING_PT_METHOD_PRIVATE_H
