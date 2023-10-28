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
#ifndef PANDA_RUNTIME_COMPILER_H_
#define PANDA_RUNTIME_COMPILER_H_

#include "compiler/compile_method.h"
#include "compiler/compiler_task_runner.h"
#include "compiler/optimizer/ir/runtime_interface.h"
#include "libpandabase/mem/code_allocator.h"
#include "libpandabase/os/mutex.h"
#include "runtime/entrypoints/entrypoints.h"
#include "runtime/include/hclass.h"
#include "runtime/include/compiler_interface.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/locks.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/method.h"
#include "runtime/include/runtime_options.h"
#include "runtime/interpreter/frame.h"
#include "runtime/mem/gc/gc_barrier_set.h"
#include "runtime/mem/tlab.h"
#include "runtime/compiler_thread_pool_worker.h"
#include "runtime/compiler_task_manager_worker.h"
#include "runtime/include/thread.h"

#include "runtime/osr.h"

namespace panda {

using compiler::RuntimeInterface;
using compiler::UnresolvedTypesInterface;

inline panda::Method *MethodCast(RuntimeInterface::MethodPtr method)
{
    return static_cast<panda::Method *>(method);
}

struct ScopedMutatorLock : public os::memory::ReadLockHolder<MutatorLock> {
    ScopedMutatorLock() : os::memory::ReadLockHolder<MutatorLock>(*PandaVM::GetCurrent()->GetMutatorLock()) {}
};

class PANDA_PUBLIC_API ClassHierarchyAnalysisWrapper : public compiler::IClassHierarchyAnalysis {
public:
    RuntimeInterface::MethodPtr GetSingleImplementation(RuntimeInterface::MethodPtr method) override
    {
        return static_cast<Method *>(method)->GetSingleImplementation();
    }
    bool IsSingleImplementation(RuntimeInterface::MethodPtr method) override
    {
        return static_cast<Method *>(method)->HasSingleImplementation();
    }
    void AddDependency(RuntimeInterface::MethodPtr callee, RuntimeInterface::MethodPtr caller) override;
};

class PANDA_PUBLIC_API InlineCachesWrapper : public compiler::InlineCachesInterface {
public:
    CallKind GetClasses(RuntimeInterface::MethodPtr m, uintptr_t pc,
                        ArenaVector<RuntimeInterface::ClassPtr> *classes) override;
};

class PANDA_PUBLIC_API UnresolvedTypesWrapper : public UnresolvedTypesInterface {
public:
    bool AddTableSlot(RuntimeInterface::MethodPtr method, uint32_t type_id, SlotKind kind) override;
    uintptr_t GetTableSlot(RuntimeInterface::MethodPtr method, uint32_t type_id, SlotKind kind) const override;

private:
    PandaMap<RuntimeInterface::MethodPtr, PandaMap<std::pair<uint32_t, SlotKind>, uintptr_t>> slots_;
};

class PANDA_PUBLIC_API PandaRuntimeInterface : public RuntimeInterface {
public:
    void *GetRuntimeEntry() override
    {
        return nullptr;
    }

    compiler::IClassHierarchyAnalysis *GetCha() override
    {
        return &cha_;
    }

    compiler::InlineCachesInterface *GetInlineCaches() override
    {
        return &inline_caches_;
    }

    compiler::UnresolvedTypesInterface *GetUnresolvedTypes() override
    {
        return &unresolved_types_;
    }

    unsigned GetReturnReasonOk() const override
    {
        return static_cast<unsigned>(CompilerInterface::ReturnReason::RET_OK);
    }
    unsigned GetReturnReasonDeopt() const override
    {
        return static_cast<unsigned>(CompilerInterface::ReturnReason::RET_DEOPTIMIZATION);
    }

    panda::Class *ClassCast(ClassPtr cls) const
    {
        return static_cast<panda::Class *>(cls);
    }

