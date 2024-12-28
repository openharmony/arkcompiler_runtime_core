/**
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

#include "program.h"

#include "utils/logger.h"

#include "configs/guard_context.h"
#include "graph_analyzer.h"

namespace {
constexpr std::string_view TAG = "[Program]";
}

void panda::guard::Program::Create()
{
    LOG(INFO, PANDAGUARD) << TAG << "===== program create start =====";

    for (const auto &[name, record] : this->prog_->record_table) {
        if (Node::IsJsonFile(record)) {
            Node node(this, name, "");
            node.Create();
            this->nodeTable_.emplace(name, node);
        }

        std::string pkgName;
        if (Node::FindPkgName(record, pkgName)) {
            if (GuardContext::GetInstance()->GetGuardOptions()->IsSkippedRemoteHar(pkgName)) {
                LOG(INFO, PANDAGUARD) << TAG << "skip record: " << record.name;
                continue;
            }

            Node node(this, name, pkgName);
            node.Create();
            this->nodeTable_.emplace(name, node);
        }
    }

    LOG(INFO, PANDAGUARD) << TAG << "===== program create end =====";
}

void panda::guard::Program::ForEachFunction(const std::function<FunctionTraver> &callback)
{
    for (auto &[_, node] : this->nodeTable_) {
        node.ForEachFunction(callback);
    }
}

void panda::guard::Program::RemoveConsoleLog()
{
    if (!GuardContext::GetInstance()->GetGuardOptions()->IsRemoveLogObfEnabled()) {
        return;
    }

    this->ForEachFunction([](Function &function) { function.RemoveConsoleLog(); });
}

void panda::guard::Program::Obfuscate()
{
    LOG(INFO, PANDAGUARD) << TAG << "===== program obfuscate start =====";

    for (auto &[name, node] : this->nodeTable_) {
        node.Obfuscate();
        node.UpdateScopeNames();
    }

    this->UpdateReference();

    this->RemoveConsoleLog();

    LOG(INFO, PANDAGUARD) << TAG << "===== program obfuscate end =====";
}

void panda::guard::Program::UpdateReference()
{
    this->ForEachFunction([](Function &function) -> void { function.UpdateReference(); });
    for (auto &[_, node] : this->nodeTable_) {
        node.UpdateFileNameReferences();
    }
}
