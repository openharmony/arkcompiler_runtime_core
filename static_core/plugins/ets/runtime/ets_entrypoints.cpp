/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

namespace panda::ets {

using TypeId = panda_file::Type::TypeId;

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-label-as-value"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

static inline bool Launch(EtsCoroutine *current_coro, Method *method, const EtsHandle<EtsPromise> &promise_handle,
                          PandaVector<Value> &&args)
{
    ASSERT(current_coro != nullptr);
    PandaEtsVM *ets_vm = current_coro->GetPandaVM();
    auto promise_ref = ets_vm->GetGlobalObjectStorage()->Add(promise_handle.GetPtr(), mem::Reference::ObjectType::WEAK);
    auto evt = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(promise_ref);
    promise_handle.GetPtr()->SetEventPtr(evt);
    // create the coro and put it to the ready queue
    auto *coro = current_coro->GetCoroutineManager()->Launch(evt, method, std::move(args), CoroutineAffinity::NONE);
    if (UNLIKELY(coro == nullptr)) {
        // OOM
        promise_handle.GetPtr()->SetEventPtr(nullptr);
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(evt);
        return false;
    }
    return true;
}

void LaunchCoroutine(Method *method, ObjectHeader *obj, uint64_t *args, ObjectHeader *this_obj)
{
    auto *promise = reinterpret_cast<EtsPromise *>(obj);
    ASSERT(promise != nullptr);

    PandaVector<Value> values;
    if (this_obj != nullptr) {
        ASSERT(!method->IsStatic());
        // Add this for virtual call
        values.push_back(Value(this_obj));
    } else {
        ASSERT(method->IsStatic());
    }
    arch::ArgReaderStack<RUNTIME_ARCH> arg_reader(reinterpret_cast<uint8_t *>(args));
    arch::ValueWriter writer(&values);
    ARCH_COPY_METHOD_ARGS(method, arg_reader, writer);

    auto *current_coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(current_coro);
    EtsHandle<EtsPromise> promise_handle(current_coro, promise);
    bool successful_launch = Launch(current_coro, method, promise_handle, std::move(values));
    if (UNLIKELY(!successful_launch)) {
        HandlePendingException();
        UNREACHABLE();
    }
}

extern "C" void CreateLaunchStaticCoroutineEntrypoint(Method *method, ObjectHeader *obj, uint64_t *args)
{
    // BEGIN_ENTRYPOINT();

    LaunchCoroutine(method, obj, args, nullptr);
}

extern "C" void CreateLaunchVirtualCoroutineEntrypoint(Method *method, ObjectHeader *obj, uint64_t *args,
                                                       ObjectHeader *this_obj)
{
    // BEGIN_ENTRYPOINT();

    LaunchCoroutine(method, obj, args, this_obj);
}

template <BytecodeInstruction::Format FORMAT>
ObjectHeader *LaunchFromInterpreterImpl(Method *method, Frame *frame, const uint8_t *pc)
{
    EtsPromise *promise = EtsPromise::Create();
    if (UNLIKELY(promise == nullptr)) {
        return nullptr;
    }

    auto num_args = method->GetNumArgs();
    auto args = PandaVector<Value> {num_args};
    auto frame_handler = StaticFrameHandler(frame);
    auto vreg_iter = interpreter::VRegisterIterator<FORMAT> {BytecodeInstruction(pc), frame};
    for (decltype(num_args) i = 0; i < num_args; ++i) {
        args[i] = Value::FromVReg(frame_handler.GetVReg(vreg_iter.GetVRegIdx(i)));
    }

    auto *current_coro = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(current_coro);
    EtsHandle<EtsPromise> promise_handle(current_coro, promise);
    bool successful_launch = Launch(current_coro, method, promise_handle, std::move(args));
    if (UNLIKELY(!successful_launch)) {
        return nullptr;
    }
    frame->GetAccAsVReg().SetReference(promise_handle.GetPtr());
    return promise_handle.GetPtr();
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

}  // namespace panda::ets
