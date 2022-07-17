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
#ifndef PANDA_RUNTIME_METHOD_H_
#define PANDA_RUNTIME_METHOD_H_

#include <atomic>
#include <cstdint>
#include <functional>
#include <string_view>

#include "intrinsics_enum.h"
#include "libpandabase/utils/arch.h"
#include "libpandabase/utils/logger.h"
#include "libpandafile/code_data_accessor-inl.h"
#include "libpandafile/file.h"
#include "libpandafile/file_items.h"
#include "libpandafile/modifiers.h"
#include "runtime/bridge/bridge.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/class_helper.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_smart_pointers.h"
#include "runtime/interpreter/frame.h"
#include "value.h"

namespace panda {

class Class;
class ManagedThread;
class ProfilingData;

#ifdef PANDA_ENABLE_GLOBAL_REGISTER_VARIABLES
namespace interpreter {
class AccVRegisterT;
}  // namespace interpreter
using interpreter::AccVRegisterT;
#else
namespace interpreter {
using AccVRegisterT = AccVRegister;
}  // namespace interpreter
#endif

class FrameDeleter {
public:
    explicit FrameDeleter(ManagedThread *thread) : thread_(thread) {}

    void operator()(Frame *frame) const;

private:
    ManagedThread *thread_;
};

class Method {
public:
    using UniqId = uint64_t;

    enum CompilationStage {
        NOT_COMPILED,
        WAITING,
        COMPILATION,
        COMPILED,
        FAILED,
    };

    enum class VerificationStage {
        // There is allocated a separate bit for each state. Totally 3 bits is used.
        // When the method is not verified all bits are zero.
        // The next state is waiting for verification uses 2nd bit.
        // The final result (ok or fail) is stored in 1st and 0th bits.
        // State is changing as follow:
        //       000      ->    100    --+-->  110
        // (not verified)    (waiting)   |    (ok)
        //                               |
        //                               +-->  101
        //                                    (fail)
        // To read the state __builtin_ffs is used which returns index + 1 of the first set bit
        // or zero for 0 value. See BitsToVerificationStage for details about conversion set bit
        // index to VerificationStage.
        // So the value's order is choosen in a such way early stage must have highest value.
        NOT_VERIFIED = 0,
        VERIFIED_FAIL = 1,
        VERIFIED_OK = 2,
        WAITING = 4,
    };

    enum AnnotationField : uint32_t {
        IC_SIZE = 0,
        FUNCTION_LENGTH,
        FUNCTION_NAME,
        STRING_DATA_BEGIN = FUNCTION_NAME,
        STRING_DATA_END = FUNCTION_NAME
    };

    class Proto {
    public:
        using ShortyVector = PandaSmallVector<panda_file::Type>;
        using RefTypeVector = PandaSmallVector<std::string_view>;
        Proto() = default;

        Proto(const panda_file::File &pf, panda_file::File::EntityId proto_id);

        Proto(ShortyVector shorty, RefTypeVector ref_types)
            : shorty_(std::move(shorty)), ref_types_(std::move(ref_types))
        {
        }

        bool operator==(const Proto &other) const
        {
            return shorty_ == other.shorty_ && ref_types_ == other.ref_types_;
        }

        panda_file::Type GetReturnType() const
        {
            return shorty_[0];
        }

        std::string_view GetReturnTypeDescriptor() const;

        ShortyVector &GetShorty()
        {
            return shorty_;
        }

        const ShortyVector &GetShorty() const
        {
            return shorty_;
        }

        RefTypeVector &GetRefTypes()
        {
            return ref_types_;
        }

        const RefTypeVector &GetRefTypes() const
        {
            return ref_types_;
        }

        ~Proto() = default;

        DEFAULT_COPY_SEMANTIC(Proto);
        DEFAULT_MOVE_SEMANTIC(Proto);

    private:
        ShortyVector shorty_;
        RefTypeVector ref_types_;
    };