    size_t GetStackOverflowCheckOffset() const override
    {
        return ManagedThread::GetStackOverflowCheckOffset();
    }

    /**********************************************************************************/
    /// Binary file information
    BinaryFilePtr GetBinaryFileForMethod(MethodPtr method) const override
    {
        return const_cast<panda_file::File *>(MethodCast(method)->GetPandaFile());
    }

    MethodId ResolveMethodIndex(MethodPtr parent_method, MethodIndex index) const override;

    FieldId ResolveFieldIndex(MethodPtr parent_method, FieldIndex index) const override;

    IdType ResolveTypeIndex(MethodPtr parent_method, TypeIndex index) const override;

    /**********************************************************************************/
    /// Method information
    MethodPtr GetMethodById(MethodPtr parent_method, MethodId id) const override;

    MethodId GetMethodId(MethodPtr method) const override;

    IntrinsicId GetIntrinsicId([[maybe_unused]] MethodPtr method) const override;

    uint64_t GetUniqMethodId(MethodPtr method) const override;

    MethodPtr ResolveVirtualMethod(ClassPtr cls, MethodPtr method) const override;

    MethodPtr ResolveInterfaceMethod(ClassPtr cls, MethodPtr method) const override;

    compiler::DataType::Type GetMethodReturnType(MethodPtr method) const override
    {
        return ToCompilerType(MethodCast(method)->GetEffectiveReturnType());
    }

    IdType GetMethodReturnTypeId(MethodPtr method) const override;
    IdType GetMethodArgReferenceTypeId(MethodPtr method, uint16_t num) const override;

    compiler::DataType::Type GetMethodTotalArgumentType(MethodPtr method, size_t index) const override
    {
        return ToCompilerType(MethodCast(method)->GetEffectiveArgType(index));
    }
    size_t GetMethodTotalArgumentsCount(MethodPtr method) const override
    {
        return MethodCast(method)->GetNumArgs();
    }

    bool IsMemoryBarrierRequired(MethodPtr method) const override;

    compiler::DataType::Type GetMethodReturnType(MethodPtr parent_method, MethodId id) const override;

    compiler::DataType::Type GetMethodArgumentType(MethodPtr parent_method, MethodId id, size_t index) const override;

    size_t GetMethodArgumentsCount(MethodPtr parent_method, MethodId id) const override;
    size_t GetMethodArgumentsCount(MethodPtr method) const override
    {
        return MethodCast(method)->GetNumArgs();
    }
    size_t GetMethodRegistersCount(MethodPtr method) const override
    {
        return MethodCast(method)->GetNumVregs();
    }
    const uint8_t *GetMethodCode(MethodPtr method) const override
    {
        return MethodCast(method)->GetInstructions();
    }
    size_t GetMethodCodeSize(MethodPtr method) const override
    {
        return MethodCast(method)->GetCodeSize();
    }
    SourceLanguage GetMethodSourceLanguage(MethodPtr method) const override
    {
        return static_cast<SourceLanguage>(MethodCast(method)->GetClass()->GetSourceLang());
    }
    void SetCompiledEntryPoint(MethodPtr method, void *ep) override
    {
        MethodCast(method)->SetCompiledEntryPoint(ep);
    }
    bool TrySetOsrCode(MethodPtr method, void *ep) override
    {
        ScopedMutatorLock lock;
        // Ignore OSR code if method was deoptimized during OSR compilation
        if (!static_cast<const Method *>(method)->HasCompiledCode()) {
            return false;
        }
        CompilerInterface *compiler = Thread::GetCurrent()->GetVM()->GetCompiler();
        ASSERT(compiler->GetOsrCode(static_cast<const Method *>(method)) == nullptr);
        compiler->SetOsrCode(static_cast<const Method *>(method), ep);
        return true;
    }
    void *GetOsrCode(MethodPtr method) override
    {
        return Thread::GetCurrent()->GetVM()->GetCompiler()->GetOsrCode(static_cast<const Method *>(method));
    }
    bool HasCompiledCode(MethodPtr method) override
    {
        return MethodCast(method)->HasCompiledCode();
    }

