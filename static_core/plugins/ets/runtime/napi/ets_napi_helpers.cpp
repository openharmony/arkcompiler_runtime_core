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

#include "plugins/ets/runtime/napi/ets_napi_helpers.h"

#include "libpandafile/shorty_iterator.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/mem/ets_reference.h"
#include "plugins/ets/runtime/napi/ets_napi.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "runtime/arch/helpers.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"

#include <cstdint>

namespace panda::ets::napi {
namespace {
using Type = panda_file::Type;
using TypeId = panda_file::Type::TypeId;

using ExtArchTraits = arch::ExtArchTraits<RUNTIME_ARCH>;
using ArgReader = arch::ArgReader<RUNTIME_ARCH>;
using ArgCounter = arch::ArgCounter<RUNTIME_ARCH>;

class ArgWriter : public arch::ArgWriter<RUNTIME_ARCH> {
public:
    ArgWriter(Span<uint8_t> *gpr_args, Span<uint8_t> *fpr_args, uint8_t *stack_args)
        : arch::ArgWriter<RUNTIME_ARCH>(gpr_args, fpr_args, stack_args)
    {
    }
    ~ArgWriter() = default;

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<std::is_same<T, ObjectHeader **>::value, void> Write(T v)
    {
        arch::ArgWriter<RUNTIME_ARCH>::Write<EtsReference *>(
            EtsReferenceStorage::NewEtsStackRef(reinterpret_cast<EtsObject **>(v)));
    }

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<!std::is_same<T, ObjectHeader **>::value, void> Write(T v)
    {
        // Check T is not some kind of pointer to ObjectHeader
        static_assert(!std::is_same_v<ObjectHeader, std::remove_cv_t<typename helpers::RemoveAllPointers<T>::type>>);
        arch::ArgWriter<RUNTIME_ARCH>::Write(v);
    }