    class ProtoId {
    public:
        ProtoId(const panda_file::File &pf, panda_file::File::EntityId proto_id) : pf_(pf), proto_id_(proto_id) {}
        bool operator==(const ProtoId &other) const;
        bool operator==(const Proto &other) const;
        bool operator!=(const ProtoId &other) const
        {
            return !operator==(other);
        }
        bool operator!=(const Proto &other) const
        {
            return !operator==(other);
        }

        ~ProtoId() = default;

        DEFAULT_COPY_CTOR(ProtoId)
        NO_COPY_OPERATOR(ProtoId);
        NO_MOVE_SEMANTIC(ProtoId);

    private:
        const panda_file::File &pf_;
        panda_file::File::EntityId proto_id_;
    };

    Method(Class *klass, const panda_file::File *pf, panda_file::File::EntityId file_id,
           panda_file::File::EntityId code_id, uint32_t access_flags, uint32_t num_args, const uint16_t *shorty);

    explicit Method(const Method *method)
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        : access_flags_(method->access_flags_.load(std::memory_order_acquire)),
          num_args_(method->num_args_),
          stor_16_pair_(method->stor_16_pair_),
          class_word_(method->class_word_),
          panda_file_(method->panda_file_),
          file_id_(method->file_id_),
          code_id_(method->code_id_),
          shorty_(method->shorty_)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        pointer_.native_pointer_.store(
            // Atomic with relaxed order reason: data race with native_pointer_ with no synchronization or ordering
            // constraints imposed on other reads or writes NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            method->pointer_.native_pointer_.load(std::memory_order_relaxed), std::memory_order_relaxed);

        // Atomic with release order reason: data race with compiled_entry_point_ with dependecies on writes before the
        // store which should become visible acquire
        compiled_entry_point_.store(method->IsNative() ? method->GetCompiledEntryPoint()
                                                       : GetCompiledCodeToInterpreterBridge(method),
                                    std::memory_order_release);
        SetCompilationStatus(CompilationStage::NOT_COMPILED);
    }

    Method() = delete;
    Method(const Method &) = delete;
    Method(Method &&) = delete;
    Method &operator=(const Method &) = delete;
    Method &operator=(Method &&) = delete;

    uint32_t GetNumArgs() const
    {
        return num_args_;
    }

    uint32_t GetNumVregs() const
    {
        if (!code_id_.IsValid()) {
            return 0;
        }
        return panda_file::CodeDataAccessor::GetNumVregs(*(panda_file_), code_id_);
    }

    uint32_t GetCodeSize() const
    {
        if (!code_id_.IsValid()) {
            return 0;
        }
        panda_file::CodeDataAccessor cda(*(panda_file_), code_id_);
        return cda.GetCodeSize();
    }

    const uint8_t *GetInstructions() const
    {
        if (!code_id_.IsValid()) {
            return nullptr;
        }
        return panda_file::CodeDataAccessor::GetInstructions(*panda_file_, code_id_);
    }

    /*
     * Invoke the method as a static method.
     * Number of arguments and their types must match the method's signature
     */
    Value Invoke(ManagedThread *thread, Value *args, bool proxy_call = false);

    void InvokeVoid(ManagedThread *thread, Value *args)
    {
        Invoke(thread, args);
    }

    /*
     * Invoke the method as a dynamic function.
     * Number of arguments may vary, all arguments must be of type DecodedTaggedValue.
     * args - array of arguments. The first value must be the callee function object
     * num_args - length of args array
     * data - panda::ExtFrame language-related extension data
     */
    coretypes::TaggedValue InvokeDyn(ManagedThread *thread, uint32_t num_args, coretypes::TaggedValue *args);

    template <class InvokeHelper>
    coretypes::TaggedValue InvokeDyn(ManagedThread *thread, uint32_t num_args, coretypes::TaggedValue *args);