    bool IsInstantiable(ClassPtr klass) const override
    {
        return ClassCast(klass)->IsInstantiable();
    }

    uint32_t GetAccessFlagAbstractMask() const override
    {
        return panda::ACC_ABSTRACT;
    }

    uint32_t GetVTableIndex(MethodPtr method) const override
    {
        return MethodCast(method)->GetVTableIndex();
    }

    size_t GetClassIdForField(MethodPtr method, size_t field_id) const override
    {
        auto fda =
            panda_file::FieldDataAccessor(*MethodCast(method)->GetPandaFile(), panda_file::File::EntityId(field_id));
        return fda.GetClassId().GetOffset();
    }

    size_t GetClassIdForField(FieldPtr field) const override
    {
        return FieldCast(field)->GetClass()->GetFileId().GetOffset();
    }

    ClassPtr GetClassForField(FieldPtr field) const override;

    size_t GetClassIdForMethod(MethodPtr method) const override
    {
        auto mda = panda_file::MethodDataAccessor(*MethodCast(method)->GetPandaFile(), MethodCast(method)->GetFileId());
        return mda.GetClassId().GetOffset();
    }

    size_t GetClassIdForMethod(MethodPtr parent_method, size_t method_id) const override
    {
        auto mda = panda_file::MethodDataAccessor(*MethodCast(parent_method)->GetPandaFile(),
                                                  panda_file::File::EntityId(method_id));
        return mda.GetClassId().GetOffset();
    }

    bool HasNativeException(MethodPtr method) const override;
    bool IsMethodExternal(MethodPtr parent_method, MethodPtr callee_method) const override;

    bool IsMethodIntrinsic(MethodPtr method) const override
    {
        return MethodCast(method)->IsIntrinsic();
    }

    bool IsMethodAbstract(MethodPtr method) const override
    {
        return MethodCast(method)->IsAbstract();
    }

    bool IsMethodIntrinsic(MethodPtr parent_method, MethodId id) const override;

    bool IsMethodFinal(MethodPtr method) const override
    {
        return MethodCast(method)->IsFinal();
    }

    bool IsMethodStatic(MethodPtr parent_method, MethodId id) const override;
    bool IsMethodStatic(MethodPtr method) const override;

    bool IsMethodCanBeInlined(MethodPtr method) const override
    {
        auto method_ptr = MethodCast(method);
        if (Runtime::GetCurrent()->GetOptions().GetVerificationMode() == VerificationMode::ON_THE_FLY &&
            method_ptr->GetVerificationStage() != Method::VerificationStage::VERIFIED_OK) {
            return false;
        }
        return !(method_ptr->IsIntrinsic() || method_ptr->IsNative() || method_ptr->IsAbstract());
    }

    bool IsMethodStaticConstructor([[maybe_unused]] MethodPtr method) const override;

    std::string GetFileName(MethodPtr method) const override
    {
        return MethodCast(method)->GetPandaFile()->GetFilename();
    }

    std::string GetFullFileName(MethodPtr method) const override
    {
        return MethodCast(method)->GetPandaFile()->GetFullFileName();
    }

    std::string GetClassNameFromMethod(MethodPtr method) const override
    {
        ScopedMutatorLock lock;
        return MethodCast(method)->GetClass()->GetName();
    }

    std::string GetClassName(ClassPtr cls) const override
    {
        ScopedMutatorLock lock;
        return ClassCast(cls)->GetName();
    }

    std::string GetMethodName(MethodPtr method) const override
    {
        return utf::Mutf8AsCString(MethodCast(method)->GetName().data);
    }

    std::string GetLineNumberAndSourceFile(MethodPtr method, uint32_t pc) const override
    {
        return std::string(MethodCast(method)->GetLineNumberAndSourceFile(pc));
    }

