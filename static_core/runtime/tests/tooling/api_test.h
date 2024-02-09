/**
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

#ifndef PANDA_RUNTIME_TESTS_TOOLING_API_TEST_H
#define PANDA_RUNTIME_TESTS_TOOLING_API_TEST_H

#include <utility>
#include "runtime/include/tooling/debug_interface.h"

namespace ark::tooling::test {
using BreakpointCallback = std::function<bool(PtThread, Method *, const PtLocation &)>;
using LoadModuleCallback = std::function<bool(std::string_view)>;
using PausedCallback = std::function<bool(PauseReason)>;
using ExceptionCallback =
    std::function<bool(PtThread, Method *, const PtLocation &, ObjectHeader *, Method *, const PtLocation &)>;
using ExceptionCatchCallback = std::function<bool(PtThread, Method *, const PtLocation &, ObjectHeader *)>;
using PropertyAccessCallback = std::function<bool(PtThread, Method *, const PtLocation &, ObjectHeader *, PtProperty)>;
using PropertyModificationCallback =
    std::function<bool(PtThread, Method *, const PtLocation &, ObjectHeader *, PtProperty, VRegValue)>;
using FramePopCallback = std::function<bool(PtThread, Method *, bool)>;
using GarbageCollectionStartCallback = std::function<bool()>;
using GarbageCollectionFinishCallback = std::function<bool()>;
using ObjectAllocCallback = std::function<bool(BaseClass *, ObjectHeader *, PtThread, size_t)>;
using MethodEntryCallback = std::function<bool(PtThread, Method *)>;
using MethodExitCallback = std::function<bool(PtThread, Method *, bool, VRegValue)>;
using SingleStepCallback = std::function<bool(PtThread, Method *, const PtLocation &)>;
using ThreadStartCallback = std::function<bool(PtThread)>;
using ThreadEndCallback = std::function<bool(PtThread)>;
using VmStartCallback = std::function<bool()>;
using VmInitializationCallback = std::function<bool(PtThread)>;
using VmDeathCallback = std::function<bool()>;
using ExceptionRevokedCallback = std::function<bool(ExceptionWrapper, ExceptionID)>;
using ExecutionContextCreatedCallback = std::function<bool(ExecutionContextWrapper)>;
using ExecutionContextDestroyedCallback = std::function<bool(ExecutionContextWrapper)>;
using ExecutionContextsClearedCallback = std::function<bool()>;
using InspectRequestedCallback = std::function<bool(PtObject, PtObject)>;
using ClassLoadCallback = std::function<bool(PtThread, BaseClass *)>;
using ClassPrepareCallback = std::function<bool(PtThread, BaseClass *)>;
using MonitorWaitCallback = std::function<bool(PtThread, ObjectHeader *, int64_t)>;
using MonitorWaitedCallback = std::function<bool(PtThread, ObjectHeader *, bool)>;
using MonitorContendedEnterCallback = std::function<bool(PtThread, ObjectHeader *)>;
using MonitorContendedEnteredCallback = std::function<bool(PtThread, ObjectHeader *)>;

using Scenario = std::function<bool()>;

enum class DebugEvent {
    BREAKPOINT,
    LOAD_MODULE,
    PAUSED,
    EXCEPTION,
    EXCEPTION_CATCH,
    FIELD_ACCESS,
    FIELD_MODIFICATION,
    FRAME_POP,
    GARBAGE_COLLECTIION_START,
    GARBAGE_COLLECTIION_FINISH,
    METHOD_ENTRY,
    METHOD_EXIT,
    SINGLE_STEP,
    THREAD_START,
    THREAD_END,
    VM_START,
    VM_INITIALIZATION,
    VM_DEATH,
    EXCEPTION_REVOKED,
    EXECUTION_CONTEXT_CREATED,
    EXECUTION_CONTEXT_DESTROYED,
    EXECUTION_CONTEXT_CLEARED,
    INSPECT_REQUESTED,
    CLASS_LOAD,
    CLASS_PREPARE,
    MONITOR_WAIT,
    MONITOR_WAITED,
    MONITOR_CONTENDED_ENTER,
    MONITOR_CONTENDED_ENTERED,
    UNINITIALIZED
};

std::ostream &operator<<(std::ostream &out, DebugEvent value);

struct ApiTest {
    NO_COPY_SEMANTIC(ApiTest);
    NO_MOVE_SEMANTIC(ApiTest);
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    BreakpointCallback breakpoint;
    LoadModuleCallback loadModule;
    PausedCallback paused;
    ExceptionCallback exception;
    ExceptionCatchCallback exceptionCatch;
    PropertyAccessCallback propertyAccess;
    PropertyModificationCallback propertyModification;
    FramePopCallback framePop;
    GarbageCollectionStartCallback garbageCollectionStart;
    GarbageCollectionFinishCallback garbageCollectionFinish;
    ObjectAllocCallback objectAlloc;
    MethodEntryCallback methodEntry;
    MethodExitCallback methodExit;
    SingleStepCallback singleStep;
    ThreadStartCallback threadStart;
    ThreadEndCallback threadEnd;
    VmStartCallback vmStart;
    VmInitializationCallback vmInit;
    VmDeathCallback vmDeath;
    ExceptionRevokedCallback exceptionRevoked;
    ExecutionContextCreatedCallback executionContextCreated;
    ExecutionContextDestroyedCallback executionContextDestroyed;
    ExecutionContextsClearedCallback executionContextCleared;
    InspectRequestedCallback inspectRequested;
    ClassLoadCallback classLoad;
    ClassPrepareCallback classPrepare;
    MonitorWaitCallback monitorWait;
    MonitorWaitedCallback monitorWaited;
    MonitorContendedEnterCallback monitorContendedEnter;
    MonitorContendedEnteredCallback monitorContendedEntered;

    Scenario scenario;
    DebugInterface *debugInterface {nullptr};
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    ApiTest();
    virtual ~ApiTest() = default;

    virtual std::pair<const char *, const char *> GetEntryPoint() = 0;
};
}  // namespace ark::tooling::test

#endif  // PANDA_RUNTIME_TESTS_TOOLING_API_TEST_H