    /*
     * Enter execution context (ECMAScript generators)
     * pc - pc of context
     * acc - accumulator of context
     * nregs - number of registers in context
     * regs - registers of context
     * data - panda::ExtFrame language-related extension data
     */
    coretypes::TaggedValue InvokeContext(ManagedThread *thread, const uint8_t *pc, coretypes::TaggedValue acc,
                                         uint32_t nregs, coretypes::TaggedValue *regs);

    template <class InvokeHelper>
    coretypes::TaggedValue InvokeContext(ManagedThread *thread, const uint8_t *pc, coretypes::TaggedValue acc,
                                         uint32_t nregs, coretypes::TaggedValue *regs);

    /*
     * Create new frame for native method, but don't start execution
     * Number of arguments may vary, all arguments must be of type DecodedTaggedValue.
     * args - array of arguments. The first value must be the callee function object
     * num_vregs - number of registers in frame
     * num_args - length of args array
     * data - panda::ExtFrame language-related extension data
     */
    template <class InvokeHelper, class ValueT>
    Frame *EnterNativeMethodFrame(ManagedThread *thread, uint32_t num_vregs, uint32_t num_args, ValueT *args);

    /*
     * Pop native method frame
     */
    static void ExitNativeMethodFrame(ManagedThread *thread);

    Class *GetClass() const
    {
        return reinterpret_cast<Class *>(class_word_);
    }

    void SetClass(Class *cls)
    {
        class_word_ = static_cast<ClassHelper::classWordSize>(ToObjPtrType(cls));
    }

    void SetPandaFile(const panda_file::File *file)
    {
        panda_file_ = file;
    }

    const panda_file::File *GetPandaFile() const
    {
        return panda_file_;
    }

    panda_file::File::EntityId GetFileId() const
    {
        return file_id_;
    }

    panda_file::File::EntityId GetCodeId() const
    {
        return code_id_;
    }

    inline uint16_t GetHotnessCounter() const
    {
        return stor_16_pair_.hotness_counter_;
    }

    inline NO_THREAD_SANITIZE void IncrementHotnessCounter()
    {
        ++stor_16_pair_.hotness_counter_;
    }

    NO_THREAD_SANITIZE void ResetHotnessCounter()
    {
        stor_16_pair_.hotness_counter_ = 0;
    }

    template <class AccVRegisterPtrT>
    NO_THREAD_SANITIZE void SetAcc([[maybe_unused]] AccVRegisterPtrT acc);

    // NO_THREAD_SANITIZE because of perfomance degradation (see commit 7c913cb1 and MR 997#note_113500)
    template <class AccVRegisterPtrT>
    NO_THREAD_SANITIZE bool IncrementHotnessCounter(uintptr_t bytecode_offset, [[maybe_unused]] AccVRegisterPtrT cc,
                                                    bool osr = false);

    // TODO(xucheng): change the input type to uint16_t when we don't input the max num of int32_t
    inline NO_THREAD_SANITIZE void SetHotnessCounter(uint32_t counter)
    {
        stor_16_pair_.hotness_counter_ = static_cast<uint16_t>(counter);
    }

    int64_t GetBranchTakenCounter(uint32_t pc);
    int64_t GetBranchNotTakenCounter(uint32_t pc);

    const void *GetCompiledEntryPoint()
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return compiled_entry_point_.load(std::memory_order_acquire);
    }