    int64_t GetBranchTakenCounter(MethodPtr method, uint32_t pc) const override
    {
        return MethodCast(method)->GetBranchTakenCounter(pc);
    }

    int64_t GetBranchNotTakenCounter(MethodPtr method, uint32_t pc) const override
    {
        return MethodCast(method)->GetBranchNotTakenCounter(pc);
    }

    int64_t GetThrowTakenCounter(MethodPtr method, uint32_t pc) const override
    {
        return MethodCast(method)->GetThrowTakenCounter(pc);
    }

    std::string GetMethodFullName(MethodPtr method, bool with_signature) const override
    {
        return std::string(MethodCast(method)->GetFullName(with_signature));
    }

    ClassPtr GetClass(MethodPtr method) const override
    {
        ScopedMutatorLock lock;
        return reinterpret_cast<ClassPtr>(MethodCast(method)->GetClass());
    }

    std::string GetBytecodeString(MethodPtr method, uintptr_t pc) const override;

    panda::pandasm::LiteralArray GetLiteralArray(MethodPtr method, LiteralArrayId id) const override;

    bool IsInterfaceMethod(MethodPtr parent_method, MethodId id) const override;

    bool IsInterfaceMethod(MethodPtr method) const override;

    bool IsInstanceConstructor(MethodPtr method) const override
    {
        return MethodCast(method)->IsInstanceConstructor();
    }

    bool IsDestroyed(MethodPtr method) const override
    {
        return MethodCast(method)->IsDestroyed();
    }

    bool CanThrowException(MethodPtr method) const override;

    uint32_t FindCatchBlock(MethodPtr method, ClassPtr cls, uint32_t pc) const override;

    /**********************************************************************************/
    /// Thread information
    ::panda::mem::BarrierType GetPreType() const override;

    ::panda::mem::BarrierType GetPostType() const override;

    ::panda::mem::BarrierOperand GetBarrierOperand(::panda::mem::BarrierPosition barrier_position,
                                                   std::string_view operand_name) const override;

    /**********************************************************************************/
    /// Array information
    uint32_t GetArrayElementSize(MethodPtr method, IdType id) const override;

    uint32_t GetMaxArrayLength(ClassPtr klass) const override;

    uintptr_t GetPointerToConstArrayData(MethodPtr method, IdType id) const override;

    size_t GetOffsetToConstArrayData(MethodPtr method, IdType id) const override;

    ClassPtr GetArrayU16Class(MethodPtr method) const override;
    /**********************************************************************************/
    /// String information
    bool IsCompressedStringsEnabled() const override
    {
        return panda::coretypes::String::GetCompressedStringsEnabled();
    }

    ObjectPointerType GetNonMovableString(MethodPtr method, StringId id) const override;

    ClassPtr GetStringClass(MethodPtr method) const override;

    /**********************************************************************************/
    /// TLAB information
    size_t GetTLABMaxSize() const override;

    size_t GetTLABAlignment() const override
    {
        return DEFAULT_ALIGNMENT_IN_BYTES;
    }

    bool IsTrackTlabAlloc() const override
    {
        return panda::mem::PANDA_TRACK_TLAB_ALLOCATIONS ||
               Logger::IsLoggingOn(Logger::Level::DEBUG, Logger::Component::MM_OBJECT_EVENTS);
    }

    /**********************************************************************************/
    /// Object information
    ClassPtr GetClass(MethodPtr method, IdType id) const override;

    compiler::ClassType GetClassType(MethodPtr method, IdType id) const override;
    compiler::ClassType GetClassType(ClassPtr klass_ptr) const override;

    bool IsArrayClass(MethodPtr method, IdType id) const override;

    bool IsStringClass(MethodPtr method, IdType id) const override;

    bool IsArrayClass(ClassPtr cls) const override
    {
        return ClassCast(cls)->IsArrayClass();
    }

    ClassPtr GetArrayElementClass(ClassPtr cls) const override;

