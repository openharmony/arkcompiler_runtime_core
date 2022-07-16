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

#include "runtime/compiler.h"

#include "intrinsics.h"
#include "libpandafile/bytecode_instruction.h"
#include "libpandafile/type_helper.h"
#include "runtime/cha.h"
#include "runtime/jit/profiling_data.h"
#include "runtime/include/class_linker-inl.h"
#include "runtime/include/exceptions.h"
#include "runtime/include/field.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread.h"
#include "runtime/include/coretypes/native_pointer.h"
#include "runtime/mem/heap_manager.h"

namespace panda {

#ifdef PANDA_COMPILER_CFI
namespace compiler {
void CleanJitDebugCode();
}  // namespace compiler
#endif

#include <get_intrinsics.inl>

class ErrorHandler : public ClassLinkerErrorHandler {
    void OnError([[maybe_unused]] ClassLinker::Error error, [[maybe_unused]] const PandaString &message) override {}
};

bool Compiler::IsCompilationExpired(const CompilerTask &ctx)
{
    return (ctx.IsOsr() && GetOsrCode(ctx.GetMethod()) != nullptr) ||
           (!ctx.IsOsr() && ctx.GetMethod()->HasCompiledCode());
}

CompilerProcessor::CompilerProcessor(Compiler *compiler)
{
    compiler_ = compiler;
}

bool CompilerProcessor::Process(CompilerTask task)
{
    Method::CompilationStage status = task.GetMethod()->GetCompilationStatus();
    if (status == Method::WAITING) {
        // It is possible when main thread compile the mathod without queue
        if (task.GetMethod()->AtomicSetCompilationStatus(Method::WAITING, Method::COMPILATION)) {
            compiler_->CompileMethodLocked(task);
        }
    }
    return true;
}

compiler::RuntimeInterface::MethodId PandaRuntimeInterface::ResolveMethodIndex(MethodPtr parent_method,
                                                                               MethodIndex index) const
{
    return MethodCast(parent_method)->GetClass()->ResolveMethodIndex(index).GetOffset();
}

compiler::RuntimeInterface::FieldId PandaRuntimeInterface::ResolveFieldIndex(MethodPtr parent_method,
                                                                             FieldIndex index) const
{
    return MethodCast(parent_method)->GetClass()->ResolveFieldIndex(index).GetOffset();
}

compiler::RuntimeInterface::IdType PandaRuntimeInterface::ResolveTypeIndex(MethodPtr parent_method,
                                                                           TypeIndex index) const
{
    return MethodCast(parent_method)->GetClass()->ResolveClassIndex(index).GetOffset();
}

compiler::RuntimeInterface::MethodPtr PandaRuntimeInterface::GetMethodById(MethodPtr parent_method, MethodId id) const
{
    ScopedMutatorLock lock;
    ErrorHandler error_handler;
    return Runtime::GetCurrent()->GetClassLinker()->GetMethod(*MethodCast(parent_method),
                                                              panda_file::File::EntityId(id), &error_handler);
}

compiler::RuntimeInterface::MethodId PandaRuntimeInterface::GetMethodId(MethodPtr method) const
{
    return MethodCast(method)->GetFileId().GetOffset();
}

compiler::RuntimeInterface::IntrinsicId PandaRuntimeInterface::GetIntrinsicId(MethodPtr method) const
{
    return GetIntrinsicEntryPointId(MethodCast(method)->GetIntrinsic());
}

compiler::RuntimeInterface::MethodPtr PandaRuntimeInterface::ResolveVirtualMethod(ClassPtr cls, MethodPtr method) const
{
    ScopedMutatorLock lock;
    ASSERT(method != nullptr);
    return ClassCast(cls)->ResolveVirtualMethod(MethodCast(method));
}

compiler::RuntimeInterface::MethodPtr PandaRuntimeInterface::ResolveInterfaceMethod(ClassPtr cls,
                                                                                    MethodPtr method) const
{
    ScopedMutatorLock lock;
    ASSERT(method != nullptr);
    return ClassCast(cls)->ResolveVirtualMethod(MethodCast(method));
}

compiler::RuntimeInterface::ClassPtr PandaRuntimeInterface::GetClass(MethodPtr method, IdType id) const
{
    ScopedMutatorLock lock;
    auto *caller = MethodCast(method);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*caller);
    ErrorHandler handler;
    return Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClass(
        *caller->GetPandaFile(), panda_file::File::EntityId(id), caller->GetClass()->GetLoadContext(), &handler);
}

