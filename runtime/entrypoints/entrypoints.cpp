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

#include "runtime/entrypoints/entrypoints.h"

#include "libpandabase/events/events.h"
#include "macros.h"
#include "compiler/compiler_options.h"
#include "runtime/deoptimization.h"
#include "runtime/arch/memory_helpers.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/method-inl.h"
#include "runtime/include/object_header-inl.h"
#include "runtime/include/runtime.h"
#include "runtime/include/value-inl.h"
#include "runtime/include/panda_vm.h"
#include "runtime/interpreter/frame.h"
#include "runtime/interpreter/interpreter.h"
#include "runtime/interpreter/runtime_interface.h"
#include "runtime/mem/tlab.h"
#include "compiler/optimizer/ir/runtime_interface.h"
#include "runtime/handle_base-inl.h"
#include "libpandabase/utils/asan_interface.h"
#include "libpandabase/utils/tsan_interface.h"
#include "utils/cframe_layout.h"
#include "intrinsics.h"

namespace panda {

using panda::compiler::TraceId;

#undef LOG_ENTRYPOINTS

class ScopedLog {
public:
    ScopedLog() = delete;
    explicit ScopedLog(const char *function) : function_(function)
    {
        LOG(DEBUG, INTEROP) << "ENTRYPOINT: " << function;
    }
    ~ScopedLog()
    {
        LOG(DEBUG, INTEROP) << "EXIT ENTRYPOINT: " << function_;
    }
    NO_COPY_SEMANTIC(ScopedLog);
    NO_MOVE_SEMANTIC(ScopedLog);

private:
    std::string function_;
};

#ifdef LOG_ENTRYPOINTS
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_ENTRYPOINT() ScopedLog __log(__FUNCTION__)
#else
#define LOG_ENTRYPOINT()
#endif

// enable stack walker dry run on each entrypoint to discover stack issues early
#ifndef NDEBUG
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CHECK_STACK_WALKER                                         \
    if (Runtime::GetOptions().IsVerifyEntrypoints()) {             \
        StackWalker::Create(ManagedThread::GetCurrent()).Verify(); \
    }
#else
#define CHECK_STACK_WALKER
#endif

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define BEGIN_ENTRYPOINT() \
    CHECK_STACK_WALKER;    \
    LOG_ENTRYPOINT();

static void HandlePendingException(UnwindPolicy policy = UnwindPolicy::ALL)
{
    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread->HasPendingException());
    LOG(DEBUG, INTEROP) << "HandlePendingException";

    auto stack = StackWalker::Create(thread, policy);
    ASSERT(stack.IsCFrame());

    FindCatchBlockInCFrames(thread, &stack, nullptr);
}

extern "C" int HasCompiledCode(Method *method)
{
    return method->HasCompiledCode() ? 1 : 0;
}

extern "C" bool IncrementHotnessCounter(Method *method)
{
    method->IncrementHotnessCounter(0, nullptr);
    return method->GetCompiledEntryPoint() != GetCompiledCodeToInterpreterBridge(method);
}

extern "C" NO_ADDRESS_SANITIZE void InterpreterEntryPoint(Method *method, Frame *frame)
{
    auto pc = method->GetInstructions();
    Method *callee = frame->GetMethod();
    ASSERT(callee != nullptr);

    if (callee->IsAbstract()) {
        ASSERT(pc == nullptr);
        panda::ThrowAbstractMethodError(callee);
        HandlePendingException();
        UNREACHABLE();
    }

    ManagedThread *thread = ManagedThread::GetCurrent();
    if (!thread->template StackOverflowCheck<true, false>()) {
        HandlePendingException(UnwindPolicy::SKIP_INLINED);
        UNREACHABLE();
    }

    Frame *prev_frame = thread->GetCurrentFrame();
    thread->SetCurrentFrame(frame);

    auto is_compiled_code = thread->IsCurrentFrameCompiled();
    thread->SetCurrentFrameIsCompiled(false);
    interpreter::Execute(thread, pc, frame);
    thread->SetCurrentFrameIsCompiled(is_compiled_code);
    if (prev_frame != nullptr && reinterpret_cast<uintptr_t>(prev_frame->GetMethod()) == COMPILED_CODE_TO_INTERPRETER) {
        thread->SetCurrentFrame(prev_frame->GetPrevFrame());
    } else {
        thread->SetCurrentFrame(prev_frame);
    }
}

extern "C" void AnnotateSanitizersEntrypoint([[maybe_unused]] void const *addr, [[maybe_unused]] size_t size)
{
#ifdef PANDA_TSAN_ON
    TSAN_ANNOTATE_HAPPENS_BEFORE(const_cast<void *>(addr));
#endif
#ifdef PANDA_ASAN_ON
    __asan_unpoison_memory_region(addr, size);
#endif
}

extern "C" void WriteTlabStatsEntrypoint(size_t size)
{
    LOG_ENTRYPOINT();

    ASSERT(size <= mem::PANDA_TLAB_SIZE);
    if (mem::PANDA_TRACK_TLAB_ALLOCATIONS) {
        auto mem_stats = Thread::GetCurrent()->GetVM()->GetHeapManager()->GetMemStats();
        if (mem_stats == nullptr) {
            return;
        }
        mem_stats->RecordAllocateObject(size, SpaceType::SPACE_TYPE_OBJECT);
    }
}

extern "C" size_t GetClassIdEntrypoint(const Method *caller, uint32_t class_id)
{
    BEGIN_ENTRYPOINT();
    auto resolved_id = caller->GetClass()->ResolveClassIndex(BytecodeId(class_id).AsIndex());
    return resolved_id.GetOffset();
}

extern "C" coretypes::Array *CreateArrayByIdEntrypoint(const Method *caller, uint32_t class_id, size_t length)
{
    BEGIN_ENTRYPOINT();
    size_t resolved_id = GetClassIdEntrypoint(caller, class_id);
    auto *klass = reinterpret_cast<Class *>(ResolveClassEntrypoint(caller, resolved_id));
    return CreateArraySlowPathEntrypoint(klass, length);
}