    bool CheckStoreArray(ClassPtr array_cls, ClassPtr str_cls) const override;

    bool IsAssignableFrom(ClassPtr cls1, ClassPtr cls2) const override;

    size_t GetObjectHashedStatusBitNum() const override
    {
        static_assert(MarkWord::MarkWordRepresentation::STATUS_SIZE == 2);
        static_assert(MarkWord::MarkWordRepresentation::STATUS_HASHED == 2);
        // preconditions above allow just check one bit
        return MarkWord::MarkWordRepresentation::STATUS_SHIFT + 1;
    }

    size_t GetObjectHashShift() const override
    {
        return MarkWord::MarkWordRepresentation::HASH_SHIFT;
    }

    size_t GetObjectHashMask() const override
    {
        return MarkWord::MarkWordRepresentation::HASH_MASK;
    }

    compiler::StringCtorType GetStringCtorType(MethodPtr ctor_method) const override
    {
        return (std::strcmp(utf::Mutf8AsCString(MethodCast(ctor_method)->GetRefArgType(1).data), "[C") == 0)
                   ? compiler::StringCtorType::CHAR_ARRAY
                   : compiler::StringCtorType::STRING;
    }

    /**********************************************************************************/
    /// Class information
    uint8_t GetClassInitializedValue() const override
    {
        return static_cast<uint8_t>(panda::Class::State::INITIALIZED);
    }

    std::optional<IdType> FindClassIdInFile(MethodPtr method, ClassPtr cls) const;
    IdType GetClassIdWithinFile(MethodPtr method, ClassPtr cls) const override;
    IdType GetLiteralArrayClassIdWithinFile(MethodPtr method, panda_file::LiteralTag tag) const override;
    bool CanUseTlabForClass(ClassPtr klass) const override;

    size_t GetClassSize(ClassPtr klass) const override
    {
        return ClassCast(klass)->GetObjectSize();
    }

    bool CanScalarReplaceObject(ClassPtr klass) const override;

    /**********************************************************************************/
    /// Field information

    FieldPtr ResolveField(MethodPtr method, size_t id, bool allow_external, uint32_t *class_id) override;
    compiler::DataType::Type GetFieldType(FieldPtr field) const override;
    compiler::DataType::Type GetFieldTypeById(MethodPtr parent_method, IdType id) const override;
    IdType GetFieldValueTypeId(MethodPtr method, IdType id) const override;
    size_t GetFieldOffset(FieldPtr field) const override;
    FieldPtr GetFieldByOffset(size_t offset) const override;
    uintptr_t GetFieldClass(FieldPtr field) const override;
    bool IsFieldVolatile(FieldPtr field) const override;
    bool HasFieldMetadata(FieldPtr field) const override;

    std::string GetFieldName(FieldPtr field) const override
    {
        return utf::Mutf8AsCString(FieldCast(field)->GetName().data);
    }

    panda::Field *FieldCast(FieldPtr field) const
    {
        ASSERT(HasFieldMetadata(field));
        return static_cast<panda::Field *>(field);
    }

    FieldId GetFieldId(FieldPtr field) const override;

    /**********************************************************************************/
    /// Type information

    PandaRuntimeInterface::ClassPtr ResolveType(MethodPtr method, size_t id) const override;

    bool IsClassInitialized(uintptr_t klass) const override;

    bool IsClassFinal(ClassPtr klass) const override
    {
        return ClassCast(klass)->IsFinal();
    }

    bool IsInterface(ClassPtr klass) const override
    {
        return ClassCast(klass)->IsInterface();
    }

    uintptr_t GetManagedType(uintptr_t klass) const override;

    panda::Class *TypeCast(uintptr_t klass) const
    {
        return reinterpret_cast<panda::Class *>(klass);
    }

    uint8_t GetReferenceTypeMask() const override
    {
        return static_cast<uint8_t>(panda_file::Type::TypeId::REFERENCE);
    }

