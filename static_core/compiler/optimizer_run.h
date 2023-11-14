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

#ifndef COMPILER_COMPILER_RUN_H_
#define COMPILER_COMPILER_RUN_H_

#include "optimizer/pipeline.h"

namespace panda::compiler {
class Graph;

inline bool RunOptimizations(Graph *graph)
{
    return Pipeline::Create(graph)->Run();
}

}  // namespace panda::compiler
#endif  // COMPILER_COMPILER_RUN_H_