extern "C" coretypes::Array *CreateArraySlowPathEntrypoint(Class *klass, size_t length)
{
    BEGIN_ENTRYPOINT();

    TSAN_ANNOTATE_HAPPENS_AFTER(klass);
    auto arr = coretypes::Array::Create(klass, length);
    if (UNLIKELY(arr == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    if (compiler::options.IsCompilerEnableTlabEvents()) {
        EVENT_SLOWPATH_ALLOC(ManagedThread::GetCurrent()->GetId());
    }
    return arr;
}

extern "C" coretypes::Array *CreateMultiArrayRecEntrypoint(ManagedThread *thread, Class *klass, uint32_t nargs,
                                                           size_t *sizes, uint32_t num)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic,-warnings-as-errors)
    auto arr_size = sizes[num];

    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    // SUPPRESS_CSA_NEXTLINE(alpha.core.CheckObjHeaderTypeRef)
    VMHandle<coretypes::Array> handle(thread, coretypes::Array::Create(klass, arr_size));
    if (handle.GetPtr() == nullptr) {
        return nullptr;
    }
    auto *component = klass->GetComponentType();

    if (component->IsArrayClass() && num + 1 < nargs) {
        for (size_t idx = 0; idx < arr_size; idx++) {
            auto *array = CreateMultiArrayRecEntrypoint(thread, component, nargs, sizes, num + 1);

            if (array == nullptr) {
                return nullptr;
            }
            handle.GetPtr()->template Set<coretypes::Array *>(idx, array);
        }
    }

    return handle.GetPtr();
}