    const void *GetCompiledEntryPoint() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return compiled_entry_point_.load(std::memory_order_acquire);
    }

    void SetCompiledEntryPoint(const void *entry_point)
    {
        // Atomic with release order reason: data race with compiled_entry_point_ with dependecies on writes before the
        // store which should become visible acquire
        compiled_entry_point_.store(entry_point, std::memory_order_release);
    }

    void SetInterpreterEntryPoint()
    {
        if (!IsNative()) {
            SetCompiledEntryPoint(GetCompiledCodeToInterpreterBridge(this));
        }
    }

    bool HasCompiledCode() const
    {
        auto entry_point = GetCompiledEntryPoint();
        return entry_point != GetCompiledCodeToInterpreterBridge() &&
               entry_point != GetCompiledCodeToInterpreterBridgeDyn();
    }

    inline CompilationStage GetCompilationStatus()
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return static_cast<CompilationStage>(
            (access_flags_.load(std::memory_order_acquire) & COMPILATION_STATUS_MASK) >> COMPILATION_STATUS_SHIFT);
    }

    inline CompilationStage GetCompilationStatus(uint32_t value)
    {
        return static_cast<CompilationStage>((value & COMPILATION_STATUS_MASK) >> COMPILATION_STATUS_SHIFT);
    }

    inline void SetCompilationStatus(enum CompilationStage new_status)
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        auto result = (access_flags_.load(std::memory_order_acquire) & ~COMPILATION_STATUS_MASK) |
                      static_cast<uint32_t>(new_status) << COMPILATION_STATUS_SHIFT;
        // Atomic with release order reason: data race with access_flags_ with dependecies on writes before the store
        // which should become visible acquire
        access_flags_.store(result, std::memory_order_release);
    }

    inline bool AtomicSetCompilationStatus(enum CompilationStage old_status, enum CompilationStage new_status)
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        uint32_t old_value = access_flags_.load(std::memory_order_acquire);
        while (GetCompilationStatus(old_value) == old_status) {
            uint32_t new_value = MakeCompilationStatusValue(old_value, new_status);
            if (access_flags_.compare_exchange_strong(old_value, new_value)) {
                return true;
            }
        }
        return false;
    }

    panda_file::Type GetReturnType() const;

    panda_file::File::StringData GetRefReturnType() const;

    // idx - index number of the argument in the signature
    panda_file::Type GetArgType(size_t idx) const;

    panda_file::File::StringData GetRefArgType(size_t idx) const;

    template <typename Callback>
    void EnumerateTypes(Callback handler) const;

    panda_file::File::StringData GetName() const;

    panda_file::File::StringData GetClassName() const;

    PandaString GetFullName(bool with_signature = false) const;

    static uint32_t GetFullNameHashFromString(const PandaString &str);
    static uint32_t GetClassNameHashFromString(const PandaString &str);

    Proto GetProto() const;

    ProtoId GetProtoId() const;

    size_t GetFrameSize() const
    {
        return Frame::GetAllocSize(GetNumArgs() + GetNumVregs(), EmptyExtFrameDataSize);
    }

    uint32_t GetNumericalAnnotation(AnnotationField field_id) const;
    panda_file::File::StringData GetStringDataAnnotation(AnnotationField field_id) const;

    uint32_t GetAccessFlags() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return access_flags_.load(std::memory_order_acquire);
    }

    void SetAccessFlags(uint32_t access_flags)
    {
        // Atomic with release order reason: data race with access_flags_ with dependecies on writes before the store
        // which should become visible acquire
        access_flags_.store(access_flags, std::memory_order_release);
    }

    bool IsStatic() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (access_flags_.load(std::memory_order_acquire) & ACC_STATIC) != 0;
    }

    bool IsNative() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (access_flags_.load(std::memory_order_acquire) & ACC_NATIVE) != 0;
    }

    bool IsPublic() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (access_flags_.load(std::memory_order_acquire) & ACC_PUBLIC) != 0;
    }

    bool IsPrivate() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (access_flags_.load(std::memory_order_acquire) & ACC_PRIVATE) != 0;
    }

    bool IsProtected() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (access_flags_.load(std::memory_order_acquire) & ACC_PROTECTED) != 0;
    }

    bool IsIntrinsic() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (access_flags_.load(std::memory_order_acquire) & ACC_INTRINSIC) != 0;
    }

    bool IsSynthetic() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (access_flags_.load(std::memory_order_acquire) & ACC_SYNTHETIC) != 0;
    }

    bool IsAbstract() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (access_flags_.load(std::memory_order_acquire) & ACC_ABSTRACT) != 0;
    }

    bool IsFinal() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (access_flags_.load(std::memory_order_acquire) & ACC_FINAL) != 0;
    }

    bool IsSynchronized() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (access_flags_.load(std::memory_order_acquire) & ACC_SYNCHRONIZED) != 0;
    }

    bool HasSingleImplementation() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (access_flags_.load(std::memory_order_acquire) & ACC_SINGLE_IMPL) != 0;
    }

    void SetHasSingleImplementation(bool v)
    {
        if (v) {
            // Atomic with acq_rel order reason: data race with access_flags_ with dependecies on reads after the load
            // and on writes before the store
            access_flags_.fetch_or(ACC_SINGLE_IMPL, std::memory_order_acq_rel);
        } else {
            // Atomic with acq_rel order reason: data race with access_flags_ with dependecies on reads after the load
            // and on writes before the store
            access_flags_.fetch_and(~ACC_SINGLE_IMPL, std::memory_order_acq_rel);
        }
    }

    Method *GetSingleImplementation()
    {
        return HasSingleImplementation() ? this : nullptr;
    }

    void SetIntrinsic(intrinsics::Intrinsic intrinsic)
    {
        ASSERT(!IsIntrinsic());
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        ASSERT((access_flags_.load(std::memory_order_acquire) & INTRINSIC_MASK) == 0);
        auto result = ACC_INTRINSIC | static_cast<uint32_t>(intrinsic) << INTRINSIC_SHIFT;
        // Atomic with acq_rel order reason: data race with access_flags_ with dependecies on reads after the load and
        // on writes before the store
        access_flags_.fetch_or(result, std::memory_order_acq_rel);
    }

    intrinsics::Intrinsic GetIntrinsic() const
    {
        ASSERT(IsIntrinsic());
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return static_cast<intrinsics::Intrinsic>((access_flags_.load(std::memory_order_acquire) & INTRINSIC_MASK) >>
                                                  INTRINSIC_SHIFT);
    }

    void SetVTableIndex(uint16_t vtable_index)
    {
        stor_16_pair_.vtable_index_ = vtable_index;
    }

    uint16_t GetVTableIndex() const
    {
        return stor_16_pair_.vtable_index_;
    }

    void SetNativePointer(void *native_pointer)
    {
        ASSERT((IsNative() || IsProxy()));
        // Atomic with relaxed order reason: data race with native_pointer_ with no synchronization or ordering
        // constraints imposed on other reads or writes NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        pointer_.native_pointer_.store(native_pointer, std::memory_order_relaxed);
    }

    void *GetNativePointer() const
    {
        ASSERT((IsNative() || IsProxy()));
        // Atomic with relaxed order reason: data race with native_pointer_ with no synchronization or ordering
        // constraints imposed on other reads or writes NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return pointer_.native_pointer_.load(std::memory_order_relaxed);
    }

    const uint16_t *GetShorty() const
    {
        return shorty_;
    }

    uint32_t FindCatchBlock(const Class *cls, uint32_t pc) const;

    panda_file::Type GetEffectiveArgType(size_t idx) const;

    panda_file::Type GetEffectiveReturnType() const;

    void SetIsDefaultInterfaceMethod()
    {
        // Atomic with acq_rel order reason: data race with access_flags_ with dependecies on reads after the load and
        // on writes before the store
        access_flags_.fetch_or(ACC_DEFAULT_INTERFACE_METHOD, std::memory_order_acq_rel);
    }

    bool IsDefaultInterfaceMethod() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (access_flags_.load(std::memory_order_acquire) & ACC_DEFAULT_INTERFACE_METHOD) != 0;
    }

    bool IsConstructor() const
    {
        // Atomic with acquire order reason: data race with access_flags_ with dependecies on reads after the load which
        // should become visible
        return (access_flags_.load(std::memory_order_acquire) & ACC_CONSTRUCTOR) != 0;
    }

    bool IsInstanceConstructor() const
    {
        return IsConstructor() && !IsStatic();
    }

    bool IsStaticConstructor() const
    {
        return IsConstructor() && IsStatic();
    }

    static constexpr uint32_t GetAccessFlagsOffset()
    {
        return MEMBER_OFFSET(Method, access_flags_);
    }
    static constexpr uint32_t GetNumArgsOffset()
    {
        return MEMBER_OFFSET(Method, num_args_);
    }
    static constexpr uint32_t GetVTableIndexOffset()
    {
        return MEMBER_OFFSET(Method, stor_16_pair_) + MEMBER_OFFSET(Storage16Pair, vtable_index_);
    }
    static constexpr uint32_t GetHotnessCounterOffset()
    {
        return MEMBER_OFFSET(Method, stor_16_pair_) + MEMBER_OFFSET(Storage16Pair, hotness_counter_);
    }
    static constexpr uint32_t GetClassOffset()
    {
        return MEMBER_OFFSET(Method, class_word_);
    }

    static constexpr uint32_t GetCompiledEntryPointOffset()
    {
        return MEMBER_OFFSET(Method, compiled_entry_point_);
    }
    static constexpr uint32_t GetPandaFileOffset()
    {
        return MEMBER_OFFSET(Method, panda_file_);
    }
    static constexpr uint32_t GetNativePointerOffset()
    {
        return MEMBER_OFFSET(Method, pointer_);
    }
    static constexpr uint32_t GetShortyOffset()
    {
        return MEMBER_OFFSET(Method, shorty_);
    }

    template <typename Callback>
    void EnumerateTryBlocks(Callback callback) const;

    template <typename Callback>
    void EnumerateCatchBlocks(Callback callback) const;

    template <typename Callback>
    void EnumerateExceptionHandlers(Callback callback) const;

    static inline UniqId CalcUniqId(const panda_file::File *file, panda_file::File::EntityId file_id)
    {
        constexpr uint64_t HALF = 32ULL;
        uint64_t uid = file->GetUniqId();
        uid <<= HALF;
        uid |= file_id.GetOffset();
        return uid;
    }

    // for synthetic methods, like array .ctor
    static UniqId CalcUniqId(const uint8_t *class_descr, const uint8_t *name);

    UniqId GetUniqId() const
    {
        return CalcUniqId(panda_file_, file_id_);
    }

    int32_t GetLineNumFromBytecodeOffset(uint32_t bc_offset) const;

    panda_file::File::StringData GetClassSourceFile() const;

    void StartProfiling();
    void StopProfiling();

    bool IsProxy() const;

    ProfilingData *GetProfilingData()
    {
        if (UNLIKELY(IsNative() || IsProxy())) {
            return nullptr;
        }
        // Atomic with acquire order reason: data race with profiling_data_ with dependecies on reads after the load
        // which should become visible NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return pointer_.profiling_data_.load(std::memory_order_acquire);
    }

    ProfilingData *GetProfilingDataWithoutCheck()
    {
        // Atomic with acquire order reason: data race with profiling_data_ with dependecies on reads after the load
        // which should become visible NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return pointer_.profiling_data_.load(std::memory_order_acquire);
    }

    const ProfilingData *GetProfilingData() const
    {
        if (UNLIKELY(IsNative() || IsProxy())) {
            return nullptr;
        }
        // Atomic with acquire order reason: data race with profiling_data_ with dependecies on reads after the load
        // which should become visible NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return pointer_.profiling_data_.load(std::memory_order_acquire);
    }

    bool IsProfiling() const
    {
        return GetProfilingData() != nullptr;
    }

    bool IsProfilingWithoutLock() const
    {
        if (UNLIKELY(IsNative() || IsProxy())) {
            return false;
        }
        // Atomic with acquire order reason: data race with profiling_data_ with dependecies on reads after the load
        // which should become visible NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        return pointer_.profiling_data_.load(std::memory_order_acquire) != nullptr;
    }

    bool AddJobInQueue();
    void WaitForVerification();
    void SetVerified(bool result);
    bool IsVerified() const;
    bool Verify();
    void EnqueueForVerification();

    ~Method();

