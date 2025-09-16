/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ark_guard/ark_guard.h"

#include "static_core/libpandabase/utils/logger.h"

#include "abckit_wrapper/file_view.h"

namespace {
void InitLogger()
{
    ark::Logger::ComponentMask component_mask;
    component_mask.set(ark::Logger::Component::ARK_GUARD);
    component_mask.set(ark::Logger::Component::ABCKIT_WRAPPER);
    ark::Logger::InitializeStdLogging(ark::Logger::Level::DEBUG, component_mask);
}
}  // namespace

void ark::guard::ArkGuard::Run(int argc, const char **argv)
{
    InitLogger();
    LOG(INFO, ARK_GUARD) << "argc:" << argc;
    LOG(INFO, ABCKIT_WRAPPER) << "argv 0:" << argv[0];

    abckit_wrapper::FileView file;
    file.Init(argv[1]);
}
