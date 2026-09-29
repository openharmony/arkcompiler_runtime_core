/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <array>
#include <limits>

#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "include/object_header.h"
#include "intrinsics.h"
#include "intrinsics/helpers/ets_intrinsics_helpers.h"
#include "libarkbase/utils/logger.h"
#include "runtime/handle_scope-inl.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_array.h"
#include "plugins/ets/runtime/types/ets_std_core_array.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_map.h"
#include "plugins/ets/runtime/ets_stubs.h"
#include "runtime/execution/coroutines/stackful/stackful_coroutine_state_info.h"

namespace ark::ets::intrinsics {

namespace {

constexpr const char *COROUTINE_DUMP_INFO_DESCRIPTOR = "Lstd/dfx/CoroutineDumpInfo;";

enum class CoroutineDumpField : size_t {
    WORKER_NAME,
    IS_MAIN,
    COROUTINE_NAME,
    STATUS,
    TYPE,
    WAIT_REASON,
    LOCK_MODE,
    READER_COUNT,
    WRITER_COUNT,
    CALL_STACK,
    COUNT,
};

enum class CoroutineDumpString : size_t {
    WORKER_NAME,
    COROUTINE_NAME,
    STATUS,
    TYPE,
    WAIT_REASON,
    LOCK_MODE,
    CALL_STACK,
    COUNT,
};

using CoroutineDumpFields = std::array<EtsField *, static_cast<size_t>(CoroutineDumpField::COUNT)>;
using CoroutineDumpStrings = std::array<EtsHandle<EtsString>, static_cast<size_t>(CoroutineDumpString::COUNT)>;

EtsField *GetField(const CoroutineDumpFields &fields, CoroutineDumpField field)
{
    return fields[static_cast<size_t>(field)];
}

EtsHandle<EtsString> &GetString(CoroutineDumpStrings &strings, CoroutineDumpString string)
{
    return strings[static_cast<size_t>(string)];
}

const char *CoroutineStatusToString(Coroutine::Status status)
{
    switch (status) {
        case Coroutine::Status::CREATED:
            return "CREATED";
        case Coroutine::Status::RUNNABLE:
            return "RUNNABLE";
        case Coroutine::Status::RUNNING:
            return "RUNNING";
        case Coroutine::Status::BLOCKED:
            return "BLOCKED";
        case Coroutine::Status::TERMINATING:
            return "TERMINATING";
        default:
            return "UNKNOWN";
    }
}

const char *CoroutineTypeToString(Coroutine::Type type)
{
    switch (type) {
        case Coroutine::Type::FINALIZER:
            return "FINALIZER";
        case Coroutine::Type::MUTATOR:
            return "MUTATOR";
        case Coroutine::Type::SCHEDULER:
            return "SCHEDULER";
        default:
            return "UNKNOWN";
    }
}

const char *WaitReasonToString(WaitReason reason)
{
    switch (reason) {
        case WaitReason::MUTEX_LOCK:
            return "MUTEX_LOCK";
        case WaitReason::RWLOCK_READ:
            return "RWLOCK_READ";
        case WaitReason::RWLOCK_WRITE:
            return "RWLOCK_WRITE";
        default:
            return "NONE";
    }
}

const char *WaitModeToString(WaitMode mode)
{
    switch (mode) {
        case WaitMode::MUTEX:
            return "MUTEX";
        case WaitMode::READLOCK:
            return "READLOCK";
        case WaitMode::WRITELOCK:
            return "WRITELOCK";
        default:
            return "NONE";
    }
}

CoroutineDumpFields GetCoroutineDumpFields(EtsClass *infoClass)
{
    return {
        infoClass->GetFieldIDByName("workerName_"),    infoClass->GetFieldIDByName("isMain_"),
        infoClass->GetFieldIDByName("coroutineName_"), infoClass->GetFieldIDByName("status_"),
        infoClass->GetFieldIDByName("type_"),          infoClass->GetFieldIDByName("waitReason_"),
        infoClass->GetFieldIDByName("lockMode_"),      infoClass->GetFieldIDByName("readerCount_"),
        infoClass->GetFieldIDByName("writerCount_"),   infoClass->GetFieldIDByName("callStack_"),
    };
}

void SetCoroutineDumpInfoFields(EtsObject *result, const CoroutineDumpFields &fields, CoroutineDumpStrings &strings,
                                const StackfulCoroutineWorkerStateInfo &workerInfo, const WaitDiagnosticInfo &waitInfo)
{
    result->SetFieldObject(GetField(fields, CoroutineDumpField::WORKER_NAME),
                           GetString(strings, CoroutineDumpString::WORKER_NAME)->AsObject());
    result->SetFieldPrimitive<EtsBoolean>(GetField(fields, CoroutineDumpField::IS_MAIN),
                                          ToEtsBoolean(workerInfo.IsMainWorker()));
    result->SetFieldObject(GetField(fields, CoroutineDumpField::COROUTINE_NAME),
                           GetString(strings, CoroutineDumpString::COROUTINE_NAME)->AsObject());
    result->SetFieldObject(GetField(fields, CoroutineDumpField::STATUS),
                           GetString(strings, CoroutineDumpString::STATUS)->AsObject());
    result->SetFieldObject(GetField(fields, CoroutineDumpField::TYPE),
                           GetString(strings, CoroutineDumpString::TYPE)->AsObject());
    result->SetFieldObject(GetField(fields, CoroutineDumpField::WAIT_REASON),
                           GetString(strings, CoroutineDumpString::WAIT_REASON)->AsObject());
    result->SetFieldObject(GetField(fields, CoroutineDumpField::LOCK_MODE),
                           GetString(strings, CoroutineDumpString::LOCK_MODE)->AsObject());
    result->SetFieldPrimitive<EtsInt>(GetField(fields, CoroutineDumpField::READER_COUNT),
                                      static_cast<EtsInt>(waitInfo.GetReadersAtBlock()));
    result->SetFieldPrimitive<EtsInt>(GetField(fields, CoroutineDumpField::WRITER_COUNT),
                                      static_cast<EtsInt>(waitInfo.GetWritersAtBlock()));
    result->SetFieldObject(GetField(fields, CoroutineDumpField::CALL_STACK),
                           GetString(strings, CoroutineDumpString::CALL_STACK)->AsObject());
}

EtsObject *CreateCoroutineDumpInfo(EtsExecutionContext *executionCtx, EtsClass *infoClass,
                                   const CoroutineDumpFields &fields,
                                   const StackfulCoroutineWorkerStateInfo &workerInfo,
                                   const StackfulCoroutineStateInfo &coroutineInfo)
{
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsObject> result(executionCtx, EtsObject::Create(executionCtx, infoClass));
    if (UNLIKELY(result.GetPtr() == nullptr)) {
        ASSERT(executionCtx->GetMT()->HasPendingException());
        return nullptr;
    }

    const auto &waitInfo = coroutineInfo.GetWaitDiagnosticInfo();
    const std::array<const char *, static_cast<size_t>(CoroutineDumpString::COUNT)> stringValues {
        workerInfo.GetWorkerName().c_str(),
        coroutineInfo.GetCoroutineName().c_str(),
        CoroutineStatusToString(coroutineInfo.GetCoroutineStatus()),
        CoroutineTypeToString(coroutineInfo.GetCoroutineType()),
        WaitReasonToString(waitInfo.GetReason()),
        WaitModeToString(waitInfo.GetRequestedMode()),
        coroutineInfo.GetCallStack().c_str(),
    };
    CoroutineDumpStrings strings;
    for (size_t index = 0; index < stringValues.size(); ++index) {
        strings[index] = EtsHandle<EtsString>(executionCtx, EtsString::CreateFromMUtf8(stringValues[index]));
        if (UNLIKELY(strings[index].GetPtr() == nullptr)) {
            ASSERT(executionCtx->GetMT()->HasPendingException());
            return nullptr;
        }
    }

    SetCoroutineDumpInfoFields(result.GetPtr(), fields, strings, workerInfo, waitInfo);
    return result.GetPtr();
}

}  // namespace

EtsBoolean StdCoreRuntimeIsLittleEndianPlatform()
{
    ASSERT(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ || __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__);
    return __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
}

uint8_t StdCoreRuntimeIsSameReference(EtsObject *source, EtsObject *target)
{
    return (source == target) ? UINT8_C(1) : UINT8_C(0);
}

EtsInt StdCoreRuntimeGetHashCode(EtsObject *source)
{
    ASSERT(source != nullptr);
    return bit_cast<EtsInt>(source->GetHashCode());
}

EtsStdCoreArray *StdDfxCoroutineDumpInfoRaw()
{
    auto *executionCtx = EtsExecutionContext::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    auto *vm = PandaEtsVM::GetCurrent();
    auto coroutineStateInfo = vm->GetCoroutineStateInfo();
    auto *classLinker = vm->GetClassLinker();
    auto *bootContext = classLinker->GetEtsClassLinkerExtension()->GetBootContext();
    auto *infoClass = classLinker->GetClass(COROUTINE_DUMP_INFO_DESCRIPTOR, false, bootContext);
    if (infoClass == nullptr ||
        (!infoClass->IsInitialized() && !classLinker->InitializeClass(executionCtx, infoClass))) {
        return nullptr;
    }

    size_t coroutineCount = 0;
    if (coroutineStateInfo != nullptr) {
        for (const auto &workerInfo : coroutineStateInfo->GetWorkersInfo()) {
            coroutineCount += workerInfo.GetCoroutinesInfo().size();
        }
    }
    ASSERT(coroutineCount <= static_cast<size_t>(std::numeric_limits<EtsInt>::max()));
    EtsHandle<EtsStdCoreArray> result(executionCtx, EtsStdCoreArray::Create(executionCtx, coroutineCount));
    if (result.GetPtr() == nullptr) {
        return nullptr;
    }
    if (coroutineStateInfo == nullptr) {
        return result.GetPtr();
    }

    const auto fields = GetCoroutineDumpFields(infoClass);
    if (std::any_of(fields.begin(), fields.end(), [](EtsField *field) { return field == nullptr; })) {
        return nullptr;
    }
    uint32_t index = 0;
    for (const auto &workerInfo : coroutineStateInfo->GetWorkersInfo()) {
        for (const auto &coroutineInfo : workerInfo.GetCoroutinesInfo()) {
            auto *info = CreateCoroutineDumpInfo(executionCtx, infoClass, fields, workerInfo, coroutineInfo);
            if (info == nullptr) {
                return nullptr;
            }
            result->StdCoreArraySetUnsafe(static_cast<EtsInt>(index++), info);
        }
    }
    return result.GetPtr();
}

EtsLong StdRuntimeGetHashCodeByValue(EtsObject *source)
{
    return static_cast<EtsInt>(EtsStdCoreMap::GetHashCode(source));
}

EtsBoolean StdRuntimeSameValueZero(EtsObject *a, EtsObject *b)
{
    return ToEtsBoolean(ark::ets::intrinsics::helpers::SameValueZero(EtsExecutionContext::GetCurrent(), a, b));
}

static std::string GetClassName(EtsClass *cls)
{
    return cls->GetRuntimeClass()->GetName();
}

static ObjectHeader *CreateTypeCastException(EtsObject *source, EtsClass *target, bool inclUndefined)
{
    auto ctx = EtsExecutionContext::GetCurrent();
    ASSERT(ctx != nullptr);

    auto message = PandaString(source == nullptr ? "undefined" : GetClassName(source->GetClass()));
    message.append(" cannot be cast to ");

    if (LIKELY(target != nullptr)) {
        message.append(GetClassName(target));
        if (inclUndefined) {
            message.append(" or undefined");
        }
    } else {
        message.append(inclUndefined ? "undefined" : "never");
    }

    auto exc = ets::SetupEtsException(ctx, PlatformTypes()->coreClassCastError, message.data());
    if (UNLIKELY(exc == nullptr)) {
        ASSERT(ctx->GetMT()->HasPendingException());
        return nullptr;
    }
    return exc->GetCoreType();
}

ObjectHeader *StdCoreRuntimeFailedTypeCastExclUndefinedStub(EtsObject *source, EtsClass *target)
{
    return CreateTypeCastException(source, target, false);
}

ObjectHeader *StdCoreRuntimeFailedTypeCastInclUndefinedStub(EtsObject *source, EtsClass *target)
{
    return CreateTypeCastException(source, target, true);
}

EtsClass *StdCoreRuntimeGetTypeInfo([[maybe_unused]] EtsObject *header)
{
    return nullptr;
}

ObjectHeader *StdCoreRuntimeAllocSameTypeArray(EtsClass *cls, int32_t length)
{
    if (UNLIKELY(!cls->IsArrayClass())) {
        // should not appear for the optimized version of intrinsic, which is always inlined
        ThrowEtsException(EtsExecutionContext::GetCurrent(), PlatformTypes()->escompatError, "class is not an array");
        return nullptr;
    }
    return coretypes::Array::Create(cls->GetRuntimeClass(), length);
}

}  // namespace ark::ets::intrinsics