compiler::RuntimeInterface::ClassPtr PandaRuntimeInterface::GetStringClass(MethodPtr method) const
{
    ScopedMutatorLock lock;
    auto *caller = MethodCast(method);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*caller);
    return Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::STRING);
}

compiler::ClassType PandaRuntimeInterface::GetClassType(MethodPtr method, IdType id) const
{
    if (method == nullptr) {
        return compiler::ClassType::UNRESOLVED_CLASS;
    }
    ScopedMutatorLock lock;
    auto *caller = MethodCast(method);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*caller);
    ErrorHandler handler;
    auto klass = Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->GetClass(
        *caller->GetPandaFile(), panda_file::File::EntityId(id), caller->GetClass()->GetLoadContext(), &handler);
    if (klass == nullptr) {
        return compiler::ClassType::UNRESOLVED_CLASS;
    }
    if (klass->IsObjectClass()) {
        return compiler::ClassType::OBJECT_CLASS;
    }
    if (klass->IsInterface()) {
        return compiler::ClassType::INTERFACE_CLASS;
    }
    if (klass->IsArrayClass()) {
        auto component_class = klass->GetComponentType();
        ASSERT(component_class != nullptr);
        if (component_class->IsObjectClass()) {
            return compiler::ClassType::ARRAY_OBJECT_CLASS;
        }
        if (component_class->IsPrimitive()) {
            return compiler::ClassType::FINAL_CLASS;
        }
        return compiler::ClassType::ARRAY_CLASS;
    }
    if (klass->IsFinal()) {
        return compiler::ClassType::FINAL_CLASS;
    }
    return compiler::ClassType::OTHER_CLASS;
}

bool PandaRuntimeInterface::IsArrayClass(MethodPtr method, IdType id) const
{
    panda_file::File::EntityId cid(id);
    auto *pf = MethodCast(method)->GetPandaFile();
    return ClassHelper::IsArrayDescriptor(pf->GetStringData(cid).data);
}

compiler::RuntimeInterface::ClassPtr PandaRuntimeInterface::GetArrayElementClass(ClassPtr cls) const
{
    ScopedMutatorLock lock;
    ASSERT(ClassCast(cls)->IsArrayClass());
    return ClassCast(cls)->GetComponentType();
}

bool PandaRuntimeInterface::CheckStoreArray(ClassPtr array_cls, ClassPtr str_cls) const
{
    ASSERT(array_cls != nullptr);
    auto *element_class = ClassCast(array_cls)->GetComponentType();
    if (str_cls == nullptr) {
        return element_class->IsObjectClass();
    }
    ASSERT(str_cls != nullptr);
    return element_class->IsAssignableFrom(ClassCast(str_cls));
}

bool PandaRuntimeInterface::IsAssignableFrom(ClassPtr cls1, ClassPtr cls2) const
{
    ASSERT(cls1 != nullptr);
    ASSERT(cls2 != nullptr);
    return ClassCast(cls1)->IsAssignableFrom(ClassCast(cls2));
}

bool PandaRuntimeInterface::IsInterfaceMethod(MethodPtr parent_method, MethodId id) const
{
    ScopedMutatorLock lock;
    ErrorHandler handler;
    auto method = Runtime::GetCurrent()->GetClassLinker()->GetMethod(*MethodCast(parent_method),
                                                                     panda_file::File::EntityId(id), &handler);
    return (method->GetClass()->IsInterface() && !method->IsDefaultInterfaceMethod());
}

