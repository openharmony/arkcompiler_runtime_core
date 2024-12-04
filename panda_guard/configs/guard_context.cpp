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

#include "guard_context.h"

#include <iostream>
#include "guard_args_parser.h"

namespace {
    const std::string TAG = "[Guard_Context]";
}

std::shared_ptr<panda::guard::GuardContext> panda::guard::GuardContext::GetInstance()
{
    static auto instance = std::make_shared<GuardContext>();
    return instance;
}

void panda::guard::GuardContext::Init(int argc, const char **argv)
{
    GuardArgsParser parser;
    if (!parser.Parse(argc, argv)) {
        std::cerr << TAG << "input params parse failed" << std::endl;
        return;
    }

    debugMode_ = parser.IsDebugMode();
}

bool panda::guard::GuardContext::IsDebugMode() const
{
    return debugMode_;
}
