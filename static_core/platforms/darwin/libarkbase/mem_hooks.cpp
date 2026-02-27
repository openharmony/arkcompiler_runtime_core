/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "platforms/darwin/libarkbase/mem_hooks.h"
#include "libarkbase/utils/logger.h"

namespace ark::os::darwin::mem_hooks {
void PandaHooks::Initialize()
{
    LOG(FATAL, RUNTIME) << "PandaHooks::Initialize() not implemented for this platform.";
}
void PandaHooks::Enable()
{
    LOG(FATAL, RUNTIME) << "PandaHooks::Enable() not implemented for this platform.";
}
void PandaHooks::Disable()
{
    LOG(FATAL, RUNTIME) << "PandaHooks::Disable() not implemented for this platform.";
}
}  // namespace ark::os::darwin::mem_hooks