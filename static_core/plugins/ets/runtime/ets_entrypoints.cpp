/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ets_entrypoints.h"

#include "libpandafile/shorty_iterator.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_runtime_interface.h"
#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "runtime/arch/helpers.h"
#include "runtime/interpreter/vregister_iterator.h"

namespace ark::ets {

using TypeId = panda_file::Type::TypeId;

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-label-as-value"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

static inline bool Launch(EtsCoroutine *currentCoro, Method *method, const EtsHandle<EtsPromise> &promiseHandle,
                          PandaVector<Value> &&args)
{
    ASSERT(currentCoro != nullptr);
    PandaEtsVM *etsVm = currentCoro->GetPandaVM();
    auto promiseRef = etsVm->GetGlobalObjectStorage()->Add(promiseHandle.GetPtr(), mem::Reference::ObjectType::WEAK);
    auto evt = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(promiseRef);
    promiseHandle.GetPtr()->SetEventPtr(evt);
    // create the coro and put it to the ready queue
    auto *coro = currentCoro->GetCoroutineManager()->Launch(evt, method, std::move(args), CoroutineLaunchMode::DEFAULT);
    if (UNLIKELY(coro == nullptr)) {
        // OOM
        promiseHandle.GetPtr()->SetEventPtr(nullptr);
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(evt);
        return false;
    }
    return true;
}

void LaunchCoroutine(Method *method, ObjectHeader *obj, uint64_t *args, ObjectHeader *thisObj)
{
    auto *promise = reinterpret_cast<EtsPromise *>(obj);
    ASSERT(promise != nullptr);

    PandaVector<Value> values;
    if (thisObj != nullptr) {
        ASSERT(!method->IsStatic());
        // Add this for virtual call
        values.push_back(Value(thisObj));
    } else {
        ASSERT(method->IsStatic());
    }
    arch::ArgReaderStack<RUNTIME_ARCH> argReader(reinterpret_cast<uint8_t *>(args));
    arch::ValueWriter writer(&values);
    ARCH_COPY_METHOD_ARGS(method, argReader, writer);

    auto *currentCoro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(currentCoro);
    EtsHandle<EtsPromise> promiseHandle(currentCoro, promise);
    bool successfulLaunch = Launch(currentCoro, method, promiseHandle, std::move(values));
    if (UNLIKELY(!successfulLaunch)) {
        HandlePendingException();
        UNREACHABLE();
    }
}

extern "C" void CreateLaunchStaticCoroutineEntrypoint(Method *method, ObjectHeader *obj, uint64_t *args)
{
    LaunchCoroutine(method, obj, args, nullptr);
}

extern "C" void CreateLaunchVirtualCoroutineEntrypoint(Method *method, ObjectHeader *obj, uint64_t *args,
                                                       ObjectHeader *thisObj)
{
    LaunchCoroutine(method, obj, args, thisObj);
}

template <BytecodeInstruction::Format FORMAT>
ObjectHeader *LaunchFromInterpreterImpl(Method *method, Frame *frame, const uint8_t *pc)
{
    EtsPromise *promise = EtsPromise::Create();
    if (UNLIKELY(promise == nullptr)) {
        return nullptr;
    }

    auto numArgs = method->GetNumArgs();
    auto args = PandaVector<Value> {numArgs};
    auto frameHandler = StaticFrameHandler(frame);
    auto vregIter = interpreter::VRegisterIterator<FORMAT> {BytecodeInstruction(pc), frame};
    for (decltype(numArgs) i = 0; i < numArgs; ++i) {
        args[i] = Value::FromVReg(frameHandler.GetVReg(vregIter.GetVRegIdx(i)));
    }

    auto *currentCoro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(currentCoro);
    EtsHandle<EtsPromise> promiseHandle(currentCoro, promise);
    bool successfulLaunch = Launch(currentCoro, method, promiseHandle, std::move(args));
    if (UNLIKELY(!successfulLaunch)) {
        return nullptr;
    }
    frame->GetAccAsVReg().SetReference(promiseHandle.GetPtr());
    return promiseHandle.GetPtr();
}

extern "C" ObjectHeader *LaunchFromInterpreterShort(Method *method, Frame *frame, const uint8_t *pc)
{
    return LaunchFromInterpreterImpl<BytecodeInstruction::Format::PREF_V4_V4_ID16>(method, frame, pc);
}

extern "C" ObjectHeader *LaunchFromInterpreterLong(Method *method, Frame *frame, const uint8_t *pc)
{
    return LaunchFromInterpreterImpl<BytecodeInstruction::Format::PREF_V4_V4_V4_V4_ID16>(method, frame, pc);
}

extern "C" ObjectHeader *LaunchFromInterpreterRange(Method *method, Frame *frame, const uint8_t *pc)
{
    return LaunchFromInterpreterImpl<BytecodeInstruction::Format::PREF_V8_ID16>(method, frame, pc);
}

}  // namespace ark::ets