extern "C" coretypes::Array *ResolveLiteralArrayEntrypoint(const Method *caller, uint32_t type_id)
{
    BEGIN_ENTRYPOINT();

    auto arr = Runtime::GetCurrent()->ResolveLiteralArray(ManagedThread::GetCurrent()->GetVM(), *caller, type_id);
    if (UNLIKELY(arr == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    return arr;
}

extern "C" coretypes::Array *CreateMultiArrayEntrypoint(Class *klass, uint32_t nargs, size_t *sizes)
{
    BEGIN_ENTRYPOINT();

    auto arr = CreateMultiArrayRecEntrypoint(ManagedThread::GetCurrent(), klass, nargs, sizes, 0);
    if (UNLIKELY(arr == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    return arr;
}

extern "C" ObjectHeader *CreateObjectByClassInterpreter(ManagedThread *thread, Class *klass)
{
    if (!klass->IsInitialized()) {
        auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
        if (!class_linker->InitializeClass(thread, klass)) {
            return nullptr;
        }
    }
    return interpreter::RuntimeInterface::CreateObject(klass);
}

extern "C" ObjectHeader *CreateObjectByIdEntrypoint(ManagedThread *thread, const Method *caller, uint32_t type_id)
{
    BEGIN_ENTRYPOINT();
    size_t resolved_id = GetClassIdEntrypoint(caller, type_id);
    auto *klass = reinterpret_cast<Class *>(ResolveClassEntrypoint(caller, resolved_id));
    return CreateObjectByClassInterpreter(thread, klass);
}

extern "C" void DebugPrintEntrypoint([[maybe_unused]] panda::Frame *frame, [[maybe_unused]] const uint8_t *pc)
{
    BEGIN_ENTRYPOINT();
#ifndef NDEBUG
    if (!Logger::IsLoggingOn(Logger::Level::DEBUG, Logger::Component::INTERPRETER)) {
        return;
    }

    constexpr uint64_t standardDebugIndent = 5;
    PandaString acc_dump;
    if (frame->IsDynamic()) {
        acc_dump = frame->template GetAccAsVReg<true>().DumpVReg();
        LOG(DEBUG, INTERPRETER) << PandaString(standardDebugIndent, ' ') << "acc." << acc_dump;

        DynamicFrameHandler frame_handler(frame);
        for (size_t i = 0; i < frame->GetSize(); ++i) {
            LOG(DEBUG, INTERPRETER) << PandaString(standardDebugIndent, ' ') << "v" << i << "."
                                    << frame_handler.GetVReg(i).DumpVReg();
        }
    } else {
        acc_dump = frame->GetAccAsVReg().DumpVReg();
        LOG(DEBUG, INTERPRETER) << PandaString(standardDebugIndent, ' ') << "acc." << acc_dump;

        StaticFrameHandler frame_handler(frame);
        for (size_t i = 0; i < frame->GetSize(); ++i) {
            LOG(DEBUG, INTERPRETER) << PandaString(standardDebugIndent, ' ') << "v" << i << "."
                                    << frame_handler.GetVReg(i).DumpVReg();
        }
    }
    LOG(DEBUG, INTERPRETER) << " pc: " << (void *)pc << " ---> " << BytecodeInstruction(pc);
#endif
}

extern "C" ObjectHeader *CreateObjectByClassEntrypoint(Class *klass)
{
    BEGIN_ENTRYPOINT();

    // We need annotation here for the FullMemoryBarrier used in InitializeClassByIdEntrypoint
    TSAN_ANNOTATE_HAPPENS_AFTER(klass);
    if (klass->IsStringClass()) {
        LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*klass);
        auto str = coretypes::String::CreateEmptyString(ctx, Runtime::GetCurrent()->GetPandaVM());
        if (UNLIKELY(str == nullptr)) {
            HandlePendingException();
            UNREACHABLE();
        }
        return str;
    }

    if (LIKELY(klass->IsInstantiable())) {
        auto obj = ObjectHeader::Create(klass);
        if (UNLIKELY(obj == nullptr)) {
            HandlePendingException();
            UNREACHABLE();
        }
        if (compiler::options.IsCompilerEnableTlabEvents()) {
            EVENT_SLOWPATH_ALLOC(ManagedThread::GetCurrent()->GetId());
        }
        return obj;
    }

    ThrowInstantiationErrorEntrypoint(klass);
    UNREACHABLE();
}

extern "C" ObjectHeader *CloneObjectEntrypoint(ObjectHeader *obj)
{
    BEGIN_ENTRYPOINT();

    uint32_t flags = obj->ClassAddr<Class>()->GetFlags();  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
    if (UNLIKELY((flags & Class::IS_CLONEABLE) == 0)) {
        ASSERT(!ManagedThread::GetCurrent()->HasPendingException());
        ThrowCloneNotSupportedException();
        HandlePendingException(UnwindPolicy::SKIP_INLINED);
        return nullptr;
    }
    return ObjectHeader::Clone(obj);  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
}

extern "C" ObjectHeader *PostBarrierWriteEntrypoint(ObjectHeader *obj, size_t size)
{
    LOG_ENTRYPOINT();
    AnnotateSanitizersEntrypoint(obj, size);
    auto object_class = obj->ClassAddr<Class>();
    auto *barrier_set = ManagedThread::GetCurrent()->GetBarrierSet();
    if (object_class->IsArrayClass()) {
        if (object_class->IsObjectArrayClass()) {
            barrier_set->PostBarrierArrayWrite(obj, size);
        }
    } else {
        barrier_set->PostBarrierEveryObjectFieldWrite(obj, size);
    }
    return obj;
}

extern "C" void CheckCastByBCIDEntrypoint(const Method *caller, ObjectHeader *obj, uint32_t type_id)
{
    BEGIN_ENTRYPOINT();
    auto thread = ManagedThread::GetCurrent();
    VMHandle<ObjectHeader> handle_obj(thread, obj);  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
    size_t resolved_id = GetClassIdEntrypoint(caller, type_id);
    auto klass = reinterpret_cast<Class *>(ResolveClassEntrypoint(caller, resolved_id));

    return CheckCastEntrypoint(handle_obj.GetPtr(), klass);
}

extern "C" void CheckCastEntrypoint(const ObjectHeader *obj, Class *klass)
{
    BEGIN_ENTRYPOINT();

    // Don't use obj after ClassLinker call because GC can move it.
    // Since we need only class and class in a non-movalble object
    // it is ok to get it here.
    Class *obj_klass = obj == nullptr ? nullptr : obj->ClassAddr<Class>();  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
    if (UNLIKELY(obj_klass != nullptr && !klass->IsAssignableFrom(obj_klass))) {
        panda::ThrowClassCastException(klass, obj_klass);
        HandlePendingException();
        UNREACHABLE();
    }
}

extern "C" uint8_t IsInstanceEntrypoint(ObjectHeader *obj, Class *klass)
{
    BEGIN_ENTRYPOINT();

    // Don't use obj after ClassLinker call because GC can move it.
    // Since we need only class and class in a non-movalble object
    // it is ok to get it here.
    Class *obj_klass = obj == nullptr ? nullptr : obj->ClassAddr<Class>();  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
    if (UNLIKELY(obj_klass != nullptr && klass->IsAssignableFrom(obj_klass))) {
        return 1;
    }
    return 0;
}

extern "C" uint8_t IsInstanceByBCIDEntrypoint(const Method *caller, ObjectHeader *obj, uint32_t type_id)
{
    BEGIN_ENTRYPOINT();
    auto thread = ManagedThread::GetCurrent();
    VMHandle<ObjectHeader> handle_obj(thread, obj);  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
    size_t resolved_id = GetClassIdEntrypoint(caller, type_id);
    auto klass = reinterpret_cast<Class *>(ResolveClassEntrypoint(caller, resolved_id));

    return IsInstanceEntrypoint(handle_obj.GetPtr(), klass);
}

extern "C" void SafepointEntrypoint()
{
    BEGIN_ENTRYPOINT();
    interpreter::RuntimeInterface::Safepoint();
}

extern "C" void *ResolveClassObjectEntrypoint(const Method *caller, FileEntityId type_id)
{
    BEGIN_ENTRYPOINT();
    auto klass = reinterpret_cast<Class *>(ResolveClassEntrypoint(caller, type_id));
    return reinterpret_cast<void *>(klass->GetManagedObject());
}

extern "C" void *ResolveClassEntrypoint(const Method *caller, FileEntityId type_id)
{
    BEGIN_ENTRYPOINT();
    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    Class *klass = class_linker->GetClass(*caller, panda_file::File::EntityId(type_id));
    if (UNLIKELY(klass == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    return reinterpret_cast<void *>(klass);
}

extern "C" size_t GetFieldIdEntrypoint(const Method *caller, uint32_t field_id)
{
    BEGIN_ENTRYPOINT();
    auto resolved_id = caller->GetClass()->ResolveFieldIndex(BytecodeId(field_id).AsIndex());
    return resolved_id.GetOffset();
}

extern "C" coretypes::String *ResolveStringEntrypoint(const Method *caller, FileEntityId id)
{
    BEGIN_ENTRYPOINT();
    return Runtime::GetCurrent()->ResolveStringFromCompiledCode(ManagedThread::GetCurrent()->GetVM(), *caller,
                                                                panda_file::File::EntityId(id));
}

extern "C" coretypes::String *ResolveStringAotEntrypoint(const Method *caller, FileEntityId id, ObjectHeader **slot)
{
    BEGIN_ENTRYPOINT();
    auto runtime = Runtime::GetCurrent();
    auto aot_manager = runtime->GetClassLinker()->GetAotManager();
    auto vm = ManagedThread::GetCurrent()->GetVM();
    auto str = runtime->ResolveStringFromCompiledCode(vm, *caller, panda::panda_file::File::EntityId(id));
    if (UNLIKELY(str == nullptr)) {
        return nullptr;
    }

    // to many strings were saved to slots, so simply return the resolved string
    if (aot_manager->GetAotStringRootsCount() >= Runtime::GetOptions().GetAotStringGcRootsLimit()) {
        return str;
    }

    auto counter = reinterpret_cast<std::atomic_uint32_t *>(slot);

    // Atomic with acquire order reason: data race with slot with dependecies on reads after the load which should
    // become visible
    auto counter_val = counter->load(std::memory_order_acquire);
    if (counter_val >= compiler::RuntimeInterface::RESOLVE_STRING_AOT_COUNTER_LIMIT - 1) {
        return str;
    }

    if (counter_val < Runtime::GetOptions().GetResolveStringAotThreshold()) {
        // try to update counter, but ignore result - in the worst case we'll save
        // string's pointer to slot a bit later
        counter->compare_exchange_strong(counter_val, counter_val + 1, std::memory_order_release,
                                         std::memory_order_relaxed);
    } else {
        // try to replace the counter with string pointer and register the slot as GC root in case of success
        if (counter->compare_exchange_strong(counter_val, static_cast<uint32_t>(reinterpret_cast<uint64_t>(str)),
                                             std::memory_order_release, std::memory_order_relaxed)) {
            auto allocator = vm->GetHeapManager()->GetObjectAllocator().AsObjectAllocator();
            bool is_young =
                allocator->HasYoungSpace() && allocator->IsAddressInYoungSpace(reinterpret_cast<uintptr_t>(str));
            aot_manager->RegisterAotStringRoot(slot, is_young);
            EVENT_AOT_RESOLVE_STRING(ConvertToString(str));
        }
    }

    return str;
}

extern "C" Frame *CreateFrameWithSize(uint32_t size, uint32_t nregs, Method *method, Frame *prev)
{
    uint32_t ext_sz = EmptyExtFrameDataSize;
    if (LIKELY(method)) {
        ext_sz = Runtime::GetCurrent()->GetLanguageContext(*method).GetFrameExtSize();
    }
    size_t alloc_sz = Frame::GetAllocSize(size, ext_sz);
    Frame *frame = Thread::GetCurrent()->GetVM()->GetHeapManager()->AllocateExtFrame(alloc_sz, ext_sz);
    if (UNLIKELY(frame == nullptr)) {
        return nullptr;
    }
    return new (frame) Frame(Frame::ToExt(frame, ext_sz), method, prev, nregs);
}

extern "C" Frame *CreateFrameWithActualArgsAndSize(uint32_t size, uint32_t nregs, uint32_t num_actual_args,
                                                   Method *method, Frame *prev)
{
    auto *thread = ManagedThread::GetCurrent();
    uint32_t ext_sz = thread->GetVM()->GetFrameExtSize();
    size_t alloc_sz = Frame::GetAllocSize(size, ext_sz);
    void *mem = thread->GetStackFrameAllocator()->Alloc(alloc_sz);
    if (UNLIKELY(mem == nullptr)) {
        return nullptr;
    }
    return new (Frame::FromExt(mem, ext_sz)) Frame(mem, method, prev, nregs, num_actual_args);
}

extern "C" Frame *CreateNativeFrameWithActualArgsAndSize(uint32_t size, uint32_t nregs, uint32_t num_actual_args,
                                                         Method *method, Frame *prev)
{
    uint32_t ext_sz = EmptyExtFrameDataSize;
    size_t alloc_sz = Frame::GetAllocSize(size, ext_sz);
    void *mem = ManagedThread::GetCurrent()->GetStackFrameAllocator()->Alloc(alloc_sz);
    if (UNLIKELY(mem == nullptr)) {
        return nullptr;
    }
    return new (Frame::FromExt(mem, ext_sz)) Frame(mem, method, prev, nregs, num_actual_args);
}

template <bool is_dynamic = false>
static Frame *CreateFrame(uint32_t nregs, Method *method, Frame *prev)
{
    return CreateFrameWithSize(Frame::GetActualSize<is_dynamic>(nregs), nregs, method, prev);
}

template <bool is_dynamic>
static Frame *CreateFrameWithActualArgs(uint32_t nregs, uint32_t num_actual_args, Method *method, Frame *prev)
{
    return CreateFrameWithActualArgsAndSize(Frame::GetActualSize<is_dynamic>(nregs), nregs, num_actual_args, method,
                                            prev);
}

extern "C" Frame *CreateFrameForMethod(Method *method, Frame *prev)
{
    auto nregs = method->GetNumArgs() + method->GetNumVregs();
    return CreateFrame<false>(nregs, method, prev);
}

extern "C" Frame *CreateFrameForMethodDyn(Method *method, Frame *prev)
{
    auto nregs = method->GetNumArgs() + method->GetNumVregs();
    return CreateFrame<true>(nregs, method, prev);
}

extern "C" Frame *CreateFrameForMethodWithActualArgs(uint32_t num_actual_args, Method *method, Frame *prev)
{
    auto nargs = std::max(num_actual_args, method->GetNumArgs());
    auto nregs = nargs + method->GetNumVregs();
    return CreateFrameWithActualArgs<false>(nregs, num_actual_args, method, prev);
}

extern "C" Frame *CreateFrameForMethodWithActualArgsDyn(uint32_t num_actual_args, Method *method, Frame *prev)
{
    auto nargs = std::max(num_actual_args, method->GetNumArgs());
    auto nregs = nargs + method->GetNumVregs();
    return CreateFrameWithActualArgs<true>(nregs, num_actual_args, method, prev);
}

extern "C" void FreeFrame(Frame *frame)
{
    ASSERT(frame->GetExt() != nullptr);
    ManagedThread::GetCurrent()->GetStackFrameAllocator()->Free(frame->GetExt());
}

extern "C" uintptr_t GetStaticFieldAddressEntrypoint(Method *method, uint32_t field_id)
{
    BEGIN_ENTRYPOINT();
    auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
    auto field = class_linker->GetField(*method, panda_file::File::EntityId(field_id));
    if (UNLIKELY(field == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    auto *klass = field->GetClass();
    ASSERT(klass != nullptr);
    return reinterpret_cast<uintptr_t>(klass) + field->GetOffset();
}

extern "C" uintptr_t GetUnknownStaticFieldMemoryAddressEntrypoint(Method *method, uint32_t field_id, size_t *slot)
{
    BEGIN_ENTRYPOINT();
    auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
    auto field = class_linker->GetField(*method, panda_file::File::EntityId(field_id));
    if (UNLIKELY(field == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    auto *klass = field->GetClass();
    ASSERT(klass != nullptr);
    InitializeClassEntrypoint(klass);

    uintptr_t addr = reinterpret_cast<uintptr_t>(klass) + field->GetOffset();
    if (klass->IsInitialized() && slot != nullptr) {
        *slot = addr;
    }
    return addr;
}

extern "C" uintptr_t GetUnknownStaticFieldPtrEntrypoint(Method *method, uint32_t field_id, size_t *slot)
{
    BEGIN_ENTRYPOINT();
    auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
    auto field = class_linker->GetField(*method, panda_file::File::EntityId(field_id));
    if (UNLIKELY(field == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    auto *klass = field->GetClass();
    ASSERT(klass != nullptr);
    InitializeClassEntrypoint(klass);

    auto addr = reinterpret_cast<uintptr_t>(field);
    if (klass->IsInitialized() && slot != nullptr) {
        *slot = addr;
    }
    return addr;
}

extern "C" size_t GetFieldOffsetByIdEntrypoint(Method *caller, uint32_t field_id)
{
    BEGIN_ENTRYPOINT();
    size_t resolved_id = GetFieldIdEntrypoint(caller, field_id);
    return GetFieldOffsetEntrypoint(caller, resolved_id);
}

extern "C" uintptr_t GetStaticFieldAddressByIdEntrypoint(ManagedThread *thread, Method *caller, uint32_t field_id)
{
    BEGIN_ENTRYPOINT();
    auto *field = interpreter::RuntimeInterface::ResolveField(thread, *caller, BytecodeId(field_id));
    return reinterpret_cast<uintptr_t>(field->GetClass()) + field->GetOffset();
}

extern "C" size_t GetFieldOffsetEntrypoint(Method *method, uint32_t field_id)
{
    BEGIN_ENTRYPOINT();
    auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
    auto field = class_linker->GetField(*method, panda_file::File::EntityId(field_id));
    if (UNLIKELY(field == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    return field->GetOffset();
}

extern "C" void InitializeClassEntrypoint(Class *klass)
{
    BEGIN_ENTRYPOINT();
    auto *class_linker = Runtime::GetCurrent()->GetClassLinker();
    if (!klass->IsInitialized() && !class_linker->InitializeClass(ManagedThread::GetCurrent(), klass)) {
        HandlePendingException();
        UNREACHABLE();
    }
}

extern "C" Class *InitializeClassByIdEntrypoint(const Method *caller, FileEntityId id)
{
    BEGIN_ENTRYPOINT();
    ClassLinker *class_linker = Runtime::GetCurrent()->GetClassLinker();
    Class *klass = class_linker->GetClass(*caller, panda_file::File::EntityId(id));
    if (UNLIKELY(klass == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    // Later we store klass pointer into .aot_got section.
    // Without full memory barrier on the architectures with weak memory order
    // we can read klass pointer, but fetch klass data before it's set in GetClass and InitializeClass
    arch::FullMemoryBarrier();
    // Full barrier is not visible by TSAN so we need annotation here
    TSAN_ANNOTATE_HAPPENS_BEFORE(klass);
    InitializeClassEntrypoint(klass);
    return klass;
}

extern "C" uintptr_t NO_ADDRESS_SANITIZE ResolveVirtualCallEntrypoint(const Method *callee, ObjectHeader *obj)
{
    BEGIN_ENTRYPOINT();
    if (UNLIKELY(callee == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    auto *resolved = obj->ClassAddr<Class>()->ResolveVirtualMethod(callee);  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
    ASSERT(resolved != nullptr);

    return reinterpret_cast<uintptr_t>(resolved);
}

extern "C" uintptr_t NO_ADDRESS_SANITIZE ResolveVirtualCallAotEntrypoint(const Method *caller, ObjectHeader *obj,
                                                                         size_t callee_id,
                                                                         [[maybe_unused]] uintptr_t cache_addr)
{
    BEGIN_ENTRYPOINT();
    // Don't use obj after ClassLinker call because GC can move it.
    // Since we need only class and class in a non-movalble object
    // it is ok to get it here.
    auto *obj_klass = obj->ClassAddr<Class>();  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
    Method *method = Runtime::GetCurrent()->GetClassLinker()->GetMethod(*caller, panda_file::File::EntityId(callee_id));
    if (UNLIKELY(method == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    auto *resolved = obj_klass->ResolveVirtualMethod(method);
    ASSERT(resolved != nullptr);

#if defined(PANDA_TARGET_ARM64)
    // In arm64, use interface inlineCache
    // TODO(liyiming): will support x86_64 in future
    // issue #7018
    auto method_head = obj_klass->GetRawFirstMethodAddr();
    if (cache_addr == 0 || method_head == nullptr) {
        return reinterpret_cast<uintptr_t>(resolved);
    }

    constexpr uint32_t METHOD_COMPRESS = 3;
    constexpr uint32_t CACHE_OFFSET_32 = 32;
    constexpr uint32_t CACHE_OFFSET_34 = 34;
    auto cache = reinterpret_cast<int64_t *>(cache_addr);
    auto method_resolved = reinterpret_cast<int64_t>(resolved);
    int64_t method_cache = method_resolved - reinterpret_cast<int64_t>(method_head);

    int64_t method_cache_judge = method_cache >> CACHE_OFFSET_34;  // NOLINT(hicpp-signed-bitwise)
    if (method_cache_judge != 0 && method_cache_judge != -1) {
        return reinterpret_cast<uintptr_t>(resolved);
    }
    method_cache = method_cache >> METHOD_COMPRESS;                            // NOLINT(hicpp-signed-bitwise)
    method_cache = method_cache << CACHE_OFFSET_32;                            // NOLINT(hicpp-signed-bitwise)
    int64_t save_cache = method_cache | reinterpret_cast<int64_t>(obj_klass);  // NOLINT(hicpp-signed-bitwise)
    *cache = save_cache;
#endif
    return reinterpret_cast<uintptr_t>(resolved);
}

extern "C" uintptr_t NO_ADDRESS_SANITIZE ResolveUnknownVirtualCallEntrypoint(const Method *caller, ObjectHeader *obj,
                                                                             size_t callee_id, size_t *slot)
{
    {
        auto thread = ManagedThread::GetCurrent();
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> handle_obj(thread, obj);

        BEGIN_ENTRYPOINT();
        auto runtime = Runtime::GetCurrent();
        Method *method = runtime->GetClassLinker()->GetMethod(*caller, panda_file::File::EntityId(callee_id));
        if (LIKELY(method != nullptr)) {
            // Cache a method index in vtable
            if (slot != nullptr && (!method->GetClass()->IsInterface() || method->IsDefaultInterfaceMethod())) {
                // We save 'vtable index + 1' because 0 value means uninitialized.
                // Codegen must subtract index after loading from the slot.
                *slot = method->GetVTableIndex() + 1;
            }

            auto *resolved = handle_obj.GetPtr()->ClassAddr<Class>()->ResolveVirtualMethod(method);
            ASSERT(resolved != nullptr);

            return reinterpret_cast<uintptr_t>(resolved);
        }
    }

    HandlePendingException();
    UNREACHABLE();
}

extern "C" void CheckStoreArrayReferenceEntrypoint(coretypes::Array *array, ObjectHeader *store_obj)
{
    BEGIN_ENTRYPOINT();
    ASSERT(array != nullptr);
    ASSERT(store_obj != nullptr);

    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto *array_class = array->ClassAddr<Class>();
    auto *element_class = array_class->GetComponentType();
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    if (UNLIKELY(!store_obj->IsInstanceOf(element_class))) {
        // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
        panda::ThrowArrayStoreException(array_class, store_obj->ClassAddr<Class>());
        HandlePendingException();
        UNREACHABLE();
    }
}

extern "C" Method *GetCalleeMethodEntrypoint(const Method *caller, size_t callee_id)
{
    BEGIN_ENTRYPOINT();
    auto *method = Runtime::GetCurrent()->GetClassLinker()->GetMethod(*caller, panda_file::File::EntityId(callee_id));
    if (UNLIKELY(method == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }

    return method;
}

extern "C" Method *GetCalleeMethodFromBytecodeId(const Method *caller, size_t callee_id)
{
    BEGIN_ENTRYPOINT();
    auto resolved_id = caller->GetClass()->ResolveMethodIndex(panda::BytecodeId(callee_id).AsIndex());
    auto *method = Runtime::GetCurrent()->GetClassLinker()->GetMethod(*caller, resolved_id);
    if (UNLIKELY(method == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }

    return method;
}

extern "C" Method *ResolveVirtualMethod(const Method *method, const ObjectHeader *obj)
{
    BEGIN_ENTRYPOINT();
    // SUPPRESS_CSA_NEXTLINE(alpha.core.WasteObjHeader)
    auto *cls = obj->ClassAddr<Class>();
    ASSERT(cls != nullptr);
    auto *resolved = cls->ResolveVirtualMethod(method);
    ASSERT(resolved != nullptr);
    return resolved;
}

extern "C" Method *GetUnknownCalleeMethodEntrypoint(const Method *caller, size_t callee_id, size_t *slot)
{
    BEGIN_ENTRYPOINT();
    auto *method = Runtime::GetCurrent()->GetClassLinker()->GetMethod(*caller, panda_file::File::EntityId(callee_id));
    if (UNLIKELY(method == nullptr)) {
        HandlePendingException();
        UNREACHABLE();
    }
    auto klass = method->GetClass();
    ASSERT(klass != nullptr);
    InitializeClassEntrypoint(klass);

    if (klass->IsInitialized() && slot != nullptr) {
        *slot = reinterpret_cast<size_t>(method);
    }

    return method;
}

extern "C" void SetExceptionEvent([[maybe_unused]] events::ExceptionType type)
{
#ifdef PANDA_EVENTS_ENABLED
    auto stack = StackWalker::Create(ManagedThread::GetCurrent());
    EVENT_EXCEPTION(std::string(stack.GetMethod()->GetFullName()), stack.GetBytecodePc(), stack.GetNativePc(), type);
#endif
}

extern "C" NO_ADDRESS_SANITIZE void ThrowExceptionEntrypoint(ObjectHeader *exception)
{
    BEGIN_ENTRYPOINT();
    LOG(DEBUG, INTEROP) << "ThrowExceptionEntrypoint \n";
    ASSERT(!ManagedThread::GetCurrent()->HasPendingException());
    if (exception == nullptr) {
        NullPointerExceptionEntrypoint();
        UNREACHABLE();
    }
    ManagedThread::GetCurrent()->SetException(exception);  // SUPPRESS_CSA(alpha.core.WasteObjHeader)

    SetExceptionEvent(events::ExceptionType::THROW);
    HandlePendingException(UnwindPolicy::SKIP_INLINED);
}

extern "C" NO_ADDRESS_SANITIZE void ThrowNativeExceptionEntrypoint()
{
    BEGIN_ENTRYPOINT();
    LOG(DEBUG, INTEROP) << "ThrowNativeExceptionEntrypoint \n";
    SetExceptionEvent(events::ExceptionType::NATIVE);
    HandlePendingException(UnwindPolicy::SKIP_INLINED);
}

extern "C" NO_ADDRESS_SANITIZE void ArrayIndexOutOfBoundsExceptionEntrypoint([[maybe_unused]] ssize_t idx,
                                                                             [[maybe_unused]] size_t length)
{
    BEGIN_ENTRYPOINT();
    LOG(DEBUG, INTEROP) << "ArrayIndexOutOfBoundsExceptionEntrypoint \n";
    ASSERT(!ManagedThread::GetCurrent()->HasPendingException());
    ThrowArrayIndexOutOfBoundsException(idx, length);
    SetExceptionEvent(events::ExceptionType::BOUND_CHECK);
    HandlePendingException(UnwindPolicy::SKIP_INLINED);
}

extern "C" NO_ADDRESS_SANITIZE void StringIndexOutOfBoundsExceptionEntrypoint([[maybe_unused]] ssize_t idx,
                                                                              [[maybe_unused]] size_t length)
{
    BEGIN_ENTRYPOINT();
    LOG(DEBUG, INTEROP) << "StringIndexOutOfBoundsExceptionEntrypoint \n";
    ASSERT(!ManagedThread::GetCurrent()->HasPendingException());
    ThrowStringIndexOutOfBoundsException(idx, length);
    SetExceptionEvent(events::ExceptionType::BOUND_CHECK);
    HandlePendingException(UnwindPolicy::SKIP_INLINED);
}

extern "C" NO_ADDRESS_SANITIZE void NullPointerExceptionEntrypoint()
{
    BEGIN_ENTRYPOINT();
    LOG(DEBUG, INTEROP) << "NullPointerExceptionEntrypoint \n";
    ASSERT(!ManagedThread::GetCurrent()->HasPendingException());
    ThrowNullPointerException();
    SetExceptionEvent(events::ExceptionType::NULL_CHECK);
    HandlePendingException(UnwindPolicy::SKIP_INLINED);
}

extern "C" NO_ADDRESS_SANITIZE void AbstractMethodErrorEntrypoint(Method *method)
{
    BEGIN_ENTRYPOINT();
    LOG(DEBUG, INTEROP) << "AbstractMethodErrorEntrypoint \n";
    ManagedThread *thread = ManagedThread::GetCurrent();
    ASSERT(!thread->HasPendingException());
    auto stack = StackWalker::Create(thread, UnwindPolicy::SKIP_INLINED);
    ThrowAbstractMethodError(method);
    ASSERT(thread->HasPendingException());
    SetExceptionEvent(events::ExceptionType::ABSTRACT_METHOD);
    if (stack.IsCFrame()) {
        FindCatchBlockInCFrames(thread, &stack, nullptr);
    }
}

extern "C" NO_ADDRESS_SANITIZE void ArithmeticExceptionEntrypoint()
{
    BEGIN_ENTRYPOINT();
    LOG(DEBUG, INTEROP) << "ArithmeticExceptionEntrypoint \n";
    ASSERT(!ManagedThread::GetCurrent()->HasPendingException());
    ThrowArithmeticException();
    SetExceptionEvent(events::ExceptionType::ARITHMETIC);
    HandlePendingException(UnwindPolicy::SKIP_INLINED);
}

extern "C" NO_ADDRESS_SANITIZE void NegativeArraySizeExceptionEntrypoint(ssize_t size)
{
    BEGIN_ENTRYPOINT();
    LOG(DEBUG, INTEROP) << "NegativeArraySizeExceptionEntrypoint \n";
    ASSERT(!ManagedThread::GetCurrent()->HasPendingException());
    ThrowNegativeArraySizeException(size);
    SetExceptionEvent(events::ExceptionType::NEGATIVE_SIZE);
    HandlePendingException(UnwindPolicy::SKIP_INLINED);
}

extern "C" NO_ADDRESS_SANITIZE void ClassCastExceptionEntrypoint(Class *inst_class, ObjectHeader *src_obj)
{
    BEGIN_ENTRYPOINT();
    LOG(DEBUG, INTEROP) << "ClassCastExceptionEntrypoint \n";
    ASSERT(!ManagedThread::GetCurrent()->HasPendingException());
    ASSERT(src_obj != nullptr);
    ThrowClassCastException(inst_class, src_obj->ClassAddr<Class>());  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
    SetExceptionEvent(events::ExceptionType::CAST_CHECK);
    HandlePendingException(UnwindPolicy::SKIP_INLINED);
}

extern "C" NO_ADDRESS_SANITIZE void StackOverflowExceptionEntrypoint()
{
    // WARNING: We should not add any heavy code constructions here, like events or other debug/testing stuff,
    // because we have small stack here, see ManagedThread::STACK_OVERFLOW_RESERVED_SIZE.
    auto thread = ManagedThread::GetCurrent();

    ASSERT(!thread->HasPendingException());
    thread->DisableStackOverflowCheck();
    ThrowStackOverflowException(thread);
    thread->EnableStackOverflowCheck();
    HandlePendingException(UnwindPolicy::SKIP_INLINED);
}

extern "C" NO_ADDRESS_SANITIZE void DeoptimizeEntrypoint(uint8_t deoptimize_type)
{
    BEGIN_ENTRYPOINT();
    auto type = static_cast<panda::compiler::DeoptimizeType>(deoptimize_type);
    LOG(DEBUG, INTEROP) << "DeoptimizeEntrypoint (reason: " << panda::compiler::DeoptimizeTypeToString(type) << ")\n";
    ASSERT(!ManagedThread::GetCurrent()->HasPendingException());
    bool destroy = (type == panda::compiler::DeoptimizeType::INLINE_IC);
    auto stack = StackWalker::Create(ManagedThread::GetCurrent());
    Deoptimize(&stack, nullptr, false, destroy);
}

extern "C" NO_ADDRESS_SANITIZE void ThrowInstantiationErrorEntrypoint(Class *klass)
{
    BEGIN_ENTRYPOINT();
    LOG(DEBUG, INTEROP) << "ThrowInstantiationErrorEntrypoint \n";
    ASSERT(!ManagedThread::GetCurrent()->HasPendingException());
    const auto &name = klass->GetName();
    PandaString pname(name.cbegin(), name.cend());
    ThrowInstantiationError(pname);
    SetExceptionEvent(events::ExceptionType::INSTANTIATION_ERROR);
    HandlePendingException(UnwindPolicy::SKIP_INLINED);
}

extern "C" DecodedTaggedValue GetInitialTaggedValue(Method *method)
{
    BEGIN_ENTRYPOINT();
    return Runtime::GetCurrent()->GetLanguageContext(*method).GetInitialDecodedValue();
}

extern "C" void LockObjectEntrypoint(ObjectHeader *obj)
{
    BEGIN_ENTRYPOINT();
    panda::intrinsics::ObjectMonitorEnter(obj);  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
}

extern "C" void LockObjectSlowPathEntrypoint(ObjectHeader *obj)
{
    BEGIN_ENTRYPOINT();
    panda::intrinsics::ObjectMonitorEnter(obj);  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
    if (!ManagedThread::GetCurrent()->HasPendingException()) {
        return;
    }
    LOG(DEBUG, INTEROP) << "ThrowNativeExceptionEntrypoint after LockObject \n";
    SetExceptionEvent(events::ExceptionType::NATIVE);
    HandlePendingException(UnwindPolicy::SKIP_INLINED);
}

extern "C" void UnlockObjectEntrypoint(ObjectHeader *obj)
{
    BEGIN_ENTRYPOINT();
    panda::intrinsics::ObjectMonitorExit(obj);  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
}

extern "C" void UnlockObjectSlowPathEntrypoint(ObjectHeader *obj)
{
    BEGIN_ENTRYPOINT();
    panda::intrinsics::ObjectMonitorExit(obj);  // SUPPRESS_CSA(alpha.core.WasteObjHeader)
    if (!ManagedThread::GetCurrent()->HasPendingException()) {
        return;
    }
    LOG(DEBUG, INTEROP) << "ThrowNativeExceptionEntrypoint after UnlockObject \n";
    SetExceptionEvent(events::ExceptionType::NATIVE);
    HandlePendingException(UnwindPolicy::SKIP_INLINED);
}

extern "C" NO_ADDRESS_SANITIZE void IncompatibleClassChangeErrorForMethodConflictEntrypoint(Method *method)
{
    BEGIN_ENTRYPOINT();
    LOG(DEBUG, INTEROP) << "IncompatibleClassChangeErrorForMethodConflictEntrypoint \n";
    ManagedThread *thread = ManagedThread::GetCurrent();
    ASSERT(!thread->HasPendingException());
    auto stack = StackWalker::Create(thread, UnwindPolicy::SKIP_INLINED);
    ThrowIncompatibleClassChangeErrorForMethodConflict(method);
    ASSERT(thread->HasPendingException());
    SetExceptionEvent(events::ExceptionType::ICCE_METHOD_CONFLICT);
    if (stack.IsCFrame()) {
        FindCatchBlockInCFrames(thread, &stack, nullptr);
    }
}

extern "C" const uint8_t *GetInstructionsByMethod(const Method *method)
{
    BEGIN_ENTRYPOINT();
    return method->GetInstructions();
}

extern "C" size_t GetNumVregsByMethod(const Method *method)
{
    BEGIN_ENTRYPOINT();
    return method->GetNumVregs();
}

extern "C" size_t GetNumArgsByMethod(const Method *method)
{
    BEGIN_ENTRYPOINT();
    return method->GetNumArgs();
}

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
extern "C" void TraceEntrypoint(size_t pid, ...)
{
    LOG_ENTRYPOINT();
    auto id = static_cast<TraceId>(pid);

    va_list args;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    va_start(args, pid);
    switch (id) {
        case TraceId::METHOD_ENTER: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            [[maybe_unused]] auto method = va_arg(args, const Method *);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            [[maybe_unused]] auto kind = va_arg(args, events::MethodEnterKind);
            EVENT_METHOD_ENTER(method->GetFullName(), kind, ManagedThread::GetCurrent()->RecordMethodEnter());
            break;
        }
        case TraceId::METHOD_EXIT: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            [[maybe_unused]] auto method = va_arg(args, const Method *);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            [[maybe_unused]] auto kind = va_arg(args, events::MethodExitKind);
            EVENT_METHOD_EXIT(method->GetFullName(), kind, ManagedThread::GetCurrent()->RecordMethodExit());
            break;
        }
        case TraceId::PRINT_ARG: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            size_t args_num = va_arg(args, size_t);
            std::cerr << "[TRACE ARGS] ";
            for (size_t i = 0; i < args_num; i++) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                std::cerr << i << "=" << va_arg(args, void *) << " ";
            }
            std::cerr << std::endl;

            break;
        }
        case TraceId::TLAB_EVENT: {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            [[maybe_unused]] auto inst = va_arg(args, size_t);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            [[maybe_unused]] PandaString inst_name = (inst == 0) ? "NewArray" : "NewObject";
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            [[maybe_unused]] auto memory = va_arg(args, size_t);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            [[maybe_unused]] auto size = va_arg(args, size_t);
            [[maybe_unused]] auto tlab = reinterpret_cast<size_t>(ManagedThread::GetCurrent()->GetTLAB());
            // 1. Pointer to TLAB
            // 2. Instruction name
            // 3. Pointer to allocated memory
            // 4. size
            EVENT_TLAB_ALLOC(ManagedThread::GetCurrent()->GetId(), tlab, inst_name, memory, size);
            break;
        }
        default: {
            break;
        }
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    va_end(args);
}

extern "C" void LogEntrypoint(const char *fmt, ...)
{
    va_list args;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    va_end(args);
}

}  // namespace panda