    /**********************************************************************************/
    /// Entrypoints
    uintptr_t GetIntrinsicAddress(bool runtime_call, SourceLanguage lang, IntrinsicId id) const override;

    /**********************************************************************************/
    /// Dynamic object information

    uint32_t GetFunctionTargetOffset(Arch arch) const override;

    uint64_t GetDynamicPrimitiveUndefined() const override
    {
        return static_cast<uint64_t>(coretypes::TaggedValue::Undefined().GetRawData());
    }

    uint64_t GetDynamicPrimitiveFalse() const override
    {
        return static_cast<uint64_t>(coretypes::TaggedValue::False().GetRawData());
    }

    uint64_t GetDynamicPrimitiveTrue() const override
    {
        return static_cast<uint64_t>(coretypes::TaggedValue::True().GetRawData());
    }

    uint32_t GetNativePointerTargetOffset(Arch arch) const override;

    bool HasSafepointDuringCall() const override;

    bool IsGcValidForFastPath(SourceLanguage lang) const;

    void SetCurrentThread(Thread *thread) const override
    {
        ASSERT(thread != Thread::GetCurrent());
        Thread::SetCurrent(thread);
    }

private:
    static compiler::DataType::Type ToCompilerType(panda_file::Type type)
    {
        switch (type.GetId()) {
            case panda_file::Type::TypeId::VOID:
                return compiler::DataType::VOID;
            case panda_file::Type::TypeId::U1:
                return compiler::DataType::BOOL;
            case panda_file::Type::TypeId::I8:
                return compiler::DataType::INT8;
            case panda_file::Type::TypeId::U8:
                return compiler::DataType::UINT8;
            case panda_file::Type::TypeId::I16:
                return compiler::DataType::INT16;
            case panda_file::Type::TypeId::U16:
                return compiler::DataType::UINT16;
            case panda_file::Type::TypeId::I32:
                return compiler::DataType::INT32;
            case panda_file::Type::TypeId::U32:
                return compiler::DataType::UINT32;
            case panda_file::Type::TypeId::I64:
                return compiler::DataType::INT64;
            case panda_file::Type::TypeId::U64:
                return compiler::DataType::UINT64;
            case panda_file::Type::TypeId::F32:
                return compiler::DataType::FLOAT32;
            case panda_file::Type::TypeId::F64:
                return compiler::DataType::FLOAT64;
            case panda_file::Type::TypeId::REFERENCE:
                return compiler::DataType::REFERENCE;
            case panda_file::Type::TypeId::TAGGED:
                return compiler::DataType::ANY;
            default:
                break;
        }
        UNREACHABLE();
    }

private:
    ClassHierarchyAnalysisWrapper cha_;
    InlineCachesWrapper inline_caches_;
    UnresolvedTypesWrapper unresolved_types_;
};

class Compiler : public CompilerInterface {
public:
    explicit Compiler(CodeAllocator *code_allocator, mem::InternalAllocatorPtr internal_allocator,
                      const RuntimeOptions &options, mem::MemStatsType *mem_stats,
                      compiler::RuntimeInterface *runtime_iface)
        : code_allocator_(code_allocator),
          internal_allocator_(internal_allocator),
          gdb_debug_info_allocator_(panda::SpaceType::SPACE_TYPE_COMPILER, mem_stats),
          runtime_iface_(runtime_iface)
    {
        no_async_jit_ = options.IsNoAsyncJit();
        if (options.IsArkAot()) {
            return;
        }

        if ((Runtime::GetTaskScheduler() == nullptr) || no_async_jit_) {
            compiler_worker_ =
                internal_allocator_->New<CompilerThreadPoolWorker>(internal_allocator_, this, no_async_jit_, options);
        } else {
            compiler_worker_ = internal_allocator_->New<CompilerTaskManagerWorker>(internal_allocator_, this);
        }
        InitializeWorker();
        if (compiler::OPTIONS.WasSetCompilerDumpJitStatsCsv()) {
            jit_stats_ = internal_allocator_->New<compiler::JITStats>(internal_allocator_);
        }
    }

