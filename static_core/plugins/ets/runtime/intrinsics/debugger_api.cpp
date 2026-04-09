/**
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_execution_context.h"
#include "include/tooling/debug_interface.h"
#include "include/tooling/vreg_value.h"
#include "intrinsics.h"
#include "libarkbase/utils/logger.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/tooling/helpers.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "runtime/include/tooling/debug_interface.h"
#include "runtime/include/tooling/vreg_value.h"

namespace ark::ets::intrinsics {

static void SetRuntimeException(EtsLong regNumber, EtsExecutionContext *executionCtx, std::string_view typeName)
{
    auto errorMsg =
        "Failed to access variable at vreg #" + std::to_string(regNumber) + " and type " + std::string(typeName);
    ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreError, errorMsg);
}

template <typename T>
static T DebuggerAPIGetLocal(EtsLong regNumber)
{
    static constexpr uint32_t PREVIOUS_FRAME_DEPTH = 1;

    auto *executionCtx = EtsExecutionContext::GetCurrent();
    auto *runtime = executionCtx->GetPandaVM()->GetRuntime();
    if (UNLIKELY(!runtime->IsDebugMode())) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreError, "Debugger is not enabled");
        return static_cast<T>(0);
    }
    if (UNLIKELY(regNumber < 0)) {
        SetRuntimeException(regNumber, executionCtx, ark::ets::tooling::EtsTypeName<T>::NAME);
        return static_cast<T>(0);
    }

    ark::tooling::VRegValue vregValue;
    auto &debugger = runtime->StartDebugSession()->GetDebugger();
    auto err = debugger.GetVariable(ark::ets::tooling::CoroutineToPtThread(executionCtx->GetMT()), PREVIOUS_FRAME_DEPTH,
                                    regNumber, &vregValue);
    if (err) {
        LOG(ERROR, DEBUGGER) << "Failed to get local variable: " << err.value().GetMessage();
        SetRuntimeException(regNumber, executionCtx, ark::ets::tooling::EtsTypeName<T>::NAME);
        return static_cast<T>(0);
    }
    return ark::ets::tooling::VRegValueToEtsValue<T>(vregValue);
}

extern "C" EtsBoolean DebuggerAPIGetLocalBoolean(EtsLong regNumber)
{
    return DebuggerAPIGetLocal<EtsBoolean>(regNumber);
}

extern "C" EtsByte DebuggerAPIGetLocalByte(EtsLong regNumber)
{
    return DebuggerAPIGetLocal<EtsByte>(regNumber);
}

extern "C" EtsShort DebuggerAPIGetLocalShort(EtsLong regNumber)
{
    return DebuggerAPIGetLocal<EtsShort>(regNumber);
}

extern "C" EtsChar DebuggerAPIGetLocalChar(EtsLong regNumber)
{
    return DebuggerAPIGetLocal<EtsChar>(regNumber);
}

extern "C" EtsInt DebuggerAPIGetLocalInt(EtsLong regNumber)
{
    return DebuggerAPIGetLocal<EtsInt>(regNumber);
}

extern "C" EtsFloat DebuggerAPIGetLocalFloat(EtsLong regNumber)
{
    return DebuggerAPIGetLocal<EtsFloat>(regNumber);
}

extern "C" EtsDouble DebuggerAPIGetLocalDouble(EtsLong regNumber)
{
    return DebuggerAPIGetLocal<EtsDouble>(regNumber);
}

extern "C" EtsLong DebuggerAPIGetLocalLong(EtsLong regNumber)
{
    return DebuggerAPIGetLocal<EtsLong>(regNumber);
}

extern "C" EtsObject *DebuggerAPIGetLocalObject(EtsLong regNumber)
{
    return DebuggerAPIGetLocal<EtsObject *>(regNumber);
}

template <typename T>
static void DebuggerAPISetLocal(EtsLong regNumber, T value)
{
    static constexpr uint32_t PREVIOUS_FRAME_DEPTH = 1;

    auto *executionCtx = EtsExecutionContext::GetCurrent();
    auto *runtime = executionCtx->GetPandaVM()->GetRuntime();
    if (UNLIKELY(!runtime->IsDebugMode())) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreError, "Debugger is not enabled");
        return;
    }
    if (UNLIKELY(regNumber < 0)) {
        SetRuntimeException(regNumber, executionCtx, ark::ets::tooling::EtsTypeName<T>::NAME);
        return;
    }

    auto vregValue = ark::ets::tooling::EtsValueToVRegValue<T>(value);
    auto &debugger = runtime->StartDebugSession()->GetDebugger();
    auto err = debugger.SetVariable(ark::ets::tooling::CoroutineToPtThread(executionCtx->GetMT()), PREVIOUS_FRAME_DEPTH,
                                    regNumber, vregValue);
    if (err) {
        LOG(ERROR, DEBUGGER) << "Failed to set local variable: " << err.value().GetMessage();
        SetRuntimeException(regNumber, executionCtx, ark::ets::tooling::EtsTypeName<T>::NAME);
    }
}

extern "C" void DebuggerAPISetLocalBoolean(EtsLong regNumber, EtsBoolean value)
{
    DebuggerAPISetLocal<EtsBoolean>(regNumber, value);
}

extern "C" void DebuggerAPISetLocalByte(EtsLong regNumber, EtsByte value)
{
    DebuggerAPISetLocal<EtsByte>(regNumber, value);
}

extern "C" void DebuggerAPISetLocalShort(EtsLong regNumber, EtsShort value)
{
    DebuggerAPISetLocal<EtsShort>(regNumber, value);
}

extern "C" void DebuggerAPISetLocalChar(EtsLong regNumber, EtsChar value)
{
    DebuggerAPISetLocal<EtsChar>(regNumber, value);
}

extern "C" void DebuggerAPISetLocalInt(EtsLong regNumber, EtsInt value)
{
    DebuggerAPISetLocal<EtsInt>(regNumber, value);
}

extern "C" void DebuggerAPISetLocalFloat(EtsLong regNumber, EtsFloat value)
{
    DebuggerAPISetLocal<EtsFloat>(regNumber, value);
}

extern "C" void DebuggerAPISetLocalDouble(EtsLong regNumber, EtsDouble value)
{
    DebuggerAPISetLocal<EtsDouble>(regNumber, value);
}

extern "C" void DebuggerAPISetLocalLong(EtsLong regNumber, EtsLong value)
{
    DebuggerAPISetLocal<EtsLong>(regNumber, value);
}

extern "C" void DebuggerAPISetLocalObject(EtsLong regNumber, EtsObject *value)
{
    DebuggerAPISetLocal<EtsObject *>(regNumber, value);
}
}  // namespace ark::ets::intrinsics
