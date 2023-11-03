/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PANDA_PIPELINE_H
#define PANDA_PIPELINE_H

#include "macros.h"
#include <memory>

namespace panda::compiler {
class Graph;

/**
 * Run optimization pipeline.
 * New pipeline is created by inheriting this class and overriding needed methods.
 */
class Pipeline {
public:
    explicit Pipeline(Graph *graph) : graph_(graph) {}
    virtual ~Pipeline() = default;

    NO_COPY_SEMANTIC(Pipeline);
    NO_MOVE_SEMANTIC(Pipeline);

    virtual bool Run();
    virtual bool RunOptimizations();

    Graph *GetGraph()
    {
        return graph_;
    }

    static std::unique_ptr<Pipeline> Create(Graph *graph);

private:
    Graph *graph_ {nullptr};
};

}  // namespace panda::compiler

#endif  // PANDA_PIPELINE_H