    void PreZygoteFork() override
    {
        FinalizeWorker();
    }

    void PostZygoteFork() override
    {
        InitializeWorker();
    }

    void FinalizeWorker() override
    {
        if (compiler_worker_ != nullptr) {
            compiler_worker_->FinalizeWorker();
        }
    }

    void JoinWorker() override;

    bool IsCompilationExpired(Method *method, bool is_osr);

    ~Compiler() override
    {
        // We need to join thread first if runtime initialization fails and Destroy is not called
        FinalizeWorker();
        if (compiler_worker_ != nullptr) {
            internal_allocator_->Delete(compiler_worker_);
            compiler_worker_ = nullptr;
        }
        internal_allocator_->Delete(jit_stats_);
    }

    bool CompileMethod(Method *method, uintptr_t bytecode_offset, bool osr, TaggedValue func) override;

    virtual void AddTask(CompilerTask &&ctx, [[maybe_unused]] TaggedValue func)
    {
        compiler_worker_->AddTask(std::move(ctx));
    }

    template <compiler::TaskRunnerMode RUNNER_MODE>
    void CompileMethodLocked(compiler::CompilerTaskRunner<RUNNER_MODE> task_runner);

    /// Basic method, which starts compilation. Do not use.
    template <compiler::TaskRunnerMode RUNNER_MODE>
    void StartCompileMethod(compiler::CompilerTaskRunner<RUNNER_MODE> task_runner);

    void ScaleThreadPool(size_t number_of_threads)
    {
        // Required for testing
        GetThreadPool()->Scale(number_of_threads);
    }

    void *GetOsrCode(const Method *method) override
    {
        return osr_code_map_.Get(method);
    }

    void SetOsrCode(const Method *method, void *ptr) override
    {
        osr_code_map_.Set(method, ptr);
    }

    void RemoveOsrCode(const Method *method) override
    {
        osr_code_map_.Remove(method);
    }

    void SetNoAsyncJit(bool v) override
    {
        no_async_jit_ = v;
    }

    bool IsNoAsyncJit() override
    {
        return no_async_jit_;
    }

    compiler::RuntimeInterface *GetRuntimeInterface()
    {
        return runtime_iface_;
    }

protected:
    mem::InternalAllocatorPtr GetInternalAllocator()
    {
        return internal_allocator_;
    }

    ThreadPool<CompilerTask, CompilerProcessor, Compiler *> *GetThreadPool()
    {
        ASSERT(Runtime::GetTaskScheduler() == nullptr || no_async_jit_);
        if (compiler_worker_ != nullptr) {
            return static_cast<CompilerThreadPoolWorker *>(compiler_worker_)->GetThreadPool();
        }
        return nullptr;
    }

private:
    void InitializeWorker()
    {
        if (compiler_worker_ != nullptr) {
            compiler_worker_->InitializeWorker();
        }
    }

    CodeAllocator *code_allocator_;
    OsrCodeMap osr_code_map_;
    mem::InternalAllocatorPtr internal_allocator_;
    // This allocator is used for GDB debug structures in context of JIT unwind info.
    ArenaAllocator gdb_debug_info_allocator_;
    compiler::RuntimeInterface *runtime_iface_;
    // The lock is used for compiler thread synchronization
    os::memory::Mutex compilation_lock_;
    bool no_async_jit_;
    CompilerWorker *compiler_worker_ {nullptr};
    compiler::JITStats *jit_stats_ {nullptr};
    NO_COPY_SEMANTIC(Compiler);
    NO_MOVE_SEMANTIC(Compiler);
};

#ifndef PANDA_PRODUCT_BUILD
uint8_t CompileMethodImpl(coretypes::String *full_method_name, panda_file::SourceLang source_lang);
#endif

}  // namespace panda

#endif  // PANDA_RUNTIME_COMPILER_H_