bool PandaRuntimeInterface::CanThrowException(MethodPtr method) const
{
    ScopedMutatorLock lock;
    auto *panda_method = MethodCast(method);
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(*panda_method);
    return Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->CanThrowException(panda_method);
}
bool PandaRuntimeInterface::IsInterfaceMethod(MethodPtr method) const
{
    ScopedMutatorLock lock;
    return (MethodCast(method)->GetClass()->IsInterface() && !MethodCast(method)->IsDefaultInterfaceMethod());
}

bool PandaRuntimeInterface::HasNativeException(MethodPtr method) const
{
    if (!MethodCast(method)->IsNative()) {
        return false;
    }
    return CanThrowException(method);
}

bool PandaRuntimeInterface::IsMethodExternal(MethodPtr parent_method, MethodPtr callee_method) const
{
    if (callee_method == nullptr) {
        return true;
    }
    return MethodCast(parent_method)->GetPandaFile() != MethodCast(callee_method)->GetPandaFile();
}

compiler::DataType::Type PandaRuntimeInterface::GetMethodReturnType(MethodPtr parent_method, MethodId id) const
{
    auto *pf = MethodCast(parent_method)->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, panda_file::File::EntityId(id));
    panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
    return ToCompilerType(panda_file::GetEffectiveType(pda.GetReturnType()));
}

compiler::DataType::Type PandaRuntimeInterface::GetMethodArgumentType(MethodPtr parent_method, MethodId id,
                                                                      size_t index) const
{
    auto *pf = MethodCast(parent_method)->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, panda_file::File::EntityId(id));
    panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
    return ToCompilerType(panda_file::GetEffectiveType(pda.GetArgType(index)));
}

size_t PandaRuntimeInterface::GetMethodArgumentsCount(MethodPtr parent_method, MethodId id) const
{
    auto *pf = MethodCast(parent_method)->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, panda_file::File::EntityId(id));
    panda_file::ProtoDataAccessor pda(*pf, mda.GetProtoId());
    return pda.GetNumArgs();
}

bool PandaRuntimeInterface::IsMethodStatic(MethodPtr parent_method, MethodId id) const
{
    auto *pf = MethodCast(parent_method)->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, panda_file::File::EntityId(id));
    return mda.IsStatic();
}

bool PandaRuntimeInterface::IsMethodStatic(MethodPtr method) const
{
    return MethodCast(method)->IsStatic();
}

bool PandaRuntimeInterface::IsMethodStaticConstructor(MethodPtr method) const
{
    return MethodCast(method)->IsStaticConstructor();
}

bool PandaRuntimeInterface::IsMemoryBarrierRequired(MethodPtr method) const
{
    if (!MethodCast(method)->IsInstanceConstructor()) {
        return false;
    }
    for (auto &field : MethodCast(method)->GetClass()->GetFields()) {
        if (field.IsFinal()) {
            return true;
        }
    }
    return false;
}

bool PandaRuntimeInterface::IsMethodIntrinsic(MethodPtr parent_method, MethodId id) const
{
    Method *caller = MethodCast(parent_method);
    auto *pf = caller->GetPandaFile();
    panda_file::MethodDataAccessor mda(*pf, panda_file::File::EntityId(id));

    auto *class_name = pf->GetStringData(mda.GetClassId()).data;
    auto *class_linker = Runtime::GetCurrent()->GetClassLinker();

    auto *klass = class_linker->FindLoadedClass(class_name, caller->GetClass()->GetLoadContext());

    // Class should be loaded during intrinsics initialization
    if (klass == nullptr) {
        return false;
    }

    auto name = pf->GetStringData(mda.GetNameId());
    bool is_array_clone = ClassHelper::IsArrayDescriptor(class_name) &&
                          (utf::CompareMUtf8ToMUtf8(name.data, utf::CStringAsMutf8("clone")) == 0);
    Method::Proto proto(*pf, mda.GetProtoId());
    auto *method = klass->GetDirectMethod(name.data, proto);
    if (method == nullptr) {
        if (is_array_clone) {
            method = klass->GetClassMethod(name.data, proto);
        } else {
            return false;
        }
    }

    return method->IsIntrinsic();
}

