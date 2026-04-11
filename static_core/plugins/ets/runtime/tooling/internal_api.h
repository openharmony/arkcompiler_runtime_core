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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TOOLING_INTERNAL_API_H
#define PANDA_PLUGINS_ETS_RUNTIME_TOOLING_INTERNAL_API_H

#include "runtime/tooling/backtrace/base_defs.h"
#include "runtime/tooling/backtrace/local_stacktrace.h"

extern "C" int ArkParseArkFrameInfoLocal(ark::tooling::LocalTrace localTracePtr, uintptr_t byteCodePc,
                                         uintptr_t mapBase, uintptr_t loadOffset, ark::tooling::Function *function);
extern "C" void ArkCreateLocalStackTrace(ark::tooling::LocalTrace *localTracePtr);
extern "C" void ArkDestroyLocalStackTrace(ark::tooling::LocalTrace localTracePtr);

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_TOOLING_INTERNAL_API_H