private:
    VerificationStage GetVerificationStage() const;
    void SetVerificationStage(VerificationStage stage);
    VerificationStage ExchangeVerificationStage(VerificationStage stage);
    static VerificationStage BitsToVerificationStage(uint32_t bits);

    Value InvokeCompiledCode(ManagedThread *thread, uint32_t num_args, Value *args);

    Value GetReturnValueFromTaggedValue(DecodedTaggedValue ret_value)
    {
        Value res(static_cast<int64_t>(0));

        panda_file::Type ret_type = GetReturnType();

        if (ret_type.GetId() != panda_file::Type::TypeId::VOID) {
            if (ret_type.GetId() == panda_file::Type::TypeId::REFERENCE) {
                res = Value(reinterpret_cast<ObjectHeader *>(ret_value.value));
            } else if (ret_type.GetId() == panda_file::Type::TypeId::TAGGED) {
                res = Value(ret_value.value, ret_value.tag);
            } else {
                res = Value(ret_value.value);
            }
        }

        return res;
    }

    inline static uint32_t MakeCompilationStatusValue(uint32_t value, CompilationStage new_status)
    {
        value &= ~COMPILATION_STATUS_MASK;
        value |= static_cast<uint32_t>(new_status) << COMPILATION_STATUS_SHIFT;
        return value;
    }

    template <class InvokeHelper, class ValueT>
    ValueT InvokeInterpretedCode(ManagedThread *thread, uint32_t num_actual_args, ValueT *args);

    template <class InvokeHelper, class ValueT>
    PandaUniquePtr<Frame, FrameDeleter> InitFrame(ManagedThread *thread, uint32_t num_actual_args, ValueT *args,
                                                  Frame *current_frame);

    template <class InvokeHelper, class ValueT, bool is_native_method>
    PandaUniquePtr<Frame, FrameDeleter> InitFrameWithNumVRegs(ManagedThread *thread, uint32_t num_vregs,
                                                              uint32_t num_actual_args, ValueT *args,
                                                              Frame *current_frame);

    template <class InvokeHelper, class ValueT>
    ValueT GetReturnValueFromException();

    template <class InvokeHelper, class ValueT>
    ValueT GetReturnValueFromAcc(interpreter::AccVRegister &aac_vreg);

    template <class InvokeHelper, class ValueT>
    ValueT InvokeImpl(ManagedThread *thread, uint32_t num_actual_args, ValueT *args, bool proxy_call);

private:
    union PointerInMethod {
        // It's native pointer when the method is native or proxy method.
        std::atomic<void *> native_pointer_;
        // It's profiling data when the method isn't native or proxy method.
        std::atomic<ProfilingData *> profiling_data_;
    };

    struct Storage16Pair {
        uint16_t vtable_index_;
        uint16_t hotness_counter_;
    };

    std::atomic_uint32_t access_flags_;
    uint32_t num_args_;
    Storage16Pair stor_16_pair_;
    ClassHelper::classWordSize class_word_;

    std::atomic<const void *> compiled_entry_point_ {nullptr};
    const panda_file::File *panda_file_;
    union PointerInMethod pointer_ {
    };

    panda_file::File::EntityId file_id_;
    panda_file::File::EntityId code_id_;
    const uint16_t *shorty_;
};

static_assert(!std::is_polymorphic_v<Method>);

}  // namespace panda

#endif  // PANDA_RUNTIME_METHOD_H_