std::string PandaRuntimeInterface::GetBytecodeString(MethodPtr method, uintptr_t pc) const
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    BytecodeInstruction inst(MethodCast(method)->GetInstructions() + pc);
    std::stringstream ss;
    ss << inst;
    return ss.str();
}

PandaRuntimeInterface::FieldPtr PandaRuntimeInterface::ResolveField(PandaRuntimeInterface::MethodPtr m, size_t id,
                                                                    bool allow_external, uint32_t *pclass_id)
{
    ScopedMutatorLock lock;
    ErrorHandler handler;
    auto method = MethodCast(m);
    auto pfile = method->GetPandaFile();
    auto field = Runtime::GetCurrent()->GetClassLinker()->GetField(*method, panda_file::File::EntityId(id), &handler);
    if (field == nullptr) {
        return nullptr;
    }
    auto klass = field->GetClass();
    if (pfile == field->GetPandaFile() || allow_external) {
        if (pclass_id != nullptr) {
            *pclass_id = klass->GetFileId().GetOffset();
        }
        return field;
    }

    auto class_id = GetClassIdWithinFile(m, klass);
    if (class_id != 0) {
        if (pclass_id != nullptr) {
            *pclass_id = class_id;
        }
        return field;
    }
    return nullptr;
}

template <typename T>
void FillLiteralArrayData(const panda_file::File *pfile, pandasm::LiteralArray *lit_array,
                          const panda_file::LiteralTag &tag, const panda_file::LiteralDataAccessor::LiteralValue &value)
{
    panda_file::File::EntityId id(std::get<uint32_t>(value));
    auto sp = pfile->GetSpanFromId(id);
    auto len = panda_file::helpers::Read<sizeof(uint32_t)>(&sp);

    for (size_t i = 0; i < len; i++) {
        pandasm::LiteralArray::Literal lit;
        lit.tag_ = tag;
        lit.value_ = bit_cast<T>(panda_file::helpers::Read<sizeof(T)>(&sp));
        lit_array->literals_.push_back(lit);
    }
}

