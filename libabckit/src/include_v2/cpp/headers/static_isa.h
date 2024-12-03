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

#ifndef CPP_ABCKIT_STATIC_ISA_H
#define CPP_ABCKIT_STATIC_ISA_H

#include <memory>

namespace abckit {

class Graph;

class StaticIsa final {
    // To access private constructor.
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    friend class Graph;

public:
    StaticIsa(const StaticIsa &other) = delete;
    StaticIsa &operator=(const StaticIsa &other) = delete;
    StaticIsa(StaticIsa &&other) = delete;
    StaticIsa &operator=(StaticIsa &&other) = delete;
    ~StaticIsa() = default;

private:
    // All static methods impl
    explicit StaticIsa(const Graph &graph) : graph_(graph) {};
    // NOTE(nsizov): remove maybe unused, use graph_
    [[maybe_unused]] const Graph &graph_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_STATIC_ISA_H