    NO_COPY_SEMANTIC(ArgWriter);
    NO_MOVE_SEMANTIC(ArgWriter);
};
}  // namespace

extern "C" void EtsNapiEntryPoint();

const void *GetEtsNapiEntryPoint()
{
    return reinterpret_cast<const void *>(EtsNapiEntryPoint);
}

extern "C" void EtsNapiCriticalNativeEntryPoint();

const void *GetEtsNapiCriticalEntryPoint()
{
    return reinterpret_cast<const void *>(EtsNapiCriticalNativeEntryPoint);
}

extern "C" uint32_t EtsNapiCalcStackArgsSpaceSize(Method *method, bool is_critical)
{
    ASSERT(method != nullptr);

    ArgCounter counter;
    if (!is_critical) {
        counter.Count<EtsEnv *>();        // EtsEnv arg
        counter.Count<ObjectHeader *>();  // class or this arg
    }

    panda_file::ShortyIterator it(method->GetShorty());
    ++it;  // Skip the return type
    panda_file::ShortyIterator end;
    while (it != end) {
        Type type = *it++;
        switch (type.GetId()) {
            case TypeId::VOID:
            case TypeId::INVALID:
            case TypeId::TAGGED:
                UNREACHABLE();
            case TypeId::U1:
                counter.Count<bool>();
                break;
            case TypeId::I8:
            case TypeId::U8:
                counter.Count<uint8_t>();
                break;
            case TypeId::I16:
            case TypeId::U16:
                counter.Count<uint16_t>();
                break;
            case TypeId::I32:
            case TypeId::U32:
                counter.Count<uint32_t>();
                break;
            case TypeId::F32:
                counter.Count<float>();
                break;
            case TypeId::F64:
                counter.Count<double>();
                break;
            case TypeId::I64:
            case TypeId::U64:
                counter.Count<uint64_t>();
                break;
            case TypeId::REFERENCE:
                counter.Count<ObjectHeader *>();
                break;
        }
    }

    return counter.GetStackSpaceSize();
}

// Disable warning because the function uses ARCH_COPY_METHOD_ARGS macro.
// The macro uses computed goto
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-label-as-value"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

// input stack structure             output stack structure
// +-------+ <- out_args             +-------+
// |  ...  |                         | x0-x7 |
// +-------+ <- in_reg_args          +-------+
// | x0-x7 |                         | d0-d7 |
// +-------+                         +-------+
// | d0-d7 |                         | stack |
// +-------+                         | arg 0 |
// |  ...  |                         +-------+
// +-------+ <- in_stack_args        |  ...  |
// | stack |                         +-------+
// | arg 0 |                         | stack |
// +-------+                         | arg N |
// |  ...  |                         +-------+
extern "C" void EtsNapiBeginCritical(Method *method, uint8_t *in_regs_args, uint8_t *in_stack_args, uint8_t *out_args,
                                     ManagedThread *thread)
{
    ASSERT(method->IsStatic());
    ASSERT(thread == ManagedThread::GetCurrent());

    Span<uint8_t> in_gpr_args(in_regs_args, ExtArchTraits::GP_ARG_NUM_BYTES);
    Span<uint8_t> in_fpr_args(in_gpr_args.end(), ExtArchTraits::FP_ARG_NUM_BYTES);
    ArgReader arg_reader(in_gpr_args, in_fpr_args, in_stack_args);

    Span<uint8_t> out_gpr_args(out_args, ExtArchTraits::GP_ARG_NUM_BYTES);
    Span<uint8_t> out_fpr_args(out_gpr_args.end(), ExtArchTraits::FP_ARG_NUM_BYTES);
    auto out_stack_args = out_fpr_args.end();
    ArgWriter arg_writer(&out_gpr_args, &out_fpr_args, out_stack_args);

    arg_reader.Read<Method *>();  // Skip method

    if (UNLIKELY(method->GetNativePointer() == nullptr)) {
        auto coroutine = EtsCoroutine::CastFromThread(thread);
        coroutine->GetPandaVM()->ResolveNativeMethod(method);
    }

    ARCH_COPY_METHOD_ARGS(method, arg_reader, arg_writer);

    Runtime::GetCurrent()->GetNotificationManager()->MethodEntryEvent(thread, method);
}

//              Input stack               =======>              Output stack
// 0xFFFF
//       |                        |                      |                        |
//       |       Prev frame       |                      |       Prev frame       |
//       |          ...           |                      |          ...           |
//       +------------------------+                      +------------------------+
//       |          ...           |                      |          ...           |
//       |       stack args       |                      |       stack args       | <--------+
//       |          ...           |                      |          ...           |          |
//       +---+---+----------------+ <- in_stack_args --> +----------------+---+---+          |
//       |   |   |       LR       |                      |       LR       |   |   |          |
//       |   |   |       FP       |                      |       FP       |   |   |          |
//       |   |   |     Method *   |                      |     Method *   |   |   |          |
//       |   | c |      FLAGS     |                      |      FLAGS     | c |   |          |
//       |   | f +----------------+                      +----------------+ f |   |          |
//       |   | r |       ...      |                      |       ...      | r |   |          |
//       |   | a |     locals     |                      |     locals     | a |   |          |
//       |   | m |       ...      |                      |       ...      | m |   |          |
//       |   | e +----------------+                      +----------------+ e |   |          |
//       | N |   |       ...      |                      |       ...      |   | N |          |
//       | A |   |  callee saved  |                      |  callee saved  |   | A |          |
//       | P |   |       ...      |                      |       ...      |   | P |          |
//       | I +---+----------------+                      +----------------+---+ I |          |
//       |   |        ...         |                      |        ...         |   |          |
//       |   |     float args     |                      |     float args     |   |          |
//       | f |        ...         |                      |        ...         | f |          |
//       | r +--------------------+                      +--------------------+ r |          |
//       | a |        ...         |                      |        ...         | a |          |
//       | m |    general args    |                      |    general args    | m | <----+   |
//       | e |        ...         |                      |        ...         | e |      |   |
//       |   |    arg0|Method*    |                      |  arg0|class(opt)   |   |      |   |
//       |   +--------------------+ <-- in_regs_args --> +--------------------+   |      |   |     References
//       |   |                    |                      |        ...         |   |      |   | to ObjectHeader *s
//       |   |                    |                      |  NAPI float args   |   |      |   |    on the stack
//       |   |                    |                      |     (on regs)      |   |      |   |
//       |   |                    |                      |        ...         |   |      |   |
//       |   |                    |                      +--------------------+   |      |   |
//       |   |                    |                      |        ...         |   |      |   |
//       |   |     space for      |                      | NAPI general args  |   | -----+   |
//       |   |     NAPI args      |                      |     (on regs)      |   |          |
//       |   |                    |                      |        ...         |   |          |
//       |   |                    |                      +--------------------+   |          |
//       |   |                    |                      |        ...         |   |          |
//       |   |                    |                      |     NAPI args      |   | ---------+
//       |   |                    |                      |     (on stack)     |   |
//       |   |                    |                      |        ...         |   |
//       +---+--------------------+ <- out_stack_args -> +--------------------+---+
//       |                        |                      |                        |
// 0x0000
extern "C" uint8_t *EtsNapiBegin(Method *method, uint8_t *in_regs_args, uint8_t *in_stack_args, uint8_t *out_stack_args,
                                 ManagedThread *thread)
{
    ASSERT(!method->IsSynchronized());
    ASSERT(thread == ManagedThread::GetCurrent());

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto out_regs_args = in_regs_args - ExtArchTraits ::FP_ARG_NUM_BYTES - ExtArchTraits ::GP_ARG_NUM_BYTES;

    ASSERT(out_stack_args <= out_regs_args);
    ASSERT(out_regs_args < in_regs_args);
    ASSERT(in_regs_args < in_stack_args);

    Span<uint8_t> in_gpr_args(in_regs_args, ExtArchTraits::GP_ARG_NUM_BYTES);
    Span<uint8_t> in_fpr_args(in_gpr_args.end(), ExtArchTraits::FP_ARG_NUM_BYTES);
    ArgReader arg_reader(in_gpr_args, in_fpr_args, in_stack_args);

    Span<uint8_t> out_gpr_args(out_regs_args, ExtArchTraits::GP_ARG_NUM_BYTES);
    Span<uint8_t> out_fpr_args(out_gpr_args.end(), ExtArchTraits::FP_ARG_NUM_BYTES);
    ArgWriter arg_writer(&out_gpr_args, &out_fpr_args, out_stack_args);

    arg_reader.Read<Method *>();  // Skip method

    EtsReference *class_or_this_ref = nullptr;
    if (method->IsStatic()) {
        // Handle class object
        auto class_obj = EtsClass::FromRuntimeClass(method->GetClass())->AsObject();
        ASSERT(class_obj != nullptr);

        // Replace the method pointer (Method *) with a pointer to the class object
        auto class_ptr = reinterpret_cast<EtsObject **>(in_regs_args);
        *class_ptr = class_obj;

        class_or_this_ref = EtsReferenceStorage::NewEtsStackRef(class_ptr);
    } else {
        ASSERT(method->GetNumArgs() != 0);
        ASSERT(!method->GetArgType(0).IsPrimitive());

        // Handle this arg
        auto this_ptr = const_cast<EtsObject **>(arg_reader.ReadPtr<EtsObject *>());
        class_or_this_ref = EtsReferenceStorage::NewEtsStackRef(this_ptr);
    }

    auto coroutine = EtsCoroutine::CastFromThread(thread);
    auto ets_napi_env = coroutine->GetEtsNapiEnv();

    // Prepare eTS NAPI args
    arg_writer.Write(static_cast<EtsEnv *>(ets_napi_env));
    arg_writer.Write(class_or_this_ref);
    ARCH_COPY_METHOD_ARGS(method, arg_reader, arg_writer);

    // Completed the preparation of eTS NAPI arguments on the stack

    // ATTENTION!!!
    // Don't move the following code above, because only from this point on,
    // the stack of the current frame is correct, so we can safely walk it
    // (e.g. stop at a safepoint and walk the stack args of NAPI frame)

    if (UNLIKELY(method->GetNativePointer() == nullptr)) {
        coroutine->GetPandaVM()->ResolveNativeMethod(method);
    }

    Runtime::GetCurrent()->GetNotificationManager()->MethodEntryEvent(thread, method);

    constexpr uint32_t MAX_LOCAL_REF = 4096;
    if (UNLIKELY(!ets_napi_env->GetEtsReferenceStorage()->PushLocalEtsFrame(MAX_LOCAL_REF))) {
        LOG(FATAL, RUNTIME) << "eTS NAPI push local frame failed";
    }

    auto ets_method = EtsMethod::FromRuntimeMethod(method);
    if (!ets_method->IsFastNative()) {
        thread->NativeCodeBegin();
    }

    return out_regs_args;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

extern "C" void EtsNapiEnd(Method *method, ManagedThread *thread, bool is_fast_native)
{
    ASSERT(method != nullptr);
    ASSERT(!method->IsSynchronized());
    ASSERT(thread == ManagedThread::GetCurrent());

    if (!is_fast_native) {
        thread->NativeCodeEnd();
    }

    Runtime::GetCurrent()->GetNotificationManager()->MethodExitEvent(thread, method);

    auto coroutine = EtsCoroutine::CastFromThread(thread);
    auto storage = coroutine->GetEtsNapiEnv()->GetEtsReferenceStorage();
    storage->PopLocalEtsFrame(nullptr);
}

extern "C" EtsObject *EtsNapiObjEnd(Method *method, EtsReference *ets_ref, ManagedThread *thread, bool is_fast_native)
{
    ASSERT(method != nullptr);
    ASSERT(!method->IsSynchronized());
    ASSERT(thread == ManagedThread::GetCurrent());

    // End native scope first to get into managed scope for object manipulation
    if (!is_fast_native) {
        thread->NativeCodeEnd();
    }

    Runtime::GetCurrent()->GetNotificationManager()->MethodExitEvent(thread, method);

    EtsObject *ret = nullptr;

    auto coroutine = EtsCoroutine::CastFromThread(thread);
    auto storage = coroutine->GetEtsNapiEnv()->GetEtsReferenceStorage();
    if (ets_ref != nullptr) {
        ret = storage->GetEtsObject(ets_ref);
    }

    storage->PopLocalEtsFrame(nullptr);

    return ret;
}

extern "C" bool IsEtsMethodFastNative(Method *method)
{
    return EtsMethod::FromRuntimeMethod(method)->IsFastNative();
}

// Disable warning because the function uses ARCH_COPY_METHOD_ARGS macro.
// The macro uses computed goto
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-label-as-value"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
extern "C" ObjectPointerType EtsAsyncCall(Method *method, EtsCoroutine *current_coro, uint8_t *reg_args,
                                          uint8_t *stack_args)
{
    PandaEtsVM *vm = current_coro->GetPandaVM();
    EtsClassLinker *ets_class_linker = vm->GetClassLinker();
    Method *impl = ets_class_linker->GetAsyncImplMethod(method, current_coro);
    if (impl == nullptr) {
        ASSERT(current_coro->HasPendingException());
        // Exception is thrown by GetAsyncImplMethod
        return 0;
    }
    ASSERT(!current_coro->HasPendingException());

    EtsPromise *promise = EtsPromise::Create(current_coro);
    if (UNLIKELY(promise == nullptr)) {
        ThrowOutOfMemoryError(current_coro, "Cannot allocate Promise");
        return 0;
    }
    auto promise_ref = vm->GetGlobalObjectStorage()->Add(promise, mem::Reference::ObjectType::WEAK);
    auto evt = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(promise_ref);
    promise->SetEventPtr(evt);

    auto *cm = current_coro->GetCoroutineManager();

    PandaVector<Value> args;
    args.reserve(method->GetNumArgs());
    Span<uint8_t> gpr_args(reg_args, ExtArchTraits::GP_ARG_NUM_BYTES);
    Span<uint8_t> fpr_args(gpr_args.end(), ExtArchTraits::FP_ARG_NUM_BYTES);
    ArgReader arg_reader(gpr_args, fpr_args, stack_args);
    arg_reader.Read<Method *>();  // Skip method
    if (method->IsStatic()) {
        // Replace the method pointer (Method *) by the pointer to the object class
        // to satisfy stack walker
        auto class_obj = EtsClass::FromRuntimeClass(method->GetClass())->AsObject();
        ASSERT(class_obj != nullptr);
        auto class_ptr = reinterpret_cast<EtsObject **>(reg_args);
        *class_ptr = class_obj;
    } else {
        // Handle this arg
        ASSERT(method->GetNumArgs() != 0);
        ASSERT(!method->GetArgType(0).IsPrimitive());
        args.push_back(Value(*const_cast<ObjectHeader **>(arg_reader.ReadPtr<ObjectHeader *>())));
    }

    arch::ValueWriter writer(&args);
    ARCH_COPY_METHOD_ARGS(method, arg_reader, writer);

    [[maybe_unused]] EtsHandleScope scope(current_coro);
    EtsHandle<EtsPromise> promise_handle(current_coro, promise);
    auto *coro = cm->Launch(evt, impl, std::move(args));
    if (UNLIKELY(coro == nullptr)) {
        ASSERT(current_coro->HasPendingException());
        // OOM is thrown by Launch
        promise_handle.GetPtr()->SetEventPtr(nullptr);
        Runtime::GetCurrent()->GetInternalAllocator()->Delete(evt);
        return 0;
    }
    cm->Schedule();
    return ToObjPtr(promise_handle.GetPtr());
}
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}  // namespace panda::ets::napi
