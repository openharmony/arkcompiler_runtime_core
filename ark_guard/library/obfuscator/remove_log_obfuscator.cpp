/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "remove_log_obfuscator.h"

#include <string_view>

#include "abckit_wrapper/method.h"
#include "logger.h"

namespace {
// Lightweight log detection: avoid regex, use simple string checks
bool IsCallLog(const abckit::Instruction &instruction)
{
    if (instruction.GetId() == 0 || !instruction.CheckIsCall()) {
        return false;
    }

    const auto &functionName = instruction.GetFunction().GetName();
    const std::string_view name(functionName);

    // "^.*:std.core.Console;.*" -> contains ":std.core.Console;"
    if (name.find(":std.core.Console;") != std::string_view::npos) {
        return true;
    }
    // "^<level>:.*;std.core.Array;void;" -> starts with "<level>:" and contains ";std.core.Array;void;"
    constexpr std::string_view ARRAY_VOID_SUFFIX = ";std.core.Array;void;";
    // 5 is length of "debug:" or "info:" etc.
    if (name.find(ARRAY_VOID_SUFFIX) != std::string_view::npos && name.size() >= 5) {
        if (name.substr(0, 6) == "debug:" ||  // 6 is length of "debug:" etc.
            name.substr(0, 5) == "info:" ||   // 5 is length of "info:" etc.
            name.substr(0, 5) == "warn:" ||   // 5 is length of "warn:" etc.
            name.substr(0, 6) == "error:" ||  // 6 is length of "error:" etc.
            name.substr(0, 6) == "fatal:") {  // 6 is length of "fatal:" etc.
            return true;
        }
    }
    return false;
}

bool RemoveConsoleLog(const abckit_wrapper::Method *method)
{
    const auto &function = method->GetArkTsImpl<abckit::core::Function>();
    const auto &arkTsFunction = method->GetArkTsImpl<abckit::arkts::Function>();
    if (function.IsExternal() || arkTsFunction.IsAbstract() || arkTsFunction.IsNative()) {
        return true;
    }
    auto graph = function.CreateGraph();
    bool isRemove = false;
    for (const auto &block : graph.GetBlocksRPO()) {
        for (auto &ins : block.GetInstructions()) {
            if (IsCallLog(ins)) {
                ins.Remove();
                isRemove = true;
            }
        }
    }
    if (isRemove) {
        LOG_I << "Remove console log for method: " << method->GetFullyQualifiedName();
        function.SetGraph(graph);
    }
    return true;
}
}  // namespace

bool ark::guard::RemoveLogObfuscator::Execute(abckit_wrapper::FileView &fileView)
{
    return fileView.ModulesAccept(this->Wrap<abckit_wrapper::LocalModuleFilter>());
}

bool ark::guard::RemoveLogObfuscator::VisitMethod(abckit_wrapper::Method *method)
{
    return RemoveConsoleLog(method);
}

bool ark::guard::RemoveLogObfuscator::Visit(abckit_wrapper::Module *module)
{
    return module->ChildrenAccept(*this);
}

bool ark::guard::RemoveLogObfuscator::VisitNamespace(abckit_wrapper::Namespace *ns)
{
    return ns->ChildrenAccept(*this);
}

bool ark::guard::RemoveLogObfuscator::VisitClass(abckit_wrapper::Class *clazz)
{
    return clazz->ChildrenAccept(*this);
}

bool ark::guard::RemoveLogObfuscator::VisitAnnotationInterface(abckit_wrapper::AnnotationInterface *ai)
{
    return true;
}

bool ark::guard::RemoveLogObfuscator::VisitField(abckit_wrapper::Field *field)
{
    return true;
}