panda::pandasm::LiteralArray PandaRuntimeInterface::GetLiteralArray(MethodPtr m, LiteralArrayId id) const
{
    auto method = MethodCast(m);
    auto pfile = method->GetPandaFile();
    pandasm::LiteralArray lit_array;

    panda_file::LiteralDataAccessor lit_array_accessor(*pfile, pfile->GetLiteralArraysId());
    lit_array_accessor.EnumerateLiteralVals(
        panda_file::File::EntityId(id), [&lit_array, pfile](const panda_file::LiteralDataAccessor::LiteralValue &value,
                                                            const panda_file::LiteralTag &tag) {
            switch (tag) {
                case panda_file::LiteralTag::ARRAY_U1: {
                    FillLiteralArrayData<bool>(pfile, &lit_array, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_I8:
                case panda_file::LiteralTag::ARRAY_U8: {
                    FillLiteralArrayData<uint8_t>(pfile, &lit_array, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_I16:
                case panda_file::LiteralTag::ARRAY_U16: {
                    FillLiteralArrayData<uint16_t>(pfile, &lit_array, tag, value);
                    break;
                }
                // in the case of ARRAY_STRING, the array stores strings ids
                case panda_file::LiteralTag::ARRAY_STRING:
                case panda_file::LiteralTag::ARRAY_I32:
                case panda_file::LiteralTag::ARRAY_U32: {
                    FillLiteralArrayData<uint32_t>(pfile, &lit_array, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_I64:
                case panda_file::LiteralTag::ARRAY_U64: {
                    FillLiteralArrayData<uint64_t>(pfile, &lit_array, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_F32: {
                    FillLiteralArrayData<float>(pfile, &lit_array, tag, value);
                    break;
                }
                case panda_file::LiteralTag::ARRAY_F64: {
                    FillLiteralArrayData<double>(pfile, &lit_array, tag, value);
                    break;
                }
                case panda_file::LiteralTag::TAGVALUE:
                case panda_file::LiteralTag::ACCESSOR:
                case panda_file::LiteralTag::NULLVALUE: {
                    break;
                }
                default: {
                    UNREACHABLE();
                    break;
                }
            }
        });
    return lit_array;
}

std::optional<RuntimeInterface::IdType> PandaRuntimeInterface::FindClassIdInFile(MethodPtr method, ClassPtr cls) const
{
    auto klass = ClassCast(cls);
    auto pfile = MethodCast(method)->GetPandaFile();
    auto class_name = klass->GetName();
    PandaString storage;
    auto class_id = pfile->GetClassId(ClassHelper::GetDescriptor(utf::CStringAsMutf8(class_name.c_str()), &storage));
    if (class_id.IsValid() && class_name == ClassHelper::GetName(pfile->GetStringData(class_id).data)) {
        return std::optional<RuntimeInterface::IdType>(class_id.GetOffset());
    }
    return std::nullopt;
}

RuntimeInterface::IdType PandaRuntimeInterface::GetClassIdWithinFile(MethodPtr method, ClassPtr cls) const
{
    auto class_id = FindClassIdInFile(method, cls);
    return class_id ? class_id.value() : 0;
}

RuntimeInterface::IdType PandaRuntimeInterface::GetLiteralArrayClassIdWithinFile(
    PandaRuntimeInterface::MethodPtr method, panda_file::LiteralTag tag) const
{
    ScopedMutatorLock lock;
    ErrorHandler handler;
    auto ctx = Runtime::GetCurrent()->GetLanguageContext(*MethodCast(method));
    auto cls = Runtime::GetCurrent()->GetClassRootForLiteralTag(
        *Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx), tag);

    auto pfile = MethodCast(method)->GetPandaFile();
    auto class_name = cls->GetName();
    for (decltype(auto) class_raw_id : pfile->GetClasses()) {
        auto class_id = panda_file::File::EntityId(class_raw_id);
        if (class_id.IsValid() && class_name == ClassHelper::GetName(pfile->GetStringData(class_id).data)) {
            return class_id.GetOffset();
        }
    }
    UNREACHABLE();
}

bool PandaRuntimeInterface::CanUseTlabForClass(ClassPtr klass) const
{
    auto cls = ClassCast(klass);
    return !Thread::GetCurrent()->GetVM()->GetHeapManager()->IsObjectFinalized(cls) && !cls->IsVariableSize() &&
           cls->IsInstantiable();
}

size_t PandaRuntimeInterface::GetTLABMaxSize() const
{
    return Thread::GetCurrent()->GetVM()->GetHeapManager()->GetTLABMaxAllocSize();
}

PandaRuntimeInterface::ClassPtr PandaRuntimeInterface::ResolveType(PandaRuntimeInterface::MethodPtr method,
                                                                   size_t id) const
{
    ScopedMutatorLock lock;
    ErrorHandler handler;
    auto klass = Runtime::GetCurrent()->GetClassLinker()->GetClass(*MethodCast(method), panda_file::File::EntityId(id),
                                                                   &handler);
    return klass;
}

bool PandaRuntimeInterface::IsClassInitialized(uintptr_t klass) const
{
    return TypeCast(klass)->IsInitialized();
}

uintptr_t PandaRuntimeInterface::GetManagedType(uintptr_t klass) const
{
    return reinterpret_cast<uintptr_t>(TypeCast(klass)->GetManagedObject());
}

compiler::DataType::Type PandaRuntimeInterface::GetFieldType(FieldPtr field) const
{
    return ToCompilerType(FieldCast(field)->GetType());
}

compiler::DataType::Type PandaRuntimeInterface::GetFieldTypeById(MethodPtr parent_method, IdType id) const
{
    auto *pf = MethodCast(parent_method)->GetPandaFile();
    panda_file::FieldDataAccessor fda(*pf, panda_file::File::EntityId(id));
    return ToCompilerType(panda_file::Type::GetTypeFromFieldEncoding(fda.GetType()));
}

RuntimeInterface::ClassPtr PandaRuntimeInterface::GetClassForField(FieldPtr field) const
{
    return FieldCast(field)->GetClass();
}

uint32_t PandaRuntimeInterface::GetArrayElementSize(MethodPtr method, IdType id) const
{
    auto *pf = MethodCast(method)->GetPandaFile();
    auto *descriptor = pf->GetStringData(panda_file::File::EntityId(id)).data;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ASSERT(descriptor[0] == '[');
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return Class::GetTypeSize(panda_file::Type::GetTypeIdBySignature(static_cast<char>(descriptor[1])));
}

uintptr_t PandaRuntimeInterface::GetPointerToConstArrayData(MethodPtr method, IdType id) const
{
    auto *pf = MethodCast(method)->GetPandaFile();
    return Runtime::GetCurrent()->GetPointerToConstArrayData(*pf, id);
}

size_t PandaRuntimeInterface::GetOffsetToConstArrayData(MethodPtr method, IdType id) const
{
    auto *pf = MethodCast(method)->GetPandaFile();
    auto offset =
        Runtime::GetCurrent()->GetPointerToConstArrayData(*pf, id) - reinterpret_cast<uintptr_t>(pf->GetBase());
    return static_cast<size_t>(offset);
}

size_t PandaRuntimeInterface::GetFieldOffset(FieldPtr field) const
{
    if (!HasFieldMetadata(field)) {
        return reinterpret_cast<uintptr_t>(field) >> 1U;
    }
    return FieldCast(field)->GetOffset();
}

RuntimeInterface::FieldPtr PandaRuntimeInterface::GetFieldByOffset(size_t offset) const
{
    ASSERT(MinimumBitsToStore(offset) < std::numeric_limits<uintptr_t>::digits);
    return reinterpret_cast<FieldPtr>((offset << 1U) | 1U);
}

uintptr_t PandaRuntimeInterface::GetFieldClass(FieldPtr field) const
{
    return reinterpret_cast<uintptr_t>(FieldCast(field)->GetClass());
}

bool PandaRuntimeInterface::IsFieldVolatile(FieldPtr field) const
{
    return FieldCast(field)->IsVolatile();
}

bool PandaRuntimeInterface::HasFieldMetadata(FieldPtr field) const
{
    return (reinterpret_cast<uintptr_t>(field) & 1U) == 0;
}

panda::mem::BarrierType PandaRuntimeInterface::GetPreType() const
{
    return Thread::GetCurrent()->GetBarrierSet()->GetPreType();
}

panda::mem::BarrierType PandaRuntimeInterface::GetPostType() const
{
    return Thread::GetCurrent()->GetBarrierSet()->GetPostType();
}

panda::mem::BarrierOperand PandaRuntimeInterface::GetBarrierOperand(panda::mem::BarrierPosition barrier_position,
                                                                    std::string_view operand_name) const
{
    return Thread::GetCurrent()->GetBarrierSet()->GetBarrierOperand(barrier_position, operand_name);
}

uint32_t PandaRuntimeInterface::GetFunctionTargetOffset([[maybe_unused]] Arch arch) const
{
    // TODO(wengchangcheng): return offset of method in JSFunction
    return 0;
}

uint32_t PandaRuntimeInterface::GetNativePointerTargetOffset(Arch arch) const
{
    return cross_values::GetCoretypesNativePointerExternalPointerOffset(arch);
}

void ClassHierarchyAnalysisWrapper::AddDependency(PandaRuntimeInterface::MethodPtr callee,
                                                  RuntimeInterface::MethodPtr caller)
{
    Runtime::GetCurrent()->GetCha()->AddDependency(MethodCast(callee), MethodCast(caller));
}

/**
 * With 'no-async-jit' compilation inside of c2i bridge can forced and it can trigger GC
 */
bool PandaRuntimeInterface::HasSafepointDuringCall() const
{
#ifdef PANDA_PRODUCT_BUILD
    return false;
#else
    if (Runtime::GetOptions().IsArkAot()) {
        return false;
    }
    return Runtime::GetOptions().IsNoAsyncJit();
#endif
}

InlineCachesWrapper::CallKind InlineCachesWrapper::GetClasses(PandaRuntimeInterface::MethodPtr m, uintptr_t pc,
                                                              ArenaVector<RuntimeInterface::ClassPtr> *classes)
{
    ASSERT(classes != nullptr);
    classes->clear();
    auto method = static_cast<Method *>(m);
    auto profiling_data = method->GetProfilingData();
    if (profiling_data == nullptr) {
        return CallKind::UNKNOWN;
    }
    auto ic = profiling_data->FindInlineCache(pc);
    if (ic == nullptr) {
        return CallKind::UNKNOWN;
    }
    auto ic_classes = ic->GetClassesCopy();
    classes->insert(classes->end(), ic_classes.begin(), ic_classes.end());
    if (classes->empty()) {
        return CallKind::UNKNOWN;
    }
    if (classes->size() == 1) {
        return CallKind::MONOMORPHIC;
    }
    if (CallSiteInlineCache::IsMegamorphic(reinterpret_cast<Class *>((*classes)[0]))) {
        return CallKind::MEGAMORPHIC;
    }
    return CallKind::POLYMORPHIC;
}

bool UnresolvedTypesWrapper::AddTableSlot(RuntimeInterface::MethodPtr method, uint32_t type_id, SlotKind kind)
{
    std::pair<uint32_t, UnresolvedTypesInterface::SlotKind> key {type_id, kind};
    if (slots_.find(method) == slots_.end()) {
        slots_[method][key] = 0;
        return true;
    }
    auto &table = slots_.at(method);
    if (table.find(key) == table.end()) {
        table[key] = 0;
        return true;
    }
    return false;
}

uintptr_t UnresolvedTypesWrapper::GetTableSlot(RuntimeInterface::MethodPtr method, uint32_t type_id,
                                               SlotKind kind) const
{
    ASSERT(slots_.find(method) != slots_.end());
    auto &table = slots_.at(method);
    ASSERT(table.find({type_id, kind}) != table.end());
    return reinterpret_cast<uintptr_t>(&table.at({type_id, kind}));
}

bool Compiler::CompileMethod(Method *method, uintptr_t bytecode_offset, bool osr)
{
    if (method->IsAbstract()) {
        return false;
    }

    if (osr && GetOsrCode(method) != nullptr) {
        ASSERT(method == ManagedThread::GetCurrent()->GetCurrentFrame()->GetMethod());
        ASSERT(method->HasCompiledCode());
        return OsrEntry(bytecode_offset, GetOsrCode(method));
    }
    // In case if some thread raise compilation when another already compiled it, we just exit.
    if (method->HasCompiledCode() && !osr) {
        return false;
    }
    CompilerTask ctx {method, method->HasCompiledCode() ? osr : false, ManagedThread::GetCurrent()->GetVM()};
    CompileMethodAsync(ctx);
    if (no_async_jit_) {
        auto status = method->GetCompilationStatus();
        for (; (status == Method::WAITING) || (status == Method::COMPILATION);
             status = method->GetCompilationStatus()) {
            auto thread = MTManagedThread::GetCurrent();
            // TODO(asoldatov): Remove this workaround for invoking compiler from ECMA VM
            if (thread != nullptr) {
                static constexpr uint64_t SLEEP_MS = 10;
                thread->TimedWait(ThreadStatus::IS_COMPILER_WAITING, SLEEP_MS, 0);
            }
        }
    }
    return false;
}

void Compiler::CompileMethodLocked(const CompilerTask &ctx)
{
    ASSERT(runtime_iface_ != nullptr);
    auto method = ctx.GetMethod();
    os::memory::LockHolder lock(compilation_lock_);

    method->ResetHotnessCounter();

    if (IsCompilationExpired(ctx)) {
        return;
    }

    PandaVM *vm = ctx.GetVM();

    // Set current thread to have access to vm during compilation
    Thread compiler_thread(vm, Thread::ThreadType::THREAD_TYPE_COMPILER);
    ScopedCurrentThread sct(&compiler_thread);

    mem::MemStatsType *mem_stats = vm->GetMemStats();
    panda::ArenaAllocator allocator(panda::SpaceType::SPACE_TYPE_COMPILER, mem_stats);
    panda::ArenaAllocator graph_local_allocator(panda::SpaceType::SPACE_TYPE_COMPILER, mem_stats, true);
    if (!compiler::JITCompileMethod(runtime_iface_, method, ctx.IsOsr(), code_allocator_, &allocator,
                                    &graph_local_allocator, &gdb_debug_info_allocator_)) {
        // Failure during compilation, should we retry later?
        method->SetCompilationStatus(Method::FAILED);
        return;
    }
    method->SetCompilationStatus(Method::COMPILED);
}

class ScopedDisableManagedCode {
public:
    explicit ScopedDisableManagedCode(ManagedThread *thread)
        : thread_(thread), was_allowed_(thread->IsManagedCodeAllowed())
    {
        thread_->SetManagedCodeAllowed(false);
    }

    ~ScopedDisableManagedCode()
    {
        thread_->SetManagedCodeAllowed(was_allowed_);
    }

    NO_COPY_SEMANTIC(ScopedDisableManagedCode);
    NO_MOVE_SEMANTIC(ScopedDisableManagedCode);

private:
    ManagedThread *thread_;
    bool was_allowed_;
};

void Compiler::CompileMethodSync(const CompilerTask &ctx)
{
    auto method = ctx.GetMethod();
    Method::CompilationStage status = method->GetCompilationStatus();
    // The main thread compiles the method even if it is inside waiting queue
    // Note, search inside the queue may be inefficient, so just wait untill it will be handled by worker thread
    // TODO: in future with more smart queue, we should update it here
    if (method->AtomicSetCompilationStatus(status, Method::COMPILATION)) {
        ScopedDisableManagedCode sdmc(ManagedThread::GetCurrent());
        CompileMethodLocked(ctx);
    }
}

void Compiler::CompileMethodAsync(const CompilerTask &ctx)
{
    // We should enter here only with NOT_COMPILED, so avoid extra check
    if (ctx.GetMethod()->AtomicSetCompilationStatus(ctx.IsOsr() ? Method::COMPILED : Method::NOT_COMPILED,
                                                    Method::WAITING)) {
        AddTask(ctx);
    }
}

void Compiler::JoinWorker()
{
    is_thread_pool_created_ = false;
    if (thread_pool_ != nullptr) {
        internal_allocator_->Delete(thread_pool_);
        thread_pool_ = nullptr;
    }
#ifdef PANDA_COMPILER_CFI
    if (!Runtime::GetOptions().IsArkAot() && compiler::options.IsCompilerEmitDebugInfo()) {
        compiler::CleanJitDebugCode();
    }
#endif
}

object_pointer_type PandaRuntimeInterface::GetNonMovableString(MethodPtr method, StringId id) const
{
    auto vm = Runtime::GetCurrent()->GetPandaVM();
    auto pf = MethodCast(method)->GetPandaFile();
    return ToObjPtrType(vm->GetNonMovableString(*pf, panda_file::File::EntityId {id}));
}

}  // namespace panda
