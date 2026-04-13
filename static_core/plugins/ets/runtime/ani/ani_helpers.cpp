/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/ani/ani_helpers.h"
#include "plugins/ets/runtime/ani/verify/types/vref.h"
#include "runtime/execution/job_launch.h"
#include "libarkfile/shorty_iterator.h"
#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ani/scoped_objects_fix.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/mem/ets_reference.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_promise.h"
#include "plugins/ets/runtime/ets_handle_scope.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "runtime/arch/helpers.h"
#include "runtime/include/managed_thread.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime.h"
#include "runtime/include/runtime_notification.h"
#include "plugins/ets/runtime/ani/ani_type_check.h"

#include <cstdint>

namespace ark::ets::ani {
namespace {
using Type = panda_file::Type;
using TypeId = panda_file::Type::TypeId;

using ExtArchTraits = arch::ExtArchTraits<RUNTIME_ARCH>;
using ArgReader = arch::ArgReader<RUNTIME_ARCH>;
using ArgCounter = arch::ArgCounter<RUNTIME_ARCH>;

class ArgWriter : public arch::ArgWriter<RUNTIME_ARCH> {
public:
    ArgWriter(Span<uint8_t> gprArgs, Span<uint8_t> fprArgs, uint8_t *stackArgs)
        : arch::ArgWriter<RUNTIME_ARCH>(gprArgs, fprArgs, stackArgs)
    {
    }
    ~ArgWriter() = default;

