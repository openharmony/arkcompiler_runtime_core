/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef CPP_ABCKIT_CORE_FUNCTION_H
#define CPP_ABCKIT_CORE_FUNCTION_H

#include "libabckit/include/c/abckit.h"
#include "cpp/headers/declarations.h"
#include "cpp/headers/config.h"
#include "cpp/headers/base_classes.h"
#include "cpp/headers/graph.h"
#include "cpp/headers/core/annotation.h"

#include <string_view>
#include <vector>

namespace abckit::core {

class Function : public View<AbckitCoreFunction *> {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    friend class core::Class;
    friend class core::Module;

public:
    Function(const Function &other) = default;
    Function &operator=(const Function &other) = default;
    Function(Function &&other) = default;
    Function &operator=(Function &&other) = default;
    ~Function() override = default;

    Graph GetGraph() const;
    void SetGraph(const Graph &graph);
    std::string_view GetName() const;
    std::vector<core::Annotation> GetAnnotations() const;
    bool IsStatic() const;

    // Core API's.
    // ...

private:
    Function(AbckitCoreFunction *func, const ApiConfig *conf) : View(func), conf_(conf) {};
    const ApiConfig *conf_;

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return conf_;
    }
};

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_FUNCTION_H
