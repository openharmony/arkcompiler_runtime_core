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

#ifndef COMPILER_RUNTIME_INTERFACE_H
#define COMPILER_RUNTIME_INTERFACE_H

#include <cstdint>
#include <cstddef>
#include "assembler/assembly-literals.h"
#include "constants.h"
#include "cross_values.h"
#include "datatype.h"
#include "ir-dyn-base-types.h"
#include "mem/gc_barrier.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "source_languages.h"
#include "utils/arch.h"
#include "utils/span.h"
#include "utils/arena_containers.h"

namespace panda::compiler {
enum class ClassType {
    UNRESOLVED_CLASS = 0,
    OBJECT_CLASS,
    ARRAY_CLASS,
    ARRAY_OBJECT_CLASS,
    INTERFACE_CLASS,
    OTHER_CLASS,
    FINAL_CLASS,
    COUNT
};

class IClassHierarchyAnalysis;
class InlineCachesInterface;
class UnresolvedTypesInterface;
class Inst;

class RuntimeInterface {
public:
    using BinaryFilePtr = void *;
    using MethodPtr = void *;
    using FieldPtr = void *;
    using MethodId = uint32_t;
    using StringPtr = void *;
    using ClassPtr = void *;
    using IdType = uint32_t;
    using FieldId = uint32_t;
    using StringId = uint32_t;
    using LiteralArrayId = uint32_t;
    using MethodIndex = uint16_t;
    using FieldIndex = uint16_t;
    using TypeIndex = uint16_t;
    using StringIndex = uint16_t;
    using LiteralArrayIndex = uint16_t;

    static const uintptr_t RESOLVE_STRING_AOT_COUNTER_LIMIT = PANDA_32BITS_HEAP_START_ADDRESS;

    RuntimeInterface() = default;
    virtual ~RuntimeInterface() = default;

    virtual IClassHierarchyAnalysis *GetCha()
    {
        return nullptr;
    }

    virtual InlineCachesInterface *GetInlineCaches()
    {
        return nullptr;
    }

    virtual UnresolvedTypesInterface *GetUnresolvedTypes()
    {
        return nullptr;
    }

    virtual void *GetRuntimeEntry()
    {
        return nullptr;
    }

    virtual unsigned GetReturnReasonOk() const
    {
        return 0;
    }
    virtual unsigned GetReturnReasonDeopt() const
    {
        return 1;
    }

    virtual MethodId ResolveMethodIndex([[maybe_unused]] MethodPtr parent_method,
                                        [[maybe_unused]] MethodIndex index) const
    {
        return 0;
    }

    virtual uint32_t ResolveOffsetByIndex([[maybe_unused]] MethodPtr parent_method,
                                          [[maybe_unused]] uint16_t index) const
    {
        return 0;
    }

    virtual FieldId ResolveFieldIndex([[maybe_unused]] MethodPtr parent_method, [[maybe_unused]] FieldIndex index) const
    {
        return 0;
    }

    virtual IdType ResolveTypeIndex([[maybe_unused]] MethodPtr parent_method, [[maybe_unused]] TypeIndex index) const
    {
        return 0;
    }

    virtual size_t GetStackOverflowCheckOffset() const
    {
        return 0;
    }

    /**************************************************************************
     * Binary file information
     */
    virtual BinaryFilePtr GetBinaryFileForMethod([[maybe_unused]] MethodPtr method) const
    {
        return nullptr;
    }

    // File offsets
    uint32_t GetBinaryFileBaseOffset(Arch arch) const
    {
        return cross_values::GetFileBaseOffset(arch);
    }

    /**************************************************************************
     * Method information
     */
    virtual MethodPtr GetMethodById([[maybe_unused]] MethodPtr parent_method, [[maybe_unused]] MethodId id) const
    {
        return nullptr;
    }

