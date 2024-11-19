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

#include "../base_classes.h"
#include "../graph.h"
#include "./annotation.h"

#include <functional>
#include <string_view>
#include <vector>

namespace abckit::core {

/**
 * @brief Function
 */
class Function : public View<AbckitCoreFunction *> {
    // We restrict constructors in order to prevent C/C++ API mix-up by user.
    /// @brief to access private constructor
    friend class core::Class;
    /// @brief to access private constructor
    friend class core::Namespace;
    /// @brief to access private constructor
    friend class core::Module;
    /// @brief to access private constructor
    friend class abckit::Instruction;
    /// @brief abckit::DefaultHash<Function>
    friend class abckit::DefaultHash<Function>;

public:
    /**
     * @brief Construct a new Function object
     * @param other
     */
    Function(const Function &other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Function&
     */
    Function &operator=(const Function &other) = default;

    /**
     * @brief Construct a new Function object
     * @param other
     */
    Function(Function &&other) = default;

    /**
     * @brief Constructor
     * @param other
     * @return Function&
     */
    Function &operator=(Function &&other) = default;

    /**
     * @brief Destroy the Function object
     */
    ~Function() override = default;

    /**
     * @brief Get the Graph object
     * @return Graph
     */
    Graph GetGraph() const;

    /**
     * @brief Set the Graph object
     * @param graph
     */
    void SetGraph(const Graph &graph);

    /**
     * @brief Get the name
     * @return std::string_view
     */
    std::string_view GetName() const;

    /**
     * @brief Get the annotation
     * @return std::vector<core::Annotation>
     */
    std::vector<core::Annotation> GetAnnotations() const;

    /**
     * @brief is static
     * @return bool
     */
    bool IsStatic() const;

    // Core API's.
    /**
     * @brief Get the Module object
     * @return core::Module
     */
    core::Module GetModule() const;

    /**
     * @brief Get the Parent Class object
     *
     * @return core::Class
     */
    core::Class GetParentClass() const;

    /**
     * @brief Enumeraterated nested functions
     * @param cb - Callback that will be invoked.
     */
    void EnumerateNestedFunctions(const std::function<bool(core::Function)> &cb) const;

    /**
     * @brief Enumerates nested classes
     * @param cb - Callback that will be invoked.
     */
    void EnumerateNestedClasses(const std::function<bool(core::Class)> &cb) const;

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