    template <class T>
    ALWAYS_INLINE typename std::enable_if_t<std::is_same<T, ObjectHeader **>::value, void> Write(T v)
    {
        EtsReference *ref = nullptr;
        auto *objPtr = reinterpret_cast<EtsObject **>(v);
        ASSERT(objPtr != nullptr);
        if (EtsReferenceStorage::IsUndefinedEtsObject(*objPtr)) {
            ref = EtsReference::GetUndefined();
        } else {
            ref = EtsReferenceStorage::NewEtsStackRef(objPtr);
        }
        arch::ArgWriter<RUNTIME_ARCH>::Write<EtsReference *>(ref);
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

extern "C" void AniEntryPoint();
extern "C" void AniVerifyEntryPoint();

const void *GetAniEntryPoint(bool verifyMode)
{
    if (verifyMode) {
        return reinterpret_cast<const void *>(AniVerifyEntryPoint);
    }
    return reinterpret_cast<const void *>(AniEntryPoint);
}

extern "C" void AniCriticalNativeEntryPoint();

const void *GetAniCriticalEntryPoint()
{
    return reinterpret_cast<const void *>(AniCriticalNativeEntryPoint);
}

extern "C" uint32_t AniCalcStackArgsSpaceSize(Method *method, bool isCritical)
{
    ASSERT(method != nullptr);

    ArgCounter counter;
    if (!isCritical) {
        counter.Count<ani_env *>();       // ani_env arg
        counter.Count<ObjectHeader *>();  // class or this arg
    }

    panda_file::ShortyIterator it(method->GetShorty());
    ++it;  // Skip the return type
    panda_file::ShortyIterator end;
    while (it != end) {
        Type type = *it++;
        switch (type.GetId()) {
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
            default:
                UNREACHABLE();
                break;
        }
    }

    return counter.GetStackSpaceSize();
}

static void ThrowUnresolvedMethodException(Method *method, ManagedThread *thread)
{
    PandaStringStream ss;
    ss << "No implementation found for " << method->GetFullName();
    ThrowEtsException(EtsExecutionContext::FromMT(thread), PlatformTypes(thread)->coreLinkerUnresolvedMethodError,
                      ss.str());
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
extern "C" void AniBeginCritical(Method *method, uint8_t *inRegsArgs, uint8_t *inStackArgs, uint8_t *outArgs,
                                 ManagedThread *thread)
{
    ASSERT(method->IsStatic());
    ASSERT(thread == ManagedThread::GetCurrent());

    Span<uint8_t> inGprArgs(inRegsArgs, ExtArchTraits::GP_ARG_NUM_BYTES);
    Span<uint8_t> inFprArgs(inGprArgs.end(), ExtArchTraits::FP_ARG_NUM_BYTES);
    ArgReader argReader(inGprArgs, inFprArgs, inStackArgs);

    Span<uint8_t> outGprArgs(outArgs, ExtArchTraits::GP_ARG_NUM_BYTES);
    Span<uint8_t> outFprArgs(outGprArgs.end(), ExtArchTraits::FP_ARG_NUM_BYTES);
    auto outStackArgs = outFprArgs.end();
    ArgWriter argWriter(outGprArgs, outFprArgs, outStackArgs);

    argReader.Read<Method *>();  // Skip method

    if (UNLIKELY(method->GetNativePointer() == nullptr)) {
        ThrowUnresolvedMethodException(method, thread);
    }

    ARCH_COPY_METHOD_ARGS(method, argReader, argWriter);

    Runtime::GetCurrent()->GetNotificationManager()->MethodEntryEvent(thread, method);
}

//              Input stack              =======>             Output stack
// 0xFFFF
//       |                        |                    |                        |
//       |       Prev frame       |                    |       Prev frame       |
//       |          ...           |                    |          ...           |
//       +------------------------+                    +------------------------+
//       |          ...           |                    |          ...           |
//       |       stack args       |                    |       stack args       | <--------+
//       |          ...           |                    |          ...           |          |
//       +---+---+----------------+ <- inStackArgs --> +----------------+---+---+          |
//       |   |   |       LR       |                    |       LR       |   |   |          |
//       |   |   |       FP       |                    |       FP       |   |   |          |
//       |   |   |     Method *   |                    |     Method *   |   |   |          |
//       |   | c |      FLAGS     |                    |      FLAGS     | c |   |          |
//       |   | f +----------------+                    +----------------+ f |   |          |
//       |   | r |       ...      |                    |       ...      | r |   |          |
//       |   | a |     locals     |                    |     locals     | a |   |          |
//       |   | m |       ...      |                    |       ...      | m |   |          |
//       |   | e +----------------+                    +----------------+ e |   |          |
//       | N |   |       ...      |                    |       ...      |   | N |          |
//       | A |   |  callee saved  |                    |  callee saved  |   | A |          |
//       | P |   |       ...      |                    |       ...      |   | P |          |
//       | I +---+----------------+                    +----------------+---+ I |          |
//       |   |        ...         |                    |        ...         |   |          |
//       |   |     float args     |                    |     float args     |   |          |
//       | f |        ...         |                    |        ...         | f |          |
//       | r +--------------------+                    +--------------------+ r |          |
//       | a |        ...         |                    |        ...         | a |          |
//       | m |    general args    |                    |    general args    | m | <----+   |
//       | e |        ...         |                    |        ...         | e |      |   |
//       |   |    arg0|Method*    |                    |arg0|class/null(opt)|   |      |   |
//       |   +--------------------+ <-- inRegsArgs --> +--------------------+   |      |   |     References
//       |   |                    |                    |        ...         |   |      |   | to ObjectHeader *s
//       |   |                    |                    |  NAPI float args   |   |      |   |    on the stack
//       |   |                    |                    |     (on regs)      |   |      |   |
//       |   |                    |                    |        ...         |   |      |   |
//       |   |                    |                    +--------------------+   |      |   |
//       |   |                    |                    |        ...         |   |      |   |
//       |   |     space for      |                    | NAPI general args  |   | -----+   |
//       |   |     NAPI args      |                    |     (on regs)      |   |          |
//       |   |                    |                    |        ...         |   |          |
//       |   |                    |                    +--------------------+   |          |
//       |   |                    |                    |        ...         |   |          |
//       |   |                    |                    |     NAPI args      |   | ---------+
//       |   |                    |                    |     (on stack)     |   |
//       |   |                    |                    |        ...         |   |
//       +---+--------------------+ <- outStackArgs -> +--------------------+---+
//       |                        |                    |                        |
// 0x0000
static uint8_t *PrepareArgsOnStack(Method *method, uint8_t *inRegsArgs, uint8_t *inStackArgs, uint8_t *outStackArgs,
                                   ani_env *pandaEnv)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto outRegsArgs = inRegsArgs - ExtArchTraits ::FP_ARG_NUM_BYTES - ExtArchTraits ::GP_ARG_NUM_BYTES;

    ASSERT(outStackArgs <= outRegsArgs);
    ASSERT(outRegsArgs < inRegsArgs);
    ASSERT(inRegsArgs < inStackArgs);

    Span<uint8_t> inGprArgs(inRegsArgs, ExtArchTraits::GP_ARG_NUM_BYTES);
    Span<uint8_t> inFprArgs(inGprArgs.end(), ExtArchTraits::FP_ARG_NUM_BYTES);
    ArgReader argReader(inGprArgs, inFprArgs, inStackArgs);

    Span<uint8_t> outGprArgs(outRegsArgs, ExtArchTraits::GP_ARG_NUM_BYTES);
    Span<uint8_t> outFprArgs(outGprArgs.end(), ExtArchTraits::FP_ARG_NUM_BYTES);
    ArgWriter argWriter(outGprArgs, outFprArgs, outStackArgs);

    argReader.Read<Method *>();  // Skip method

    EtsMethod *etsMethod = EtsMethod::FromRuntimeMethod(method);
    EtsReference *classOrThisRef = nullptr;
    if (IsFunction(etsMethod)) {
        // NOTE:
        //  Replace the method pointer (Method *) with a pointer to the nullptr
        //  to avoid GC crash during traversal of method arguments.
        auto classPtr = reinterpret_cast<EtsObject **>(inRegsArgs);
        *classPtr = nullptr;
    } else if (IsStaticMethod(etsMethod)) {
        // Handle class object
        auto classObj = EtsClass::FromRuntimeClass(method->GetClass())->AsObject();
        ASSERT(classObj != nullptr);

        // Replace the method pointer (Method *) with a pointer to the class object
        auto classPtr = reinterpret_cast<EtsObject **>(inRegsArgs);
        *classPtr = classObj;

        classOrThisRef = EtsReferenceStorage::NewEtsStackRef(classPtr);
    } else {
        ASSERT(method->GetNumArgs() != 0);
        ASSERT(!method->GetArgType(0).IsPrimitive());
        ASSERT(!IsFunction(etsMethod));

        // Handle this arg
        auto thisPtr = const_cast<EtsObject **>(argReader.ReadPtr<EtsObject *>());
        classOrThisRef = EtsReferenceStorage::NewEtsStackRef(thisPtr);
    }

    // Prepare ANI args
    argWriter.Write(static_cast<ani_env *>(pandaEnv));
    if (!IsFunction(etsMethod)) {
        argWriter.Write(classOrThisRef);
    }
    ARCH_COPY_METHOD_ARGS(method, argReader, argWriter);
    // Completed the preparation of ANI arguments on the stack

    return outRegsArgs;
}

extern "C" uint8_t *AniBegin(Method *method, uint8_t *inRegsArgs, uint8_t *inStackArgs, uint8_t *outStackArgs,
                             ManagedThread *thread)
{
    ASSERT(!method->IsSynchronized());
    ASSERT(thread == ManagedThread::GetCurrent());

    PandaAniEnv *pandaEnv = EtsExecutionContext::FromMT(thread)->GetPandaAniEnv();
    ani_env *argEnv = pandaEnv->IsVerifyANI() ? static_cast<ani_env *>(pandaEnv->GetEnvANIVerifier()->GetEnv())
                                              : static_cast<ani_env *>(pandaEnv);
    uint8_t *outRegsArgs = PrepareArgsOnStack(method, inRegsArgs, inStackArgs, outStackArgs, argEnv);

    // ATTENTION!!!
    // Don't move the following code above, because only from this point on,
    // the stack of the current frame is correct, so we can safely walk it
    // (e.g. stop at a safepoint and walk the stack args of NAPI frame)

    if (UNLIKELY(method->GetNativePointer() == nullptr)) {
        ThrowUnresolvedMethodException(method, thread);
    }

    Runtime::GetCurrent()->GetNotificationManager()->MethodEntryEvent(thread, method);

    // CC-OFFNXT(G.NAM.03) project code style
    constexpr uint32_t MAX_LOCAL_REF = 4096;
    if (UNLIKELY(!pandaEnv->GetEtsReferenceStorage()->PushLocalEtsFrame(MAX_LOCAL_REF))) {
        LOG(FATAL, RUNTIME) << "eTS NAPI push local frame failed";
    }

    EtsMethod *etsMethod = EtsMethod::FromRuntimeMethod(method);
    if (!etsMethod->IsFastNative()) {
        thread->NativeCodeBegin();
    }

    return outRegsArgs;
}

extern "C" uint8_t *AniBeginVerify(Method *method, uint8_t *inRegsArgs, uint8_t *inStackArgs, uint8_t *outStackArgs,
                                   ManagedThread *thread)
{
    auto *executionCtx = EtsExecutionContext::FromMT(thread);
    auto *pandaEnv = executionCtx->GetPandaAniEnv();
    auto envANIVerifier = pandaEnv->GetEnvANIVerifier();
    envANIVerifier->PushNativeFrame(pandaEnv);
    return AniBegin(method, inRegsArgs, inStackArgs, outStackArgs, thread);
}

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

extern "C" void AniEnd(Method *method, ManagedThread *thread, bool isFastNative)
{
    ASSERT(method != nullptr);
    ASSERT(!method->IsSynchronized());
    ASSERT(thread == ManagedThread::GetCurrent());

    if (!isFastNative) {
        thread->NativeCodeEnd();
    }

    Runtime::GetCurrent()->GetNotificationManager()->MethodExitEvent(thread, method);

    auto storage = EtsExecutionContext::FromMT(thread)->GetPandaAniEnv()->GetEtsReferenceStorage();
    storage->PopLocalEtsFrame(EtsReference::GetUndefined());
}

extern "C" void AniEndVerify(Method *method, ManagedThread *thread, bool isFastNative)
{
    auto executionCtx = EtsExecutionContext::FromMT(thread);

    // Pop native frame verification before ending the native method
    auto envANIVerifier = executionCtx->GetPandaAniEnv()->GetEnvANIVerifier();
    auto err = envANIVerifier->PopNativeFrame();
    if (err) {
        LOG(FATAL, ANI) << "Error during popping ANI native frame: " << err.value();
    }

    AniEnd(method, thread, isFastNative);
}

extern "C" EtsObject *AniObjEnd(Method *method, EtsReference *etsRef, ManagedThread *thread, bool isFastNative)
{
    ASSERT(method != nullptr);
    ASSERT(!method->IsSynchronized());
    ASSERT(thread == ManagedThread::GetCurrent());

    // End native scope first to get into managed scope for object manipulation
    if (!isFastNative) {
        thread->NativeCodeEnd();
    }

    Runtime::GetCurrent()->GetNotificationManager()->MethodExitEvent(thread, method);

    auto storage = EtsExecutionContext::FromMT(thread)->GetPandaAniEnv()->GetEtsReferenceStorage();
    EtsObject *ret = nullptr;
    if (LIKELY(!thread->HasPendingException())) {
        ret = storage->GetEtsObject(etsRef);
    }

    storage->PopLocalEtsFrame(EtsReference::GetUndefined());

    return ret;
}

extern "C" EtsObject *AniObjEndVerify(Method *method, verify::VRef *vref, ManagedThread *thread, bool isFastNative)
{
    auto *executionCtx = EtsExecutionContext::FromMT(thread);
    ani_ref ref = vref->GetRef();
    auto etsRef = reinterpret_cast<EtsReference *>(ref);

    EtsObject *obj = AniObjEnd(method, etsRef, thread, isFastNative);
    // Pop native frame verification before ending the native method
    auto envANIVerifier = executionCtx->GetPandaAniEnv()->GetEnvANIVerifier();
    auto err = envANIVerifier->PopNativeFrame();
    if (err) {
        LOG(FATAL, ANI) << "Error during popping ANI native frame: " << err.value();
    }
    return obj;
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
// CC-OFFNXT(huge_method[C++], G.FUN.01) solid logic
extern "C" ObjectPointerType EtsAsyncCall(Method *method, EtsCoroutine *currentCoro, uint8_t *regArgs,
                                          uint8_t *stackArgs)
{
    EtsExecutionContext *executionCtx = EtsExecutionContext::FromMT(currentCoro);
    PandaEtsVM *vm = executionCtx->GetPandaVM();
    auto *jobMan = vm->GetJobManager();
    Method *impl = vm->GetClassLinker()->GetAsyncImplMethod(method, executionCtx);
    if (impl == nullptr) {
        ASSERT(currentCoro->HasPendingException());
        // Exception is thrown by GetAsyncImplMethod
        return 0;
    }
    ASSERT(!currentCoro->HasPendingException());
    if (jobMan->IsJobSwitchDisabled()) {
        ThrowEtsException(executionCtx, PlatformTypes(executionCtx)->coreInvalidJobOperationError,
                          "Cannot call async in the current context!");
        return 0;
    }

    PandaVector<Value> args;
    args.reserve(method->GetNumArgs());
    Span<uint8_t> gprArgs(regArgs, ExtArchTraits::GP_ARG_NUM_BYTES);
    Span<uint8_t> fprArgs(gprArgs.end(), ExtArchTraits::FP_ARG_NUM_BYTES);
    ArgReader argReader(gprArgs, fprArgs, stackArgs);
    argReader.Read<Method *>();  // Skip method
    if (method->IsStatic()) {
        // Replace the method pointer (Method *) by the pointer to the object class
        // to satisfy stack walker
        auto classObj = EtsClass::FromRuntimeClass(method->GetClass())->AsObject();
        ASSERT(classObj != nullptr);
        auto classPtr = reinterpret_cast<EtsObject **>(regArgs);
        *classPtr = classObj;
    }

    // Create object after arg fix ^^^.
    // Arg fix is needed for StackWalker. So if GC gets triggered in EtsPromise::Create
    // it StackWalker correctly finds all vregs.
    EtsPromise *promise = EtsPromise::Create(executionCtx);
    if (UNLIKELY(promise == nullptr)) {
        ThrowOutOfMemoryError(currentCoro, "Cannot allocate Promise");
        return 0;
    }
    auto promiseRef = vm->GetGlobalObjectStorage()->Add(promise->GetCoreType(), mem::Reference::ObjectType::GLOBAL);
    auto evt = Runtime::GetCurrent()->GetInternalAllocator()->New<CompletionEvent>(promiseRef, jobMan);

    // Read values from stack and keep in args values for Launch after possible GC in EtsPromise::Create
    if (!method->IsStatic()) {
        // Handle this arg
        ASSERT(method->GetNumArgs() != 0 && !method->GetArgType(0).IsPrimitive());
        args.push_back(Value(*const_cast<ObjectHeader **>(argReader.ReadPtr<ObjectHeader *>())));
    }
    arch::ValueWriter writer(&args);
    ARCH_COPY_METHOD_ARGS(method, argReader, writer);

    [[maybe_unused]] EtsHandleScope scope(executionCtx);
    EtsHandle<EtsPromise> promiseHandle(executionCtx, promise);

    auto epInfo = Job::ManagedEntrypointInfo {evt, impl, std::move(args)};
    auto *job = jobMan->CreateJob(impl->GetFullName(), std::move(epInfo), EtsCoroutine::ASYNC_CALL);
    LaunchResult launchResult = jobMan->Launch(job, LaunchParams {true});
    if (UNLIKELY(launchResult != LaunchResult::OK)) {
        ASSERT(currentCoro->HasPendingException());
        // OOM is thrown by Launch
        if (launchResult == LaunchResult::RESOURCE_LIMIT_EXCEED) {
            jobMan->DestroyJob(job);
        }
        vm->GetGlobalObjectStorage()->Remove(promiseRef);
        return 0;
    }
    return ToObjPtr(promiseHandle.GetPtr());
}
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}  // namespace ark::ets::ani
