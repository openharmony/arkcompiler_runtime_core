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

#ifdef PANDA_COMPILER_DEBUG_INFO
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

/// Intrinsics fast paths are supported only for G1 GC.
bool PandaRuntimeInterface::IsGcValidForFastPath(SourceLanguage lang) const
{
    auto runtime = Runtime::GetCurrent();
    if (lang == SourceLanguage::INVALID) {
        lang = ManagedThread::GetCurrent()->GetThreadLang();
    }
    auto gc_type = runtime->GetGCType(runtime->GetOptions(), lang);
    return gc_type == mem::GCType::G1_GC;
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

uint64_t PandaRuntimeInterface::GetUniqMethodId(MethodPtr method) const
{
    return MethodCast(method)->GetUniqId();
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

compiler::RuntimeInterface::IdType PandaRuntimeInterface::GetMethodReturnTypeId(MethodPtr method) const
{
    auto *pf = MethodCast(method)->GetPandaFile();
    panda_file::ProtoDataAccessor pda(*pf,
                                      panda_file::MethodDataAccessor::GetProtoId(*pf, MethodCast(method)->GetFileId()));
    return pda.GetReferenceType(0).GetOffset();
}

compiler::RuntimeInterface::IdType PandaRuntimeInterface::GetMethodArgReferenceTypeId(MethodPtr method,
                                                                                      uint16_t num) const
{
    auto *pf = MethodCast(method)->GetPandaFile();
    panda_file::ProtoDataAccessor pda(*pf,
                                      panda_file::MethodDataAccessor::GetProtoId(*pf, MethodCast(method)->GetFileId()));
    return pda.GetReferenceType(num).GetOffset();
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

compiler::ClassType PandaRuntimeInterface::GetClassType(ClassPtr klass_ptr) const
{
    if (klass_ptr == nullptr) {
        return compiler::ClassType::UNRESOLVED_CLASS;
    }
    auto klass = ClassCast(klass_ptr);
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

bool PandaRuntimeInterface::IsStringClass(MethodPtr method, IdType id) const
{
    auto cls = GetClass(method, id);
    if (cls == nullptr) {
        return false;
    }
    return ClassCast(cls)->IsStringClass();
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

uint32_t PandaRuntimeInterface::FindCatchBlock(MethodPtr m, ClassPtr cls, uint32_t pc) const
{
    ScopedMutatorLock lock;
    return MethodCast(m)->FindCatchBlockInPandaFile(ClassCast(cls), pc);
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
        // We insert memory barrier after call to constructor to ensure writes
        // to final fields will be visible after constructor finishes
        // Static fields are initialized in runtime entrypoints like InitializeClass,
        // so barrier is not needed here if they are final
        if (field.IsFinal() && !field.IsStatic()) {
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
        lit.tag = tag;
        lit.value = bit_cast<T>(panda_file::helpers::Read<sizeof(T)>(&sp));
        lit_array->literals.push_back(lit);
    }
}

panda::pandasm::LiteralArray PandaRuntimeInterface::GetLiteralArray(MethodPtr m, LiteralArrayId id) const
{
    auto method = MethodCast(m);
    auto pfile = method->GetPandaFile();
    id = pfile->GetLiteralArrays()[id];
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

bool PandaRuntimeInterface::CanScalarReplaceObject(ClassPtr klass) const
{
    auto cls = ClassCast(klass);
    auto ctx = Runtime::GetCurrent()->GetLanguageContext(*cls);
    return Runtime::GetCurrent()->GetClassLinker()->GetExtension(ctx)->CanScalarReplaceObject(cls);
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

compiler::RuntimeInterface::IdType PandaRuntimeInterface::GetFieldValueTypeId(MethodPtr method, IdType id) const
{
    auto *pf = MethodCast(method)->GetPandaFile();
    auto type_id = panda_file::FieldDataAccessor::GetTypeId(*pf, panda_file::File::EntityId(id));
    return type_id.GetOffset();
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

uint32_t PandaRuntimeInterface::GetMaxArrayLength(ClassPtr klass) const
{
    ScopedMutatorLock lock;
    if (ClassCast(klass)->IsArrayClass()) {
        return INT32_MAX / ClassCast(klass)->GetComponentSize();
    }
    return INT32_MAX;
}

uintptr_t PandaRuntimeInterface::GetPointerToConstArrayData(MethodPtr method, IdType id) const
{
    auto *pf = MethodCast(method)->GetPandaFile();
    return Runtime::GetCurrent()->GetPointerToConstArrayData(*pf, pf->GetLiteralArrays()[id]);
}

size_t PandaRuntimeInterface::GetOffsetToConstArrayData(MethodPtr method, IdType id) const
{
    auto *pf = MethodCast(method)->GetPandaFile();
    auto offset = Runtime::GetCurrent()->GetPointerToConstArrayData(*pf, pf->GetLiteralArrays()[id]) -
                  reinterpret_cast<uintptr_t>(pf->GetBase());
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

RuntimeInterface::FieldId PandaRuntimeInterface::GetFieldId(FieldPtr field) const
{
    return FieldCast(field)->GetFileId().GetOffset();
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

/// With 'no-async-jit' compilation inside of c2i bridge can forced and it can trigger GC
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

bool Compiler::CompileMethod(Method *method, uintptr_t bytecode_offset, bool osr, TaggedValue func)
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
    bool ctx_osr = method->HasCompiledCode() ? osr : false;
    if (method->AtomicSetCompilationStatus(ctx_osr ? Method::COMPILED : Method::NOT_COMPILED, Method::WAITING)) {
        CompilerTask ctx {method, ctx_osr, ManagedThread::GetCurrent()->GetVM()};
        AddTask(std::move(ctx), func);
    }
    if (no_async_jit_) {
        auto status = method->GetCompilationStatus();
        for (; (status == Method::WAITING) || (status == Method::COMPILATION);
             status = method->GetCompilationStatus()) {
            if (compiler_worker_ == nullptr || compiler_worker_->IsWorkerJoined()) {
                // JIT thread is destroyed, wait makes no sence
                return false;
            }
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

void Compiler::CompileMethodLocked(const CompilerTask &&ctx)
{
    os::memory::LockHolder lock(compilation_lock_);
    StartCompileMethod(std::move(ctx));
}

void Compiler::StartCompileMethod(const CompilerTask &&ctx)
{
    ASSERT(runtime_iface_ != nullptr);
    auto method = ctx.GetMethod();

    method->ResetHotnessCounter();

    if (IsCompilationExpired(ctx)) {
        ASSERT(!no_async_jit_);
        return;
    }

    PandaVM *vm = ctx.GetVM();

    // Set current thread to have access to vm during compilation
    Thread compiler_thread(vm, Thread::ThreadType::THREAD_TYPE_COMPILER);
    ScopedCurrentThread sct(&compiler_thread);

    auto is_osr = ctx.IsOsr();
    mem::MemStatsType *mem_stats = vm->GetMemStats();
    panda::ArenaAllocator allocator(panda::SpaceType::SPACE_TYPE_COMPILER, mem_stats);
    panda::ArenaAllocator graph_local_allocator(panda::SpaceType::SPACE_TYPE_COMPILER, mem_stats, true);
    if (!compiler::JITCompileMethod(runtime_iface_, method, ctx.IsOsr(), code_allocator_, &allocator,
                                    &graph_local_allocator, &gdb_debug_info_allocator_, jit_stats_)) {
        // If deoptimization occurred during OSR compilation, the compilation returns false.
        // For the case we need reset compiation status
        if (is_osr) {
            method->SetCompilationStatus(Method::NOT_COMPILED);
            return;
        }
        // Failure during compilation, should we retry later?
        method->SetCompilationStatus(Method::FAILED);
        return;
    }
    // Check that method was not deoptimized
    method->AtomicSetCompilationStatus(Method::COMPILATION, Method::COMPILED);
}

void Compiler::JoinWorker()
{
    if (compiler_worker_ != nullptr) {
        compiler_worker_->JoinWorker();
    }
#ifdef PANDA_COMPILER_DEBUG_INFO
    if (!Runtime::GetOptions().IsArkAot() && compiler::OPTIONS.IsCompilerEmitDebugInfo()) {
        compiler::CleanJitDebugCode();
    }
#endif
}

ObjectPointerType PandaRuntimeInterface::GetNonMovableString(MethodPtr method, StringId id) const
{
    auto vm = Runtime::GetCurrent()->GetPandaVM();
    auto pf = MethodCast(method)->GetPandaFile();
    return ToObjPtrType(vm->GetNonMovableString(*pf, panda_file::File::EntityId {id}));
}

#ifndef PANDA_PRODUCT_BUILD
uint8_t CompileMethodImpl(coretypes::String *full_method_name, panda_file::SourceLang source_lang)
{
    auto name = ConvertToString(full_method_name);
    auto *class_linker = Runtime::GetCurrent()->GetClassLinker();

    size_t pos = name.find_last_of("::");
    if (pos == std::string_view::npos) {
        return 1;
    }
    auto class_name = PandaString(name.substr(0, pos - 1));
    auto method_name = PandaString(name.substr(pos + 1));

    PandaString descriptor;
    auto class_name_bytes = ClassHelper::GetDescriptor(utf::CStringAsMutf8(class_name.c_str()), &descriptor);
    auto method_name_bytes = utf::CStringAsMutf8(method_name.c_str());

    ClassLinkerExtension *ext = class_linker->GetExtension(source_lang);
    Class *cls = class_linker->GetClass(class_name_bytes, true, ext->GetBootContext());
    if (cls == nullptr) {
        static constexpr uint8_t CLASS_IS_NULL = 2;
        return CLASS_IS_NULL;
    }

    auto method = cls->GetDirectMethod(method_name_bytes);
    if (method == nullptr) {
        static constexpr uint8_t METHOD_IS_NULL = 3;
        return METHOD_IS_NULL;
    }

    if (method->IsAbstract()) {
        static constexpr uint8_t ABSTRACT_ERROR = 4;
        return ABSTRACT_ERROR;
    }
    if (method->HasCompiledCode()) {
        return 0;
    }
    auto *compiler = Runtime::GetCurrent()->GetPandaVM()->GetCompiler();
    auto status = method->GetCompilationStatus();
    for (; (status != Method::COMPILED) && (status != Method::FAILED); status = method->GetCompilationStatus()) {
        if (status == Method::NOT_COMPILED) {
            ASSERT(!method->HasCompiledCode());
            compiler->CompileMethod(method, 0, false, TaggedValue::Hole());
        }
        static constexpr uint64_t SLEEP_MS = 10;
        MTManagedThread::GetCurrent()->TimedWait(ThreadStatus::IS_COMPILER_WAITING, SLEEP_MS, 0);
    }
    static constexpr uint8_t COMPILATION_FAILED = 5;
    return (status == Method::COMPILED ? 0 : COMPILATION_FAILED);
}
#endif  // PANDA_PRODUCT_BUILD

}  // namespace panda