    virtual MethodId GetMethodId([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    virtual MethodPtr ResolveVirtualMethod([[maybe_unused]] ClassPtr cls, [[maybe_unused]] MethodPtr id) const
    {
        return nullptr;
    }

    virtual MethodPtr ResolveInterfaceMethod([[maybe_unused]] ClassPtr cls, [[maybe_unused]] MethodPtr id) const
    {
        return nullptr;
    }

    virtual DataType::Type GetMethodReturnType([[maybe_unused]] MethodPtr method) const
    {
        return DataType::NO_TYPE;
    }

    // Return this argument type for index == 0 in case of instance method
    virtual DataType::Type GetMethodTotalArgumentType([[maybe_unused]] MethodPtr method,
                                                      [[maybe_unused]] size_t index) const
    {
        return DataType::NO_TYPE;
    }
    // Return total arguments count including this for instance method
    virtual size_t GetMethodTotalArgumentsCount([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }
    virtual DataType::Type GetMethodReturnType([[maybe_unused]] MethodPtr parent_method,
                                               [[maybe_unused]] MethodId id) const
    {
        return DataType::NO_TYPE;
    }
    virtual DataType::Type GetMethodArgumentType([[maybe_unused]] MethodPtr parent_method, [[maybe_unused]] MethodId id,
                                                 [[maybe_unused]] size_t index) const
    {
        return DataType::NO_TYPE;
    }
    virtual size_t GetMethodArgumentsCount([[maybe_unused]] MethodPtr parent_method, [[maybe_unused]] MethodId id) const
    {
        return 0;
    }
    virtual size_t GetMethodArgumentsCount([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }
    virtual size_t GetMethodRegistersCount([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }
    virtual const uint8_t *GetMethodCode([[maybe_unused]] MethodPtr method) const
    {
        return nullptr;
    }
    virtual size_t GetMethodCodeSize([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    virtual SourceLanguage GetMethodSourceLanguage([[maybe_unused]] MethodPtr method) const
    {
        return SourceLanguage::PANDA_ASSEMBLY;
    }

    virtual void SetCompiledEntryPoint([[maybe_unused]] MethodPtr method, [[maybe_unused]] void *entry_point) {}

    virtual void SetOsrCode([[maybe_unused]] MethodPtr method, [[maybe_unused]] void *entry_point) {}

    virtual void *GetOsrCode([[maybe_unused]] MethodPtr method)
    {
        return nullptr;
    }

    virtual bool HasCompiledCode([[maybe_unused]] MethodPtr method)
    {
        return false;
    }

    virtual uint32_t GetAccessFlagAbstractMask() const
    {
        return 0;
    }

    virtual uint32_t GetVTableIndex([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    virtual bool IsMethodExternal([[maybe_unused]] MethodPtr method, [[maybe_unused]] MethodPtr callee_method) const
    {
        return false;
    }

    virtual bool IsMethodIntrinsic([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodAbstract([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodIntrinsic([[maybe_unused]] MethodPtr parent_method, [[maybe_unused]] MethodId id) const
    {
        return false;
    }

    // return true if the method is Jni with exception
    virtual bool HasNativeException([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodStatic([[maybe_unused]] MethodPtr parent_method, [[maybe_unused]] MethodId id) const
    {
        return false;
    }

    virtual bool IsMethodStatic([[maybe_unused]] MethodPtr method) const
    {
        return true;
    }

    virtual bool IsMethodFinal([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsMethodCanBeInlined([[maybe_unused]] MethodPtr method) const
    {
        return true;
    }

    virtual bool IsMethodStaticConstructor([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual std::string GetFileName([[maybe_unused]] MethodPtr method) const
    {
        return "UnknownFile";
    }

    virtual std::string GetClassNameFromMethod([[maybe_unused]] MethodPtr method) const
    {
        return "UnknownClass";
    }

    virtual std::string GetClassName([[maybe_unused]] ClassPtr klass) const
    {
        return "UnknownClass";
    }

    virtual ClassPtr GetClass([[maybe_unused]] MethodPtr method) const
    {
        return nullptr;
    }

    // returns Class for Field
    virtual ClassPtr GetClassForField([[maybe_unused]] FieldPtr field) const
    {
        return nullptr;
    }

    ClassPtr ResolveClassForField(MethodPtr method, size_t field_id);

    virtual std::string GetMethodName([[maybe_unused]] MethodPtr method) const
    {
        return "UnknownMethod";
    }

    virtual int64_t GetBranchTakenCounter([[maybe_unused]] MethodPtr method, [[maybe_unused]] uint32_t pc) const
    {
        return 0;
    }

    virtual int64_t GetBranchNotTakenCounter([[maybe_unused]] MethodPtr method, [[maybe_unused]] uint32_t pc) const
    {
        return 0;
    }

    virtual bool IsConstructor([[maybe_unused]] MethodPtr method, [[maybe_unused]] uint32_t class_id)
    {
        return false;
    }

    // returns true if need to encode memory barrier before return
    virtual bool IsMemoryBarrierRequired([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual std::string GetMethodFullName([[maybe_unused]] MethodPtr method, [[maybe_unused]] bool with_signature) const
    {
        return "UnknownMethod";
    }

    std::string GetMethodFullName(MethodPtr method) const
    {
        return GetMethodFullName(method, false);
    }

    virtual std::string GetBytecodeString([[maybe_unused]] MethodPtr method, [[maybe_unused]] uintptr_t pc) const
    {
        return std::string();
    }

    virtual panda::pandasm::LiteralArray GetLiteralArray([[maybe_unused]] MethodPtr method,
                                                         [[maybe_unused]] LiteralArrayId id) const
    {
        return panda::pandasm::LiteralArray();
    }

    virtual bool IsInterfaceMethod([[maybe_unused]] MethodPtr parent_method, [[maybe_unused]] MethodId id) const
    {
        return false;
    }

    virtual bool IsInterfaceMethod([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool IsInstanceConstructor([[maybe_unused]] MethodPtr method) const
    {
        return false;
    }

    virtual bool CanThrowException([[maybe_unused]] MethodPtr method) const
    {
        return true;
    }

    // Method offsets
    uint32_t GetAccessFlagsOffset(Arch arch) const
    {
        return panda::cross_values::GetMethodAccessFlagsOffset(arch);
    }
    uint32_t GetVTableIndexOffset(Arch arch) const
    {
        return panda::cross_values::GetMethodVTableIndexOffset(arch);
    }
    uint32_t GetClassOffset(Arch arch) const
    {
        return panda::cross_values::GetMethodClassOffset(arch);
    }
    uint32_t GetCompiledEntryPointOffset(Arch arch) const
    {
        return panda::cross_values::GetMethodCompiledEntryPointOffset(arch);
    }
    uint32_t GetPandaFileOffset(Arch arch) const
    {
        return panda::cross_values::GetMethodPandaFileOffset(arch);
    }

    /**************************************************************************
     * Exec state information
     */
    size_t GetTlsFrameKindOffset(Arch arch) const
    {
        return panda::cross_values::GetManagedThreadFrameKindOffset(arch);
    }
    uint32_t GetFlagAddrOffset(Arch arch) const
    {
        return panda::cross_values::GetManagedThreadFlagOffset(arch);
    }
    size_t GetTlsFrameOffset(Arch arch) const
    {
        return panda::cross_values::GetManagedThreadFrameOffset(arch);
    }
    size_t GetExceptionOffset(Arch arch) const
    {
        return panda::cross_values::GetManagedThreadExceptionOffset(arch);
    }
    size_t GetTlsNativePcOffset(Arch arch) const
    {
        return panda::cross_values::GetManagedThreadNativePcOffset(arch);
    }
    size_t GetTlsCardTableAddrOffset(Arch arch) const
    {
        return panda::cross_values::GetManagedThreadCardTableAddrOffset(arch);
    }
    size_t GetTlsCardTableMinAddrOffset(Arch arch) const
    {
        return panda::cross_values::GetManagedThreadCardTableMinAddrOffset(arch);
    }
    size_t GetTlsConcurrentMarkingAddrOffset(Arch arch) const
    {
        return panda::cross_values::GetManagedThreadConcurrentMarkingAddrOffset(arch);
    }
    uint32_t GetLanguageExtensionsDataOffset([[maybe_unused]] Arch arch) const
    {
        return panda::cross_values::GetManagedThreadLanguageExtensionDataOffset(arch);
    }

    virtual ::panda::mem::BarrierType GetPreType() const
    {
        return ::panda::mem::BarrierType::PRE_WRB_NONE;
    }

    virtual ::panda::mem::BarrierType GetPostType() const
    {
        return ::panda::mem::BarrierType::POST_WRB_NONE;
    }

    virtual ::panda::mem::BarrierOperand GetBarrierOperand(
        [[maybe_unused]] ::panda::mem::BarrierPosition barrier_position,
        [[maybe_unused]] std::string_view operand_name) const
    {
        return ::panda::mem::BarrierOperand(::panda::mem::BarrierOperandType::BOOL_ADDRESS, false);
    }

    /**************************************************************************
     * Array information
     */
    uint32_t GetClassArraySize(Arch arch) const
    {
        return panda::cross_values::GetCoretypesArrayClassSize(arch);
    }

    virtual uint32_t GetArrayElementSize([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType id) const
    {
        return ARRAY_DEFAULT_ELEMENT_SIZE;
    }

    virtual uintptr_t GetPointerToConstArrayData([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType id) const
    {
        return 0;
    }

    virtual size_t GetOffsetToConstArrayData([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType id) const
    {
        return 0;
    }

    // Array offsets
    uint32_t GetArrayDataOffset(Arch arch) const
    {
        return panda::cross_values::GetCoretypesArrayDataOffset(arch);
    }
    uint32_t GetArrayLengthOffset(Arch arch) const
    {
        return panda::cross_values::GetCoretypesArrayLengthOffset(arch);
    }

    /**************************************************************************
     * String information
     */
    virtual bool IsCompressedStringsEnabled() const
    {
        return true;
    }

    virtual uint32_t GetStringCompressionMask() const
    {
        return 1;
    }

    virtual object_pointer_type GetNonMovableString([[maybe_unused]] MethodPtr method,
                                                    [[maybe_unused]] StringId id) const
    {
        return 0;
    }

    virtual ClassPtr GetStringClass([[maybe_unused]] MethodPtr method) const
    {
        return nullptr;
    }

    // String offsets
    uint32_t GetStringDataOffset(Arch arch) const
    {
        return panda::cross_values::GetCoretypesStringDataOffset(arch);
    }
    uint32_t GetStringLengthOffset(Arch arch) const
    {
        return panda::cross_values::GetCoretypesStringLengthOffset(arch);
    }
    uintptr_t GetStringClassPointerTlsOffset(Arch arch) const
    {
        return cross_values::GetManagedThreadStringClassPtrOffset(arch);
    }

    /**************************************************************************
     * managed Thread object information
     */

    uint32_t GetThreadObjectOffset(Arch arch) const
    {
        return panda::cross_values::GetManagedThreadObjectOffset(arch);
    }

    /**************************************************************************
     * TLAB information
     */

    virtual size_t GetTLABMaxSize() const
    {
        return 0;
    }

    virtual size_t GetTLABAlignment() const
    {
        return 1;
    }

    virtual bool IsTrackTlabAlloc() const
    {
        return false;
    }

    // TLAB offsets
    size_t GetCurrentTLABOffset(Arch arch) const
    {
        return panda::cross_values::GetManagedThreadTlabOffset(arch);
    }
    size_t GetTLABStartPointerOffset(Arch arch) const
    {
        return panda::cross_values::GetTlabMemoryStartAddrOffset(arch);
    }
    size_t GetTLABFreePointerOffset(Arch arch) const
    {
        return panda::cross_values::GetTlabCurFreePositionOffset(arch);
    }
    size_t GetTLABEndPointerOffset(Arch arch) const
    {
        return panda::cross_values::GetTlabMemoryEndAddrOffset(arch);
    }

    /**************************************************************************
     * Object information
     */
    virtual ClassPtr GetClass([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType id) const
    {
        return nullptr;
    }

    virtual ClassType GetClassType([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType id) const
    {
        return ClassType::UNRESOLVED_CLASS;
    }

    virtual bool IsArrayClass([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType id) const
    {
        return false;
    }

    virtual bool IsArrayClass([[maybe_unused]] ClassPtr klass) const
    {
        return false;
    }

    virtual ClassPtr GetArrayElementClass([[maybe_unused]] ClassPtr cls) const
    {
        return nullptr;
    }

    virtual bool CheckStoreArray([[maybe_unused]] ClassPtr array_cls, [[maybe_unused]] ClassPtr str_cls) const
    {
        return false;
    }

    virtual bool IsAssignableFrom([[maybe_unused]] ClassPtr cls1, [[maybe_unused]] ClassPtr cls2) const
    {
        return false;
    }

    virtual size_t GetObjectHashedStatusBitNum() const
    {
        return 0;
    }

    virtual size_t GetObjectHashShift() const
    {
        return 0;
    }

    virtual size_t GetObjectHashMask() const
    {
        return 0;
    }

    // Offset of class in ObjectHeader
    uint32_t GetObjClassOffset(Arch arch) const
    {
        return panda::cross_values::GetObjectHeaderClassPointerOffset(arch);
    }

    // Offset of the managed object in the BaseClass
    uint32_t GetManagedClassOffset(Arch arch) const
    {
        return panda::cross_values::GetBaseClassManagedObjectOffset(arch);
    }

    // Offset of mark word in ObjectHeader
    uint32_t GetObjMarkWordOffset(Arch arch) const
    {
        return panda::cross_values::GetObjectHeaderMarkWordOffset(arch);
    }

    /**************************************************************************
     * Class information
     */

    // Returns Class Id for Field.
    // We don't get class id directly from the field's class, because we need a class id regarding to the current
    // file. Class id is used in codegen to initialize class in aot mode.
    virtual size_t GetClassIdForField([[maybe_unused]] MethodPtr method, [[maybe_unused]] size_t unused) const
    {
        return 0;
    }

    virtual size_t GetClassIdForField([[maybe_unused]] FieldPtr field) const
    {
        return 0;
    }

    // returns Class Id for Method
    virtual size_t GetClassIdForMethod([[maybe_unused]] MethodPtr method) const
    {
        return 0;
    }

    // returns Class Id for Method
    virtual size_t GetClassIdForMethod([[maybe_unused]] MethodPtr method, [[maybe_unused]] size_t unused) const
    {
        return 0;
    }

    virtual IdType GetClassIdWithinFile([[maybe_unused]] MethodPtr method, [[maybe_unused]] ClassPtr klass) const
    {
        return 0;
    }

    virtual IdType GetLiteralArrayClassIdWithinFile([[maybe_unused]] MethodPtr method,
                                                    [[maybe_unused]] panda_file::LiteralTag tag) const
    {
        return 0;
    }

    virtual bool CanUseTlabForClass([[maybe_unused]] ClassPtr klass) const
    {
        return true;
    }

    // returns class size
    virtual size_t GetClassSize([[maybe_unused]] ClassPtr klass) const
    {
        return 0;
    }

    // Vtable offset in Class
    uint32_t GetVTableOffset(Arch arch) const
    {
        return panda::cross_values::GetClassVtableOffset(arch);
    }

    // returns base offset in Class(for array)
    uint32_t GetClassBaseOffset(Arch arch) const
    {
        return panda::cross_values::GetClassBaseOffset(arch);
    }

    // returns component type offset in Class(for array)
    uint32_t GetClassComponentTypeOffset(Arch arch) const
    {
        return panda::cross_values::GetClassComponentTypeOffset(arch);
    }

    // returns type offset in Class(for array)
    uint32_t GetClassTypeOffset(Arch arch) const
    {
        return panda::cross_values::GetClassTypeOffset(arch);
    }

    uint32_t GetClassStateOffset(Arch arch) const
    {
        return panda::cross_values::GetClassStateOffset(arch);
    }

    uint32_t GetClassMethodsOffset(Arch arch) const
    {
        return panda::cross_values::GetClassMethodsOffset(arch);
    }

    /**************************************************************************
     * Field information
     */

    /**
     * Try to resolve field.
     * @param method method to which the field belongs
     * @param id id of the field
     * @param allow_external allow fields defined in the external file, if false - return nullptr for external fields
     * @param class_id output variable where will be written a field's class
     * @return return field or nullptr if it cannot be resolved
     */
    virtual FieldPtr ResolveField([[maybe_unused]] MethodPtr method, [[maybe_unused]] size_t unused,
                                  [[maybe_unused]] bool allow_external, [[maybe_unused]] uint32_t *class_id)
    {
        return nullptr;
    }

    virtual DataType::Type GetFieldType([[maybe_unused]] FieldPtr field) const
    {
        return DataType::NO_TYPE;
    }

    virtual DataType::Type GetFieldTypeById([[maybe_unused]] MethodPtr method, [[maybe_unused]] IdType unused) const
    {
        return DataType::NO_TYPE;
    }

    virtual size_t GetFieldOffset([[maybe_unused]] FieldPtr field) const
    {
        return 0;
    }

    virtual FieldPtr GetFieldByOffset([[maybe_unused]] size_t offset) const
    {
        return nullptr;
    }

    virtual uintptr_t GetFieldClass([[maybe_unused]] FieldPtr field) const
    {
        return 0;
    }

    virtual bool IsFieldVolatile([[maybe_unused]] FieldPtr field) const
    {
        return false;
    }

    virtual bool HasFieldMetadata([[maybe_unused]] FieldPtr field) const
    {
        return false;
    }

    virtual std::string GetFieldName([[maybe_unused]] FieldPtr field) const
    {
        return "UnknownField";
    }

    // Return offset of the managed object in the class
    uint32_t GetFieldClassOffset(Arch arch) const
    {
        return panda::cross_values::GetFieldClassOffset(arch);
    }

    // Return offset of the managed object in the class
    uint32_t GetFieldOffsetOffset(Arch arch) const
    {
        return panda::cross_values::GetFieldOffsetOffset(arch);
    }

    /**************************************************************************
     * Type information
     */
    virtual ClassPtr ResolveType([[maybe_unused]] MethodPtr method, [[maybe_unused]] size_t unused) const
    {
        return nullptr;
    }

    virtual bool IsClassInitialized([[maybe_unused]] uintptr_t unused) const
    {
        return true;
    }

    virtual bool IsClassFinal([[maybe_unused]] ClassPtr unused) const
    {
        return false;
    }

    virtual uintptr_t GetManagedType([[maybe_unused]] uintptr_t unused) const
    {
        return 0;
    }

    virtual uint8_t GetClassInitializedValue() const
    {
        return 0;
    }

    virtual uint8_t GetReferenceTypeMask() const
    {
        return 0;
    }

    /**************************************************************************
     * Entrypoints
     */
#include <intrinsics_enum.inl>
#include <entrypoints_compiler.inl>
#include <entrypoints_compiler_checksum.inl>
#include "compiler_interface_extensions.inl.h"

    virtual IntrinsicId GetIntrinsicId([[maybe_unused]] MethodPtr method) const
    {
        return static_cast<IntrinsicId>(0);
    }

    virtual uintptr_t GetIntrinsicAddress([[maybe_unused]] bool runtime_call, [[maybe_unused]] IntrinsicId unused) const
    {
        return 0;
    }

    uintptr_t GetEntrypointTlsOffset(Arch arch, EntrypointId id) const
    {
        return cross_values::GetManagedThreadEntrypointOffset(arch, panda::EntrypointId(static_cast<uint8_t>(id)));
    }

    /**************************************************************************
     * Dynamic object information
     */

    virtual uint32_t GetFunctionTargetOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }

    virtual uint64_t GetDynamicPrimitiveUndefined() const
    {
        return static_cast<uint64_t>(coretypes::TaggedValue::Undefined().GetRawData());
    }

    virtual uint64_t GetPackConstantByPrimitiveType(compiler::AnyBaseType type, uint64_t imm) const
    {
        auto datatype = AnyBaseTypeToDataType(type);
        if (datatype == DataType::INT32) {
            return coretypes::TaggedValue::GetIntTaggedValue(imm);
        }
        if (datatype == DataType::FLOAT64) {
            return coretypes::TaggedValue::GetDoubleTaggedValue(imm);
        }
        if (datatype == DataType::BOOL) {
            return coretypes::TaggedValue::GetBoolTaggedValue(imm);
        }
        UNREACHABLE();
        return 0;
    }

    virtual uint64_t GetDynamicPrimitiveFalse() const
    {
        return static_cast<uint64_t>(coretypes::TaggedValue::False().GetRawData());
    }

    virtual uint64_t GetDynamicPrimitiveTrue() const
    {
        return static_cast<uint64_t>(coretypes::TaggedValue::True().GetRawData());
    }

    virtual uint32_t GetNativePointerTargetOffset([[maybe_unused]] Arch arch) const
    {
        return 0;
    }

    /**************************************************************************
     * Check if GC can be triggered during call.
     * This is possible when method A calling method B and waiting while B is compiling.
     */
    virtual bool HasSafepointDuringCall() const
    {
        return false;
    }

    // TypeInfoIndex adaption
    virtual TypeInfoIndex GetTypeInfoIndexByInstId([[maybe_unused]] size_t id) const
    {
        return NO_EXPLICIT_TYPE;
    }

    virtual bool IsPcBindType([[maybe_unused]] int32_t pc) const
    {
        return false;
    }

    virtual bool FillInstIdTypePairByPc([[maybe_unused]] size_t id, [[maybe_unused]] int32_t pc)
    {
        return false;
    }

    virtual bool HasInsTypeinfo() const
    {
        return false;
    }

    virtual bool AddPcTypePair([[maybe_unused]] int32_t pc, [[maybe_unused]] TypeInfoIndex type)
    {
        return false;
    }

    virtual bool FillArgTypePairs([[maybe_unused]] std::unordered_map<int32_t, TypeInfoIndex> *map) const
    {
        return false;
    }

    virtual bool SetTypeLiteralArrayKey([[maybe_unused]] std::string key)
    {
        return false;
    }

    virtual const std::string *GetTypeLiteralArrayKey() const
    {
        return nullptr;
    }

    NO_COPY_SEMANTIC(RuntimeInterface);
    NO_MOVE_SEMANTIC(RuntimeInterface);

private:
    static constexpr uint32_t ARRAY_DEFAULT_ELEMENT_SIZE = 4;
};

class IClassHierarchyAnalysis {
public:
    IClassHierarchyAnalysis() = default;
    virtual ~IClassHierarchyAnalysis() = default;

public:
    virtual RuntimeInterface::MethodPtr GetSingleImplementation([
        [maybe_unused]] RuntimeInterface::MethodPtr method) = 0;
    virtual bool IsSingleImplementation([[maybe_unused]] RuntimeInterface::MethodPtr method) = 0;
    virtual void AddDependency([[maybe_unused]] RuntimeInterface::MethodPtr caller,
                               [[maybe_unused]] RuntimeInterface::MethodPtr callee) = 0;

    NO_COPY_SEMANTIC(IClassHierarchyAnalysis);
    NO_MOVE_SEMANTIC(IClassHierarchyAnalysis);
};

class InlineCachesInterface {
public:
    using ClassList = Span<RuntimeInterface::ClassPtr>;
    enum class CallKind { UNKNOWN, MONOMORPHIC, POLYMORPHIC, MEGAMORPHIC };

    virtual CallKind GetClasses(RuntimeInterface::MethodPtr method, uintptr_t unused,
                                ArenaVector<RuntimeInterface::ClassPtr> *classes) = 0;
    virtual ~InlineCachesInterface() = default;
    InlineCachesInterface() = default;

    DEFAULT_COPY_SEMANTIC(InlineCachesInterface);
    DEFAULT_MOVE_SEMANTIC(InlineCachesInterface);
};

class UnresolvedTypesInterface {
public:
    enum class SlotKind { UNKNOWN, CLASS, MANAGED_CLASS, METHOD, VIRTUAL_METHOD, FIELD, STATIC_FIELD_PTR };
    virtual bool AddTableSlot([[maybe_unused]] RuntimeInterface::MethodPtr method, [[maybe_unused]] uint32_t type_id,
                              [[maybe_unused]] SlotKind kind) = 0;
    virtual uintptr_t GetTableSlot([[maybe_unused]] RuntimeInterface::MethodPtr method,
                                   [[maybe_unused]] uint32_t type_id, [[maybe_unused]] SlotKind kind) const = 0;
    virtual ~UnresolvedTypesInterface() = default;
    UnresolvedTypesInterface() = default;

    DEFAULT_COPY_SEMANTIC(UnresolvedTypesInterface);
    DEFAULT_MOVE_SEMANTIC(UnresolvedTypesInterface);
};

enum class TraceId {
    METHOD_ENTER = 1U << 0U,
    METHOD_EXIT = 1U << 1U,
    PRINT_ARG = 1U << 2U,
    TLAB_EVENT = 1U << 3U,
};

enum class DeoptimizeType {
    INVALID = 0,
    INLINE_IC,
    INLINE_CHA,
    NULL_CHECK,
    BOUNDS_CHECK,
    ZERO_CHECK,
    NEGATIVE_CHECK,
    CHECK_CAST,
    ANY_TYPE_CHECK,
    DEOPT_OVERFLOW,
    COUNT
};

inline const char *DeoptimizeTypeToString(DeoptimizeType deopt_type)
{
    static constexpr auto COUNT = static_cast<uint8_t>(DeoptimizeType::COUNT);
    static constexpr std::array<const char *, COUNT> DEOPT_TYPE_NAMES = {
        "INVALID_TYPE", "INLINE_IC",      "INLINE_CHA", "NULL_CHECK",    "BOUNDS_CHECK",
        "ZERO_CHECK",   "NEGATIVE_CHECK", "CHECK_CAST", "ANY_TYPE_CHECK"};
    auto idx = static_cast<uint8_t>(deopt_type);
    ASSERT(idx < COUNT);
    return DEOPT_TYPE_NAMES[idx];
}
}  // namespace panda::compiler

#endif  // COMPILER_RUNTIME_INTERFACE_H
