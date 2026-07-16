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
#include "plugins/ets/runtime/tooling/helpers.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark::ets::intrinsics {

static void SetRuntimeException(EtsLong regNumber, EtsExecutionContext *executionCtx, std::string_view typeName)
{
    auto errorMsg =
        "Failed to access variable at vreg #" + std::to_string(regNumber) + " and type " + std::string(typeName);
    ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->escompatError, errorMsg);
}

template <typename T>
static T DebuggerAPIGetLocal(EtsExecutionContext *executionCtx, EtsLong regNumber)
{
    static constexpr uint32_t PREVIOUS_FRAME_DEPTH = 1;

    auto *runtime = executionCtx->GetPandaVM()->GetRuntime();
    if (UNLIKELY(!runtime->IsDebugMode())) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->escompatError, "Debugger is not enabled");
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

EtsBoolean DebuggerAPIGetLocalBoolean(EtsLong regNumber)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    return DebuggerAPIGetLocal<EtsBoolean>(executionCtx, regNumber);
}

EtsByte DebuggerAPIGetLocalByte(EtsLong regNumber)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    return DebuggerAPIGetLocal<EtsByte>(executionCtx, regNumber);
}

EtsShort DebuggerAPIGetLocalShort(EtsLong regNumber)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    return DebuggerAPIGetLocal<EtsShort>(executionCtx, regNumber);
}

EtsChar DebuggerAPIGetLocalChar(EtsLong regNumber)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    return DebuggerAPIGetLocal<EtsChar>(executionCtx, regNumber);
}

EtsInt DebuggerAPIGetLocalInt(EtsLong regNumber)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    return DebuggerAPIGetLocal<EtsInt>(executionCtx, regNumber);
}

EtsFloat DebuggerAPIGetLocalFloat(EtsLong regNumber)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    return DebuggerAPIGetLocal<EtsFloat>(executionCtx, regNumber);
}

EtsDouble DebuggerAPIGetLocalDouble(EtsLong regNumber)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    return DebuggerAPIGetLocal<EtsDouble>(executionCtx, regNumber);
}

EtsLong DebuggerAPIGetLocalLong(EtsLong regNumber)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    return DebuggerAPIGetLocal<EtsLong>(executionCtx, regNumber);
}

EtsObject *DebuggerAPIGetLocalObject(EtsLong regNumber)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(executionCtx->GetMT());

    auto *obj = DebuggerAPIGetLocal<ObjectHeader *>(executionCtx, regNumber);
    obj = (obj == nullptr) ? executionCtx->GetNullValue() : obj;
    VMHandle<ObjectHeader> objHandle(executionCtx->GetMT(), obj);
    return EtsObject::FromCoreType(objHandle.GetPtr());
}

template <typename T>
static void DebuggerAPISetLocal(EtsExecutionContext *executionCtx, EtsLong regNumber, T value)
{
    static constexpr uint32_t PREVIOUS_FRAME_DEPTH = 1;

    auto *runtime = executionCtx->GetPandaVM()->GetRuntime();
    if (UNLIKELY(!runtime->IsDebugMode())) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->escompatError, "Debugger is not enabled");
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

void DebuggerAPISetLocalBoolean(EtsLong regNumber, EtsBoolean value)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    DebuggerAPISetLocal<EtsBoolean>(executionCtx, regNumber, value);
}

void DebuggerAPISetLocalByte(EtsLong regNumber, EtsByte value)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    DebuggerAPISetLocal<EtsByte>(executionCtx, regNumber, value);
}

void DebuggerAPISetLocalShort(EtsLong regNumber, EtsShort value)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    DebuggerAPISetLocal<EtsShort>(executionCtx, regNumber, value);
}

void DebuggerAPISetLocalChar(EtsLong regNumber, EtsChar value)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    DebuggerAPISetLocal<EtsChar>(executionCtx, regNumber, value);
}

void DebuggerAPISetLocalInt(EtsLong regNumber, EtsInt value)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    DebuggerAPISetLocal<EtsInt>(executionCtx, regNumber, value);
}

void DebuggerAPISetLocalFloat(EtsLong regNumber, EtsFloat value)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    DebuggerAPISetLocal<EtsFloat>(executionCtx, regNumber, value);
}

void DebuggerAPISetLocalDouble(EtsLong regNumber, EtsDouble value)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    DebuggerAPISetLocal<EtsDouble>(executionCtx, regNumber, value);
}

void DebuggerAPISetLocalLong(EtsLong regNumber, EtsLong value)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    DebuggerAPISetLocal<EtsLong>(executionCtx, regNumber, value);
}

void DebuggerAPISetLocalObject(EtsLong regNumber, EtsObject *value)
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(executionCtx->GetMT());
    VMHandle<EtsObject> objHandle(executionCtx->GetMT(), value->GetCoreType());

    DebuggerAPISetLocal<ObjectHeader *>(executionCtx, regNumber, objHandle.GetPtr()->GetCoreType());
}
}  // namespace ark::ets::intrinsics
