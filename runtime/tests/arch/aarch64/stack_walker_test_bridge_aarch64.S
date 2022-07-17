/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "arch/asm_support.h"
#include "arch/aarch64/call_runtime.S"

.global StackWalkerHookAArch64Bridge
.type StackWalkerHookAArch64Bridge, %function
StackWalkerHookAArch64Bridge:
    CFI_STARTPROC
    CFI_DEF_CFA(sp, 0)
    CALL_RUNTIME RUNTIME_MODE_DEFAULT, PandaRunnerHook, 0, 0
    ret
    CFI_ENDPROC
