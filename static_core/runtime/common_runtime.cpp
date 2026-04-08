/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "runtime/common_runtime.h"

#include "common_interfaces/base_runtime.h"
#include "runtime/include/runtime.h"

using namespace common_vm;

namespace ark::common_runtime {

using Logger = ark::Logger;

static Level MapLogLevel(Logger::Level level)
{
    switch (level) {
        case Logger::Level::DEBUG:
            return Level::DEBUG;
        case Logger::Level::INFO:
            return Level::INFO;
        case Logger::Level::WARNING:
            return Level::WARN;
        case Logger::Level::ERROR:
            return Level::ERROR;
        case Logger::Level::FATAL:
            return Level::FATAL;
        default:
            return Level::ERROR;
    }
}

static ComponentMark MapLogComponents()
{
    ComponentMark components = 0;
    if (Logger::IsLoggingOn(Logger::GetLevel(), Logger::Component::GC)) {
        components |= static_cast<ComponentMark>(Component::GC);
    }
    if (Logger::IsLoggingOn(Logger::GetLevel(), Logger::Component::MM_OBJECT_EVENTS)) {
        components |= static_cast<ComponentMark>(Component::MM_OBJ_EVENTS);
    }
    components |= static_cast<ComponentMark>(Component::COMMON);
    return components;
}

void InitCommonRuntime()
{
    auto &runtimeOptions = Runtime::GetCurrent()->GetOptions();
    auto *baseRuntime = BaseRuntime::GetInstance();
    ASSERT(baseRuntime != nullptr);
    auto param = BaseRuntime::GetDefaultParam();
    param.logOptions.level = MapLogLevel(Logger::GetLevel());
    param.logOptions.component = MapLogComponents();
    // Single pass compaction should be enabled explicitly
    param.gcParam.singlePassCompactionEnabled =
        runtimeOptions.WasSetG1SinglePassCompactionEnabled() && runtimeOptions.IsG1SinglePassCompactionEnabled();
    baseRuntime->Init(param);
}

void FinishCommonRuntime()
{
    auto *baseRuntime = BaseRuntime::GetInstance();
    ASSERT(baseRuntime != nullptr);
    baseRuntime->Fini();
    BaseRuntime::DestroyInstance();
}

}  // namespace ark::common_runtime
