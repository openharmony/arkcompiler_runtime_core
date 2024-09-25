/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "assembly-emitter.h"

#include "file_items.h"
#include "file_writer.h"
#include "mangling.h"
#include "os/file.h"
#include "runtime/include/profiling_gen.h"
#include "libpandafile/type_helper.h"

#include <algorithm>
#include <iostream>

namespace {

using ark::panda_file::AnnotationItem;
using ark::panda_file::ArrayValueItem;
using ark::panda_file::BaseClassItem;
using ark::panda_file::BaseFieldItem;
using ark::panda_file::BaseMethodItem;
using ark::panda_file::ClassItem;
using ark::panda_file::CodeItem;
using ark::panda_file::DebugInfoItem;
using ark::panda_file::FieldItem;
using ark::panda_file::FileWriter;
using ark::panda_file::ForeignClassItem;
using ark::panda_file::ForeignFieldItem;
using ark::panda_file::ForeignMethodItem;
using ark::panda_file::ItemContainer;
using ark::panda_file::MemoryBufferWriter;
using ark::panda_file::MethodHandleItem;
using ark::panda_file::MethodItem;
using ark::panda_file::MethodParamItem;
using ark::panda_file::ParamAnnotationsItem;
using ark::panda_file::PrimitiveTypeItem;
using ark::panda_file::ScalarValueItem;
using ark::panda_file::StringItem;
using ark::panda_file::Type;
using ark::panda_file::TypeItem;
using ark::panda_file::ValueItem;
using ark::panda_file::Writer;

std::unordered_map<Type::TypeId, PrimitiveTypeItem *> CreatePrimitiveTypes(ItemContainer *container)
{
    auto res = std::unordered_map<Type::TypeId, PrimitiveTypeItem *> {};
    res.insert({Type::TypeId::VOID, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::VOID)});
    res.insert({Type::TypeId::U1, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::U1)});
    res.insert({Type::TypeId::I8, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::I8)});
    res.insert({Type::TypeId::U8, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::U8)});
    res.insert({Type::TypeId::I16, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::I16)});
    res.insert({Type::TypeId::U16, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::U16)});
    res.insert({Type::TypeId::I32, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::I32)});
    res.insert({Type::TypeId::U32, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::U32)});
    res.insert({Type::TypeId::I64, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::I64)});
    res.insert({Type::TypeId::U64, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::U64)});
    res.insert({Type::TypeId::F32, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::F32)});
    res.insert({Type::TypeId::F64, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::F64)});
    res.insert({Type::TypeId::TAGGED, container->GetOrCreatePrimitiveTypeItem(Type::TypeId::TAGGED)});
    return res;
}

template <class T>
typename T::mapped_type Find(const T &map, typename T::key_type key)
{
    auto res = map.find(key);
    ASSERT(res != map.end());
    return res->second;
}

}  // anonymous namespace

namespace ark::pandasm {

/* static */
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::string AsmEmitter::lastError_ {};

static panda_file::Type::TypeId GetTypeId(Value::Type type)
{
    switch (type) {
        case Value::Type::U1:
            return panda_file::Type::TypeId::U1;
        case Value::Type::I8:
            return panda_file::Type::TypeId::I8;
        case Value::Type::U8:
            return panda_file::Type::TypeId::U8;
        case Value::Type::I16:
            return panda_file::Type::TypeId::I16;
        case Value::Type::U16:
            return panda_file::Type::TypeId::U16;
        case Value::Type::I32:
            return panda_file::Type::TypeId::I32;
        case Value::Type::U32:
            return panda_file::Type::TypeId::U32;
        case Value::Type::I64:
            return panda_file::Type::TypeId::I64;
        case Value::Type::U64:
            return panda_file::Type::TypeId::U64;
        case Value::Type::F32:
            return panda_file::Type::TypeId::F32;
        case Value::Type::F64:
            return panda_file::Type::TypeId::F64;
        case Value::Type::VOID:
            return panda_file::Type::TypeId::VOID;
        default:
            return panda_file::Type::TypeId::REFERENCE;
    }
}

/* static */
bool AsmEmitter::CheckValueType(Value::Type valueType, const Type &type, const Program &program)
{
    auto valueTypeId = GetTypeId(valueType);
    if (valueTypeId != type.GetId()) {
        SetLastError("Inconsistent element (" + AnnotationElement::TypeToString(valueType) +
                     ") and function's return type (" + type.GetName() + ")");
        return false;
    }

    switch (valueType) {
        case Value::Type::STRING:
        case Value::Type::RECORD:
        case Value::Type::ANNOTATION:
        case Value::Type::ENUM: {
            auto it = program.recordTable.find(type.GetName());
            if (it == program.recordTable.cend()) {
                SetLastError("Record " + type.GetName() + " not found");
                return false;
            }

            auto &record = it->second;
            if (valueType == Value::Type::ANNOTATION && !record.metadata->IsAnnotation() &&
                !record.metadata->IsRuntimeAnnotation() && !record.metadata->IsRuntimeTypeAnnotation() &&
                !record.metadata->IsTypeAnnotation()) {
                SetLastError("Record " + type.GetName() + " isn't annotation");
                return false;
            }

            if (valueType == Value::Type::ENUM && (record.metadata->GetAccessFlags() & ACC_ENUM) == 0) {
                SetLastError("Record " + type.GetName() + " isn't enum");
                return false;
            }

            break;
        }
        case Value::Type::ARRAY: {
            if (!type.IsArray()) {
                SetLastError("Inconsistent element (" + AnnotationElement::TypeToString(valueType) +
                             ") and function's return type (" + type.GetName() + ")");
                return false;
            }

            break;
        }
        default: {
            break;
        }
    }

    return true;
}

/* static */
std::string AsmEmitter::GetMethodSignatureFromProgram(const std::string &name, const Program &program)
{
    if (IsSignatureOrMangled(name)) {
        return name;
    }

    const auto itSynonym = program.functionSynonyms.find(name);
    const bool isMethodKnown = (itSynonym != program.functionSynonyms.end());
    const bool isSingleSynonym = (isMethodKnown && (itSynonym->second.size() == 1));
    if (isSingleSynonym) {
        return itSynonym->second[0];
    }
    SetLastError("More than one alternative for method " + name);
    return std::string("");
}

/* static */
// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP) big switch case
panda_file::LiteralItem *AsmEmitter::CreateLiteralItem(
    ItemContainer *container, const Value *value, std::vector<panda_file::LiteralItem> *out,
    const std::unordered_map<std::string, panda_file::BaseMethodItem *> &methods)
{
    ASSERT(out != nullptr);

    auto valueType = value->GetType();

    switch (valueType) {
        case Value::Type::U1:
        case Value::Type::I8:
        case Value::Type::U8: {
            auto v = value->GetAsScalar()->GetValue<uint8_t>();
            out->emplace_back(v);
            return &out->back();
        }
        case Value::Type::I16:
        case Value::Type::U16: {
            auto v = value->GetAsScalar()->GetValue<uint16_t>();
            out->emplace_back(v);
            return &out->back();
        }
        case Value::Type::I32:
        case Value::Type::U32:
        case Value::Type::STRING_NULLPTR: {
            auto v = value->GetAsScalar()->GetValue<uint32_t>();
            out->emplace_back(v);
            return &out->back();
        }
        case Value::Type::I64:
        case Value::Type::U64: {
            auto v = value->GetAsScalar()->GetValue<uint64_t>();
            out->emplace_back(v);
            return &out->back();
        }
        case Value::Type::F32: {
            auto v = bit_cast<uint32_t>(value->GetAsScalar()->GetValue<float>());
            out->emplace_back(v);
            return &out->back();
        }
        case Value::Type::F64: {
            auto v = bit_cast<uint64_t>(value->GetAsScalar()->GetValue<double>());
            out->emplace_back(v);
            return &out->back();
        }
        case Value::Type::STRING: {
            auto *stringItem = container->GetOrCreateStringItem(value->GetAsScalar()->GetValue<std::string>());
            out->emplace_back(stringItem);
            return &out->back();
        }
        case Value::Type::METHOD: {
            auto name = value->GetAsScalar()->GetValue<std::string>();
            auto methodItem = static_cast<ark::panda_file::MethodItem *>(Find(methods, name));
            out->emplace_back(methodItem);
            return &out->back();
        }
        default:
            return nullptr;
    }
}

/* static */
bool AsmEmitter::CheckValueRecordCase(const Value *value, const Program &program)
{
    auto t = value->GetAsScalar()->GetValue<Type>();
    if (!t.IsObject()) {
        return true;
    }

    auto recordName = t.GetName();
    bool isFound;
    if (t.IsArray()) {
        auto it = program.arrayTypes.find(t);
        isFound = it != program.arrayTypes.cend();
    } else {
        auto it = program.recordTable.find(recordName);
        isFound = it != program.recordTable.cend();
    }

    if (!isFound) {
        SetLastError("Incorrect value: record " + recordName + " not found");
        return false;
    }

    return true;
}

/* static */
bool AsmEmitter::CheckValueMethodCase(const Value *value, const Program &program)
{
    auto functionName = value->GetAsScalar()->GetValue<std::string>();
    auto it = program.functionTable.find(functionName);
    if (it == program.functionTable.cend()) {
        SetLastError("Incorrect value: function " + functionName + " not found");
        return false;
    }

    return true;
}

/* static */
bool AsmEmitter::CheckValueEnumCase(const Value *value, const Type &type, const Program &program)
{
    auto enumValue = value->GetAsScalar()->GetValue<std::string>();
    auto recordName = GetOwnerName(enumValue);
    auto fieldName = GetItemName(enumValue);

    if (recordName != type.GetName()) {
        SetLastError("Incorrect value: Expected " + type.GetName() + " enum record");
        return false;
    }

    const auto &record = program.recordTable.find(recordName)->second;
    auto it = std::find_if(record.fieldList.cbegin(), record.fieldList.cend(),
                           [&fieldName](const Field &field) { return field.name == fieldName; });
    if (it == record.fieldList.cend()) {
        SetLastError("Incorrect value: Enum field " + enumValue + " not found");
        return false;
    }

    const auto &field = *it;
    if ((field.metadata->GetAccessFlags() & ACC_ENUM) == 0) {
        SetLastError("Incorrect value: Field " + enumValue + " isn't enum");
        return false;
    }

    return true;
}

/* static */
bool AsmEmitter::CheckValueArrayCase(const Value *value, const Type &type, const Program &program)
{
    auto componentType = type.GetComponentType();
    auto valueComponentType = value->GetAsArray()->GetComponentType();
    if (valueComponentType == Value::Type::VOID && value->GetAsArray()->GetValues().empty()) {
        return true;
    }

    if (!CheckValueType(valueComponentType, componentType, program)) {
        SetLastError("Incorrect array's component type: " + GetLastError());
        return false;
    }

    for (auto &elemValue : value->GetAsArray()->GetValues()) {
        if (!CheckValue(&elemValue, componentType, program)) {
            SetLastError("Incorrect array's element: " + GetLastError());
            return false;
        }
    }

    return true;
}

/* static */
bool AsmEmitter::CheckValue(const Value *value, const Type &type, const Program &program)
{
    auto valueType = value->GetType();
    if (!CheckValueType(valueType, type, program)) {
        SetLastError("Incorrect type: " + GetLastError());
        return false;
    }

    switch (valueType) {
        case Value::Type::RECORD: {
            if (!CheckValueRecordCase(value, program)) {
                return false;
            }

            break;
        }
        case Value::Type::METHOD: {
            if (!CheckValueMethodCase(value, program)) {
                return false;
            }

            break;
        }
        case Value::Type::ENUM: {
            if (!CheckValueEnumCase(value, type, program)) {
                return false;
            }

            break;
        }
        case Value::Type::ARRAY: {
            if (!CheckValueArrayCase(value, type, program)) {
                return false;
            }

            break;
        }
        default: {
            break;
        }
    }

    return true;
}

/* static */
ScalarValueItem *AsmEmitter::CreateScalarStringValueItem(ItemContainer *container, const Value *value,
                                                         std::vector<ScalarValueItem> *out)
{
    auto *stringItem = container->GetOrCreateStringItem(value->GetAsScalar()->GetValue<std::string>());
    if (out != nullptr) {
        out->emplace_back(stringItem);
        return &out->back();
    }

    return container->CreateItem<ScalarValueItem>(stringItem);
}

/* static */
ScalarValueItem *AsmEmitter::CreateScalarRecordValueItem(
    ItemContainer *container, const Value *value, std::vector<ScalarValueItem> *out,
    const std::unordered_map<std::string, BaseClassItem *> &classes)
{
    auto type = value->GetAsScalar()->GetValue<Type>();
    BaseClassItem *classItem;
    if (type.IsObject()) {
        auto name = type.GetName();
        auto it = classes.find(name);
        if (it == classes.cend()) {
            return nullptr;
        }

        classItem = it->second;
    } else {
        classItem = container->GetOrCreateForeignClassItem(type.GetDescriptor());
    }

    if (out != nullptr) {
        out->emplace_back(classItem);
        return &out->back();
    }

    return container->CreateItem<ScalarValueItem>(classItem);
}

/* static */
ScalarValueItem *AsmEmitter::CreateScalarMethodValueItem(
    ItemContainer *container, const Value *value, std::vector<ScalarValueItem> *out, const Program &program,
    const std::unordered_map<std::string, BaseMethodItem *> &methods)
{
    auto name = value->GetAsScalar()->GetValue<std::string>();

    name = GetMethodSignatureFromProgram(name, program);

    auto it = methods.find(name);
    if (it == methods.cend()) {
        return nullptr;
    }

    auto *methodItem = it->second;
    if (out != nullptr) {
        out->emplace_back(methodItem);
        return &out->back();
    }

    return container->CreateItem<ScalarValueItem>(methodItem);
}

/* static */
ScalarValueItem *AsmEmitter::CreateScalarEnumValueItem(ItemContainer *container, const Value *value,
                                                       std::vector<ScalarValueItem> *out,
                                                       const std::unordered_map<std::string, BaseFieldItem *> &fields)
{
    auto name = value->GetAsScalar()->GetValue<std::string>();
    auto it = fields.find(name);
    if (it == fields.cend()) {
        return nullptr;
    }

    auto *fieldItem = it->second;
    if (out != nullptr) {
        out->emplace_back(fieldItem);
        return &out->back();
    }

    return container->CreateItem<ScalarValueItem>(fieldItem);
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
ScalarValueItem *AsmEmitter::CreateScalarAnnotationValueItem(
    ItemContainer *container, const Value *value, std::vector<ScalarValueItem> *out, const Program &program,
    const std::unordered_map<std::string, BaseClassItem *> &classes,
    const std::unordered_map<std::string, BaseFieldItem *> &fields,
    const std::unordered_map<std::string, BaseMethodItem *> &methods)
{
    auto annotation = value->GetAsScalar()->GetValue<AnnotationData>();
    auto *annotationItem = CreateAnnotationItem(container, annotation, program, classes, fields, methods);
    if (annotationItem == nullptr) {
        return nullptr;
    }

    if (out != nullptr) {
        out->emplace_back(annotationItem);
        return &out->back();
    }

    return container->CreateItem<ScalarValueItem>(annotationItem);
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
ScalarValueItem *AsmEmitter::CreateScalarValueItem(ItemContainer *container, const Value *value,
                                                   std::vector<ScalarValueItem> *out, const Program &program,
                                                   const std::unordered_map<std::string, BaseClassItem *> &classes,
                                                   const std::unordered_map<std::string, BaseFieldItem *> &fields,
                                                   const std::unordered_map<std::string, BaseMethodItem *> &methods)
{
    auto valueType = value->GetType();

    switch (valueType) {
        case Value::Type::U1:
        case Value::Type::I8:
        case Value::Type::U8:
        case Value::Type::I16:
        case Value::Type::U16:
        case Value::Type::I32:
        case Value::Type::U32:
        case Value::Type::STRING_NULLPTR: {
            return CreateScalarPrimValueItem<uint32_t>(container, value, out);
        }
        case Value::Type::I64:
        case Value::Type::U64: {
            return CreateScalarPrimValueItem<uint64_t>(container, value, out);
        }
        case Value::Type::F32: {
            return CreateScalarPrimValueItem<float>(container, value, out);
        }
        case Value::Type::F64: {
            return CreateScalarPrimValueItem<double>(container, value, out);
        }
        case Value::Type::STRING: {
            return CreateScalarStringValueItem(container, value, out);
        }
        case Value::Type::RECORD: {
            return CreateScalarRecordValueItem(container, value, out, classes);
        }
        case Value::Type::METHOD: {
            return CreateScalarMethodValueItem(container, value, out, program, methods);
        }
        case Value::Type::ENUM: {
            return CreateScalarEnumValueItem(container, value, out, fields);
        }
        case Value::Type::ANNOTATION: {
            return CreateScalarAnnotationValueItem(container, value, out, program, classes, fields, methods);
        }
        default: {
            UNREACHABLE();
            return nullptr;
        }
    }
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
ValueItem *AsmEmitter::CreateValueItem(ItemContainer *container, const Value *value, const Program &program,
                                       const std::unordered_map<std::string, BaseClassItem *> &classes,
                                       const std::unordered_map<std::string, BaseFieldItem *> &fields,
                                       const std::unordered_map<std::string, BaseMethodItem *> &methods)
{
    switch (value->GetType()) {
        case Value::Type::ARRAY: {
            std::vector<ScalarValueItem> elements;
            for (const auto &elemValue : value->GetAsArray()->GetValues()) {
                auto *item = CreateScalarValueItem(container, &elemValue, &elements, program, classes, fields, methods);
                if (item == nullptr) {
                    return nullptr;
                }
            }

            auto componentType = value->GetAsArray()->GetComponentType();
            return container->CreateItem<ArrayValueItem>(panda_file::Type(GetTypeId(componentType)),
                                                         std::move(elements));
        }
        default: {
            return CreateScalarValueItem(container, value, nullptr, program, classes, fields, methods);
        }
    }
}

/* static */
AnnotationItem *AsmEmitter::CreateAnnotationItem(ItemContainer *container, const AnnotationData &annotation,
                                                 const Program &program,
                                                 const std::unordered_map<std::string, BaseClassItem *> &classes,
                                                 const std::unordered_map<std::string, BaseFieldItem *> &fields,
                                                 const std::unordered_map<std::string, BaseMethodItem *> &methods)
{
    auto recordName = annotation.GetName();
    auto it = program.recordTable.find(recordName);
    if (it == program.recordTable.cend()) {
        SetLastError("Record " + recordName + " not found");
        return nullptr;
    }

    auto &record = it->second;
    if (!record.metadata->IsAnnotation()) {
        SetLastError("Record " + recordName + " isn't annotation");
        return nullptr;
    }

    std::vector<AnnotationItem::Elem> itemElements;
    std::vector<AnnotationItem::Tag> tagElements;

    for (const auto &element : annotation.GetElements()) {
        auto name = element.GetName();
        auto *value = element.GetValue();

        auto valueType = value->GetType();

        uint8_t tagType;

        if (valueType == Value::Type::ARRAY && !value->GetAsArray()->GetValues().empty()) {
            auto arrayElementType = value->GetAsArray()->GetComponentType();
            tagType = Value::GetArrayTypeAsChar(arrayElementType);
        } else {
            tagType = Value::GetTypeAsChar(valueType);
        }

        ASSERT(tagType != '0');

        auto functionName = GetMethodSignatureFromProgram(record.name + "." + name, program);
        auto funcIt = program.functionTable.find(functionName);
        if (record.HasImplementation() && funcIt == program.functionTable.cend()) {
            // Definitions of the system annotations may be absent.
            // So print message and continue if corresponding function isn't found.
            LOG(INFO, ASSEMBLER) << "Function " << functionName << " not found";
        } else if (record.HasImplementation() && !CheckValue(value, funcIt->second.returnType, program)) {
            SetLastError("Incorrect annotation element " + functionName + ": " + GetLastError());
            return nullptr;
        }

        auto *item = CreateValueItem(container, value, program, classes, fields, methods);
        if (item == nullptr) {
            SetLastError("Cannot create value item for annotation element " + functionName + ": " + GetLastError());
            return nullptr;
        }

        itemElements.emplace_back(container->GetOrCreateStringItem(name), item);
        tagElements.emplace_back(tagType);
    }

    auto *cls = classes.find(recordName)->second;
    return container->CreateItem<AnnotationItem>(cls, std::move(itemElements), std::move(tagElements));
}

MethodHandleItem *AsmEmitter::CreateMethodHandleItem(ItemContainer *container, const MethodHandle &mh,
                                                     const std::unordered_map<std::string, BaseFieldItem *> &fields,
                                                     const std::unordered_map<std::string, BaseMethodItem *> &methods)
{
    MethodHandleItem *item = nullptr;
    switch (mh.type) {
        case panda_file::MethodHandleType::PUT_STATIC:
        case panda_file::MethodHandleType::GET_STATIC:
        case panda_file::MethodHandleType::PUT_INSTANCE:
        case panda_file::MethodHandleType::GET_INSTANCE: {
            item = container->CreateItem<MethodHandleItem>(mh.type, fields.at(mh.itemName));
            break;
        }
        case panda_file::MethodHandleType::INVOKE_STATIC:
        case panda_file::MethodHandleType::INVOKE_INSTANCE:
        case panda_file::MethodHandleType::INVOKE_CONSTRUCTOR:
        case panda_file::MethodHandleType::INVOKE_DIRECT:
        case panda_file::MethodHandleType::INVOKE_INTERFACE: {
            item = container->CreateItem<MethodHandleItem>(mh.type, methods.at(mh.itemName));
            break;
        }
        default:
            UNREACHABLE();
            break;
    }
    return item;
}

/* static */
template <class T>
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::AddAnnotations(T *item, ItemContainer *container, const AnnotationMetadata &metadata,
                                const Program &program, const std::unordered_map<std::string, BaseClassItem *> &classes,
                                const std::unordered_map<std::string, BaseFieldItem *> &fields,
                                const std::unordered_map<std::string, BaseMethodItem *> &methods)
{
    for (const auto &annotation : metadata.GetAnnotations()) {
        auto *annotationItem = CreateAnnotationItem(container, annotation, program, classes, fields, methods);
        if (annotationItem == nullptr) {
            return false;
        }

        auto &record = program.recordTable.find(annotation.GetName())->second;
        if (record.metadata->IsRuntimeAnnotation()) {
            item->AddRuntimeAnnotation(annotationItem);
        } else if (record.metadata->IsAnnotation()) {
            item->AddAnnotation(annotationItem);
        } else if (record.metadata->IsRuntimeTypeAnnotation()) {
            item->AddRuntimeTypeAnnotation(annotationItem);
        } else if (record.metadata->IsTypeAnnotation()) {
            item->AddTypeAnnotation(annotationItem);
        }
    }

    return true;
}

template <class T>
static void AddBytecodeIndexDependencies(MethodItem *method, const Ins &insn,
                                         const std::unordered_map<std::string, T *> &items)
{
    ASSERT(!insn.ids.empty());

    for (const auto &id : insn.ids) {
        auto it = items.find(id);
        ASSERT_PRINT(it != items.cend(), "Symbol '" << id << "' not found");

        auto *item = it->second;
        ASSERT(item->GetIndexType() != panda_file::IndexType::NONE);
        method->AddIndexDependency(item);
    }
}

static void AddBytecodeIndexDependencies(MethodItem *method, const Function &func,
                                         const AsmEmitter::AsmEntityCollections &entities)
{
    for (const auto &insn : func.ins) {
        if (insn.opcode == Opcode::INVALID) {
            continue;
        }

        if (insn.HasFlag(InstFlags::METHOD_ID)) {
            AddBytecodeIndexDependencies(method, insn, entities.methodItems);
            continue;
        }

        if (insn.HasFlag(InstFlags::FIELD_ID)) {
            AddBytecodeIndexDependencies(method, insn, entities.fieldItems);
            continue;
        }

        if (insn.HasFlag(InstFlags::TYPE_ID)) {
            AddBytecodeIndexDependencies(method, insn, entities.classItems);
            continue;
        }
    }

    for (const auto &catchBlock : func.catchBlocks) {
        if (catchBlock.exceptionRecord.empty()) {
            continue;
        }

        auto it = entities.classItems.find(catchBlock.exceptionRecord);
        ASSERT(it != entities.classItems.cend());
        auto *item = it->second;
        ASSERT(item->GetIndexType() != panda_file::IndexType::NONE);
        method->AddIndexDependency(item);
    }
}

/* static */
void AsmEmitter::MakeStringItems(ItemContainer *items, const Program &program,
                                 AsmEmitter::AsmEntityCollections &entities)
{
    for (const auto &s : program.strings) {
        auto *item = items->GetOrCreateStringItem(s);
        entities.stringItems.insert({s, item});
    }
}

template <Value::Type TYPE, typename CType = ValueTypeHelperT<TYPE>>
static ScalarValue CreateValue(const LiteralArray::Literal &literal)
{
    if constexpr (std::is_same_v<CType, ValueTypeHelperT<TYPE>> && std::is_integral_v<CType>) {
        return ScalarValue::Create<TYPE>(std::get<std::make_unsigned_t<CType>>(literal.value));
    } else {
        return ScalarValue::Create<TYPE>(std::get<CType>(literal.value));
    }
}

template <Value::Type TYPE, typename... T>
static ScalarValue CheckAndCreateArrayValue(const LiteralArray::Literal &literal,
                                            [[maybe_unused]] const Program &program)
{
    ASSERT(program.arrayTypes.find(Type(AnnotationElement::TypeToString(TYPE), 1)) != program.arrayTypes.end());
    return CreateValue<TYPE, T...>(literal);
}

static ScalarValue MakeLiteralItemArrayValue(const LiteralArray::Literal &literal, const Program &program)
{
    switch (literal.tag) {
        case panda_file::LiteralTag::ARRAY_U1:
            return CheckAndCreateArrayValue<Value::Type::U1, bool>(literal, program);
        case panda_file::LiteralTag::ARRAY_U8:
            return CheckAndCreateArrayValue<Value::Type::U8>(literal, program);
        case panda_file::LiteralTag::ARRAY_I8:
            return CheckAndCreateArrayValue<Value::Type::I8>(literal, program);
        case panda_file::LiteralTag::ARRAY_U16:
            return CheckAndCreateArrayValue<Value::Type::U16>(literal, program);
        case panda_file::LiteralTag::ARRAY_I16:
            return CheckAndCreateArrayValue<Value::Type::I16>(literal, program);
        case panda_file::LiteralTag::ARRAY_U32:
            return CheckAndCreateArrayValue<Value::Type::U32>(literal, program);
        case panda_file::LiteralTag::ARRAY_I32:
            return CheckAndCreateArrayValue<Value::Type::I32>(literal, program);
        case panda_file::LiteralTag::ARRAY_U64:
            return CheckAndCreateArrayValue<Value::Type::U64>(literal, program);
        case panda_file::LiteralTag::ARRAY_I64:
            return CheckAndCreateArrayValue<Value::Type::I64>(literal, program);
        case panda_file::LiteralTag::ARRAY_F32:
            return CheckAndCreateArrayValue<Value::Type::F32>(literal, program);
        case panda_file::LiteralTag::ARRAY_F64:
            return CheckAndCreateArrayValue<Value::Type::F64>(literal, program);
        case panda_file::LiteralTag::ARRAY_STRING: {
            [[maybe_unused]] auto stringType =
                Type::FromDescriptor(ark::panda_file::GetStringClassDescriptor(program.lang));
            // `arrayTypes` may contain class name both with / and . depending on source language (workaround
            // for #5776)
            ASSERT(program.arrayTypes.find(Type(stringType, 1)) != program.arrayTypes.end() ||
                   program.arrayTypes.find(Type(stringType.GetPandasmName(), 1)) != program.arrayTypes.end());
            return CreateValue<Value::Type::STRING, std::string>(literal);
        }
        default:
            UNREACHABLE();
    }
}

static ScalarValue MakeLiteralItemValue(const LiteralArray::Literal &literal, const Program &program)
{
    if (literal.IsArray()) {
        return MakeLiteralItemArrayValue(literal, program);
    }
    switch (literal.tag) {
        case panda_file::LiteralTag::TAGVALUE:
        case panda_file::LiteralTag::ACCESSOR:
        case panda_file::LiteralTag::NULLVALUE:
            return CreateValue<Value::Type::U8>(literal);
        case panda_file::LiteralTag::BOOL:
            return CreateValue<Value::Type::U8, bool>(literal);
        case panda_file::LiteralTag::METHODAFFILIATE:
            return CreateValue<Value::Type::U16>(literal);
        case panda_file::LiteralTag::INTEGER:
            return CreateValue<Value::Type::I32>(literal);
        case panda_file::LiteralTag::BIGINT: {
            return CreateValue<Value::Type::I64>(literal);
        }
        case panda_file::LiteralTag::FLOAT:
            return CreateValue<Value::Type::F32>(literal);
        case panda_file::LiteralTag::DOUBLE:
            return CreateValue<Value::Type::F64>(literal);
        case panda_file::LiteralTag::STRING:
            return CreateValue<Value::Type::STRING, std::string>(literal);
        case panda_file::LiteralTag::METHOD:
        case panda_file::LiteralTag::GENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCGENERATORMETHOD:
        case panda_file::LiteralTag::ASYNCMETHOD:
            return CreateValue<Value::Type::METHOD, std::string>(literal);
        default:
            UNREACHABLE();
    }
}

/* static */
void AsmEmitter::MakeLiteralItems(ItemContainer *items, const Program &program,
                                  AsmEmitter::AsmEntityCollections &entities)
{
    for (const auto &[id, l] : program.literalarrayTable) {
        auto literalArrayItem = items->GetOrCreateLiteralArrayItem(id);
        std::vector<panda_file::LiteralItem> literalArray;

        for (auto &literal : l.literals) {
            auto value = MakeLiteralItemValue(literal, program);
            // the return pointer of vector element should not be rewrited
            CreateLiteralItem(items, &value, &literalArray, entities.methodItems);
        }

        literalArrayItem->AddItems(literalArray);
        entities.literalarrayItems.insert({id, literalArrayItem});
    }
}

/* static */
void AsmEmitter::MakeArrayTypeItems(ItemContainer *items, const Program &program,
                                    AsmEmitter::AsmEntityCollections &entities)
{
    for (const auto &t : program.arrayTypes) {
        auto *foreignRecord = items->GetOrCreateForeignClassItem(t.GetDescriptor());
        entities.classItems.insert({t.GetName(), foreignRecord});
    }
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::HandleRecordAsForeign(
    ItemContainer *items, const Program &program, AsmEmitter::AsmEntityCollections &entities,
    const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes, const std::string &name,
    const Record &rec)
{
    Type recordType = Type::FromName(name);
    auto *foreignRecord = items->GetOrCreateForeignClassItem(recordType.GetDescriptor(rec.conflict));
    entities.classItems.insert({name, foreignRecord});
    for (const auto &f : rec.fieldList) {
        ASSERT(f.metadata->IsForeign());
        auto *fieldName = items->GetOrCreateStringItem(pandasm::DeMangleName(f.name));
        std::string fullFieldName = name + "." + f.name;
        if (!f.metadata->IsForeign()) {
            SetLastError("External record " + name + " has a non-external field " + f.name);
            return false;
        }
        auto *typeItem = GetTypeItem(items, primitiveTypes, f.type, program);
        if (typeItem == nullptr) {
            SetLastError("Field " + fullFieldName + " has undefined type");
            return false;
        }
        auto *field = items->CreateItem<ForeignFieldItem>(foreignRecord, fieldName, typeItem);
        entities.fieldItems.insert({fullFieldName, field});
    }
    return true;
}

/* static */
bool AsmEmitter::HandleBaseRecord(ItemContainer *items, const Program &program, const std::string &name,
                                  const Record &baseRec, ClassItem *record)
{
    auto baseName = baseRec.metadata->GetBase();
    if (!baseName.empty()) {
        auto it = program.recordTable.find(baseName);
        if (it == program.recordTable.cend()) {
            SetLastError("Base record " + baseName + " is not defined for record " + name);
            return false;
        }
        auto &rec = it->second;
        Type baseType(baseName, 0);
        if (rec.metadata->IsForeign()) {
            record->SetSuperClass(items->GetOrCreateForeignClassItem(baseType.GetDescriptor(rec.conflict)));
        } else {
            record->SetSuperClass(items->GetOrCreateClassItem(baseType.GetDescriptor(rec.conflict)));
        }
    }
    return true;
}

/* static */
bool AsmEmitter::HandleInterfaces(ItemContainer *items, const Program &program, const std::string &name,
                                  const Record &rec, ClassItem *record)
{
    auto ifaces = rec.metadata->GetInterfaces();
    for (const auto &item : ifaces) {
        auto it = program.recordTable.find(item);
        if (it == program.recordTable.cend()) {
            std::stringstream error;
            error << "Interface record " << item << " is not defined for record " << name;
            SetLastError(error.str());
            return false;
        }
        auto &iface = it->second;
        Type ifaceType(item, 0);
        if (iface.metadata->IsForeign()) {
            record->AddInterface(items->GetOrCreateForeignClassItem(ifaceType.GetDescriptor(iface.conflict)));
        } else {
            record->AddInterface(items->GetOrCreateClassItem(ifaceType.GetDescriptor(iface.conflict)));
        }
    }
    return true;
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::HandleFields(ItemContainer *items, const Program &program, AsmEmitter::AsmEntityCollections &entities,
                              const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes,
                              const std::string &name, const Record &rec, ClassItem *record)
{
    for (const auto &f : rec.fieldList) {
        auto *fieldName = items->GetOrCreateStringItem(pandasm::DeMangleName(f.name));
        std::string fullFieldName = f.name;
        if (!panda_file::IsDummyClassName(name)) {
            fullFieldName.insert(0, name + ".");
        }
        auto *typeItem = GetTypeItem(items, primitiveTypes, f.type, program);
        if (typeItem == nullptr) {
            SetLastError("Field " + fullFieldName + " has undefined type");
            return false;
        }
        BaseFieldItem *field;
        if (f.metadata->IsForeign()) {
            field = items->CreateItem<ForeignFieldItem>(record, fieldName, typeItem);
        } else {
            field = record->AddField(fieldName, typeItem, f.metadata->GetAccessFlags());
        }
        entities.fieldItems.insert({fullFieldName, field});
    }
    return true;
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::HandleRecord(ItemContainer *items, const Program &program, AsmEmitter::AsmEntityCollections &entities,
                              const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes,
                              const std::string &name, const Record &rec)
{
    Type recordType = Type::FromName(name);
    auto *record = items->GetOrCreateClassItem(recordType.GetDescriptor(rec.conflict));
    entities.classItems.insert({name, record});

    record->SetAccessFlags(rec.metadata->GetAccessFlags());
    record->SetSourceLang(rec.language);

    if (!rec.sourceFile.empty()) {
        auto *sourceFileItem = items->GetOrCreateStringItem(rec.sourceFile);
        record->SetSourceFile(sourceFileItem);
    }

    if (!HandleBaseRecord(items, program, name, rec, record)) {
        return false;
    }

    if (!HandleInterfaces(items, program, name, rec, record)) {
        return false;
    }

    if (!HandleFields(items, program, entities, primitiveTypes, name, rec, record)) {
        return false;
    }

    return true;
}

/* static */
bool AsmEmitter::MakeRecordItems(
    ItemContainer *items, const Program &program, AsmEmitter::AsmEntityCollections &entities,
    const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes)
{
    for (const auto &[name, rec] : program.recordTable) {
        if (rec.metadata->IsForeign()) {
            if (!HandleRecordAsForeign(items, program, entities, primitiveTypes, name, rec)) {
                return false;
            }
        } else {
            if (!HandleRecord(items, program, entities, primitiveTypes, name, rec)) {
                return false;
            }
        }
    }
    return true;
}

/* static */
StringItem *AsmEmitter::GetMethodName(ItemContainer *items, const Function &func, const std::string &name)
{
    if (func.metadata->IsCtor()) {
        return items->GetOrCreateStringItem(ark::panda_file::GetCtorName(func.language));
    }

    if (func.metadata->IsCctor()) {
        return items->GetOrCreateStringItem(ark::panda_file::GetCctorName(func.language));
    }

    return items->GetOrCreateStringItem(GetItemName(name));
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::HandleAreaForInner(ItemContainer *items, const Program &program, ClassItem **area,
                                    ForeignClassItem **foreignArea, const std::string &name,
                                    const std::string &recordOwnerName)
{
    auto iter = program.recordTable.find(recordOwnerName);
    if (iter != program.recordTable.end()) {
        auto &rec = iter->second;
        Type recordOwnerType = Type::FromName(recordOwnerName);
        auto descriptor = recordOwnerType.GetDescriptor(rec.conflict);
        if (rec.metadata->IsForeign()) {
            *foreignArea = items->GetOrCreateForeignClassItem(descriptor);
            if (*foreignArea == nullptr) {
                SetLastError("Unable to create external record " + iter->first);
                return false;
            }
        } else {
            *area = items->GetOrCreateClassItem(descriptor);
            (*area)->SetAccessFlags(rec.metadata->GetAccessFlags());
        }
    } else {
        SetLastError("Function " + name + " is bound to undefined record " + recordOwnerName);
        return false;
    }
    return true;
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::HandleRecordOnwer(ItemContainer *items, const Program &program, ClassItem **area,
                                   ForeignClassItem **foreignArea, const std::string &name,
                                   const std::string &recordOwnerName)
{
    if (recordOwnerName.empty()) {
        *area = items->GetOrCreateGlobalClassItem();
        (*area)->SetAccessFlags(ACC_PUBLIC);
        (*area)->SetSourceLang(program.lang);
    } else {
        if (!HandleAreaForInner(items, program, area, foreignArea, name, recordOwnerName)) {
            return false;
        }
    }
    return true;
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::HandleFunctionParams(
    ItemContainer *items, const Program &program, size_t idx, const std::string &name, const Function &func,
    const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes,
    std::vector<MethodParamItem> &params)
{
    for (size_t i = idx; i < func.params.size(); i++) {
        const auto &p = func.params[i].type;
        auto *typeItem = GetTypeItem(items, primitiveTypes, p, program);
        if (typeItem == nullptr) {
            SetLastError("Argument " + std::to_string(i) + " of function " + name + " has undefined type");
            return false;
        }
        params.emplace_back(typeItem);
    }
    return true;
}

/* static */
bool AsmEmitter::HandleFunctionLocalVariables(ItemContainer *items, const Function &func, const std::string &name)
{
    for (const auto &v : func.localVariableDebug) {
        if (v.name.empty()) {
            SetLastError("Function '" + name + "' has an empty local variable name");
            return false;
        }
        if (v.signature.empty()) {
            SetLastError("Function '" + name + "' has an empty local variable signature");
            return false;
        }
        items->GetOrCreateStringItem(v.name);
        // Skip signature and signature type for parameters
        ASSERT(v.reg >= 0);
        if (func.IsParameter(v.reg)) {
            continue;
        }
        items->GetOrCreateStringItem(v.signature);
        if (!v.signatureType.empty()) {
            items->GetOrCreateStringItem(v.signatureType);
        }
    }
    return true;
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::CreateMethodItem(ItemContainer *items, AsmEmitter::AsmEntityCollections &entities,
                                  const Function &func, TypeItem *typeItem, ClassItem *area,
                                  ForeignClassItem *foreignArea, uint32_t accessFlags, StringItem *methodName,
                                  const std::string &mangledName, const std::string &name,
                                  std::vector<MethodParamItem> &params)
{
    auto *proto = items->GetOrCreateProtoItem(typeItem, params);
    BaseMethodItem *method;
    if (foreignArea == nullptr) {
        if (func.metadata->IsForeign()) {
            method = items->CreateItem<ForeignMethodItem>(area, methodName, proto, accessFlags);
        } else {
            method = area->AddMethod(methodName, proto, accessFlags, params);
        }
    } else {
        if (!func.metadata->IsForeign()) {
            SetLastError("Non-external function " + name + " is bound to external record");
            return false;
        }
        method = items->CreateItem<ForeignMethodItem>(foreignArea, methodName, proto, accessFlags);
    }
    entities.methodItems.insert({mangledName, method});
    if (!func.metadata->IsForeign() && func.metadata->HasImplementation()) {
        if (!func.sourceFile.empty()) {
            items->GetOrCreateStringItem(func.sourceFile);
        }
        if (!func.sourceCode.empty()) {
            items->GetOrCreateStringItem(func.sourceCode);
        }
    }
    return true;
}

/* static */
bool AsmEmitter::MakeFunctionItems(
    ItemContainer *items, const Program &program, AsmEmitter::AsmEntityCollections &entities,
    const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes, bool emitDebugInfo)
{
    for (const auto &f : program.functionTable) {
        const auto &[mangled_name, func] = f;

        auto name = pandasm::DeMangleName(mangled_name);

        StringItem *methodName = GetMethodName(items, func, name);

        ClassItem *area = nullptr;
        ForeignClassItem *foreignArea = nullptr;

        std::string recordOwnerName = GetOwnerName(name);
        if (!HandleRecordOnwer(items, program, &area, &foreignArea, name, recordOwnerName)) {
            return false;
        }

        auto params = std::vector<MethodParamItem> {};

        uint32_t accessFlags = func.metadata->GetAccessFlags();

        if (func.params.empty() || func.params[0].type.GetName() != recordOwnerName || func.metadata->IsCctor()) {
            accessFlags |= ACC_STATIC;
        }

        bool isStatic = (accessFlags & ACC_STATIC) != 0;
        size_t idx = isStatic ? 0 : 1;

        if (!HandleFunctionParams(items, program, idx, name, func, primitiveTypes, params)) {
            return false;
        }

        if (emitDebugInfo && !HandleFunctionLocalVariables(items, func, name)) {
            return false;
        }

        auto *typeItem = GetTypeItem(items, primitiveTypes, func.returnType, program);
        if (typeItem == nullptr) {
            SetLastError("Function " + name + " has undefined return type");
            return false;
        }

        if (!CreateMethodItem(items, entities, func, typeItem, area, foreignArea, accessFlags, methodName, mangled_name,
                              name, params)) {
            return false;
        }
    }
    return true;
}

/* static */
bool AsmEmitter::MakeRecordAnnotations(ItemContainer *items, const Program &program,
                                       const AsmEmitter::AsmEntityCollections &entities)
{
    for (const auto &[name, record] : program.recordTable) {
        if (record.metadata->IsForeign()) {
            continue;
        }

        auto *classItem = static_cast<ClassItem *>(Find(entities.classItems, name));
        if (!AddAnnotations(classItem, items, *record.metadata, program, entities.classItems, entities.fieldItems,
                            entities.methodItems)) {
            SetLastError("Cannot emit annotations for record " + record.name + ": " + GetLastError());
            return false;
        }

        for (const auto &field : record.fieldList) {
            auto fieldName = field.name;
            if (!panda_file::IsDummyClassName(name)) {
                fieldName.insert(0, record.name + ".");
            }
            auto *fieldItem = static_cast<FieldItem *>(Find(entities.fieldItems, fieldName));
            if (!AddAnnotations(fieldItem, items, *field.metadata, program, entities.classItems, entities.fieldItems,
                                entities.methodItems)) {
                SetLastError("Cannot emit annotations for field " + fieldName + ": " + GetLastError());
                return false;
            }

            auto res = field.metadata->GetValue();
            if (res) {
                auto value = res.value();
                auto *item = CreateValueItem(items, &value, program, entities.classItems, entities.fieldItems,
                                             entities.methodItems);
                fieldItem->SetValue(item);
            }
        }
    }
    return true;
}

/* static */
void AsmEmitter::SetCodeAndDebugInfo(ItemContainer *items, MethodItem *method, const Function &func, bool emitDebugInfo)
{
    auto *code = items->CreateItem<CodeItem>();
    method->SetCode(code);
    code->AddMethod(method);  // we need it for Profile-Guided optimization

    if (!emitDebugInfo && !func.CanThrow()) {
        return;
    }

    auto *lineNumberProgram = items->CreateLineNumberProgramItem();
    auto *debugInfo = items->CreateItem<DebugInfoItem>(lineNumberProgram);
    if (emitDebugInfo) {
        for (const auto &v : func.localVariableDebug) {
            ASSERT(v.reg >= 0);
            if (func.IsParameter(v.reg)) {
                debugInfo->AddParameter(items->GetOrCreateStringItem(v.name));
            }
        }
    } else {
        auto nparams = method->GetParams().size();
        for (size_t i = 0; i < nparams; i++) {
            debugInfo->AddParameter(nullptr);
        }
    }
    method->SetDebugInfo(debugInfo);
}

/* static */
void AsmEmitter::SetMethodSourceLang(const Program &program, MethodItem *method, const Function &func,
                                     const std::string &name)
{
    std::string recordName = GetOwnerName(name);
    if (recordName.empty()) {
        method->SetSourceLang(func.language);
        return;
    }

    auto &rec = program.recordTable.find(recordName)->second;
    if (rec.language != func.language) {
        method->SetSourceLang(func.language);
    }
}

/* static */
bool AsmEmitter::AddMethodAndParamsAnnotations(ItemContainer *items, const Program &program,
                                               const AsmEmitter::AsmEntityCollections &entities, MethodItem *method,
                                               const Function &func)
{
    if (!AddAnnotations(method, items, *func.metadata, program, entities.classItems, entities.fieldItems,
                        entities.methodItems)) {
        SetLastError("Cannot emit annotations for function " + func.name + ": " + GetLastError());
        return false;
    }

    auto &paramItems = method->GetParams();
    for (size_t protoIdx = 0; protoIdx < paramItems.size(); protoIdx++) {
        size_t paramIdx = method->IsStatic() ? protoIdx : protoIdx + 1;
        auto &param = func.params[paramIdx];
        auto &paramItem = paramItems[protoIdx];
        if (!AddAnnotations(&paramItem, items, *param.metadata, program, entities.classItems, entities.fieldItems,
                            entities.methodItems)) {
            SetLastError("Cannot emit annotations for parameter a" + std::to_string(paramIdx) + " of function " +
                         func.name + ": " + GetLastError());
            return false;
        }
    }

    if (method->HasRuntimeParamAnnotations()) {
        items->CreateItem<ParamAnnotationsItem>(method, true);
    }

    if (method->HasParamAnnotations()) {
        items->CreateItem<ParamAnnotationsItem>(method, false);
    }

    return true;
}

/* static */
bool AsmEmitter::MakeFunctionDebugInfoAndAnnotations(ItemContainer *items, const Program &program,
                                                     const AsmEmitter::AsmEntityCollections &entities,
                                                     bool emitDebugInfo)
{
    for (const auto &[name, func] : program.functionTable) {
        if (func.metadata->IsForeign()) {
            continue;
        }

        auto *method = static_cast<MethodItem *>(Find(entities.methodItems, name));

        if (func.metadata->HasImplementation()) {
            SetCodeAndDebugInfo(items, method, func, emitDebugInfo);
            AddBytecodeIndexDependencies(method, func, entities);
        }

        SetMethodSourceLang(program, method, func, name);

        if (!AddMethodAndParamsAnnotations(items, program, entities, method, func)) {
            return false;
        }
        if (func.profileSize <= MethodItem::MAX_PROFILE_SIZE) {
            method->SetProfileSize(func.profileSize);
        }
    }
    return true;
}

/* static */
void AsmEmitter::FillMap(PandaFileToPandaAsmMaps *maps, AsmEmitter::AsmEntityCollections &entities)
{
    for (const auto &[name, method] : entities.methodItems) {
        maps->methods.insert({method->GetFileId().GetOffset(), std::string(name)});
    }

    for (const auto &[name, field] : entities.fieldItems) {
        maps->fields.insert({field->GetFileId().GetOffset(), std::string(name)});
    }

    for (const auto &[name, cls] : entities.classItems) {
        maps->classes.insert({cls->GetFileId().GetOffset(), std::string(name)});
    }

    for (const auto &[name, str] : entities.stringItems) {
        maps->strings.insert({str->GetFileId().GetOffset(), std::string(name)});
    }

    for (const auto &[name, arr] : entities.literalarrayItems) {
        maps->literalarrays.emplace(arr->GetFileId().GetOffset(), name);
    }
}

/* static */
// CC-OFFNXT(G.FUN.01-CPP) solid logic
void AsmEmitter::EmitDebugInfo(ItemContainer *items, const Program &program, const std::vector<uint8_t> *bytes,
                               const MethodItem *method, const Function &func, const std::string &name,
                               bool emitDebugInfo)
{
    auto *debugInfo = method->GetDebugInfo();
    if (debugInfo == nullptr) {
        return;
    }

    auto *lineNumberProgram = debugInfo->GetLineNumberProgram();
    auto *constantPool = debugInfo->GetConstantPool();

    std::string recordName = GetOwnerName(name);
    std::string recordSourceFile;
    if (!recordName.empty()) {
        auto &rec = program.recordTable.find(recordName)->second;
        recordSourceFile = rec.sourceFile;
    }

    if (!func.sourceFile.empty() && func.sourceFile != recordSourceFile) {
        if (!func.sourceCode.empty()) {
            auto *sourceCodeItem = items->GetOrCreateStringItem(func.sourceCode);
            ASSERT(sourceCodeItem->GetOffset() != 0);
            lineNumberProgram->EmitSetSourceCode(constantPool, sourceCodeItem);
        }
        auto *sourceFileItem = items->GetOrCreateStringItem(func.sourceFile);
        ASSERT(sourceFileItem->GetOffset() != 0);
        lineNumberProgram->EmitSetFile(constantPool, sourceFileItem);
    }
    func.BuildLineNumberProgram(debugInfo, *bytes, items, constantPool, emitDebugInfo);
}

/* static */
bool AsmEmitter::EmitFunctions(ItemContainer *items, const Program &program,
                               const AsmEmitter::AsmEntityCollections &entities, bool emitDebugInfo)
{
    for (const auto &f : program.functionTable) {
        const auto &[name, func] = f;

        if (func.metadata->IsForeign() || !func.metadata->HasImplementation()) {
            continue;
        }

        auto emitter = BytecodeEmitter {};
        auto *method = static_cast<MethodItem *>(Find(entities.methodItems, name));
        if (!func.Emit(emitter, method, entities.methodItems, entities.fieldItems, entities.classItems,
                       entities.stringItems, entities.literalarrayItems)) {
            SetLastError("Internal error during emitting function: " + func.name);
            return false;
        }

        auto *code = method->GetCode();
        code->SetNumVregs(func.regsNum);
        code->SetNumArgs(func.GetParamsNum());

        auto numIns = static_cast<uint32_t>(
            std::count_if(func.ins.begin(), func.ins.end(), [](auto it) { return it.opcode != Opcode::INVALID; }));
        code->SetNumInstructions(numIns);

        auto *bytes = code->GetInstructions();
        auto status = emitter.Build(static_cast<std::vector<unsigned char> *>(bytes));
        if (status != BytecodeEmitter::ErrorCode::SUCCESS) {
            SetLastError("Internal error during emitting binary code, status=" +
                         std::to_string(static_cast<int>(status)));
            return false;
        }
        auto tryBlocks = func.BuildTryBlocks(method, entities.classItems, *bytes);
        for (auto &tryBlock : tryBlocks) {
            code->AddTryBlock(tryBlock);
        }

        EmitDebugInfo(items, program, bytes, method, func, name, emitDebugInfo);
    }
    return true;
}

/* static */
bool AsmEmitter::Emit(ItemContainer *items, const Program &program, PandaFileToPandaAsmMaps *maps, bool emitDebugInfo,
                      ark::panda_file::pgo::ProfileOptimizer *profileOpt)
{
    auto primitiveTypes = CreatePrimitiveTypes(items);

    auto entities = AsmEmitter::AsmEntityCollections {};

    SetLastError("");

    MakeStringItems(items, program, entities);

    MakeArrayTypeItems(items, program, entities);

    if (!MakeRecordItems(items, program, entities, primitiveTypes)) {
        return false;
    }

    if (!MakeFunctionItems(items, program, entities, primitiveTypes, emitDebugInfo)) {
        return false;
    }

    MakeLiteralItems(items, program, entities);

    // Add annotations for records and fields
    if (!MakeRecordAnnotations(items, program, entities)) {
        return false;
    }

    // Add Code and DebugInfo items last due to they have variable size that depends on bytecode
    if (!MakeFunctionDebugInfoAndAnnotations(items, program, entities, emitDebugInfo)) {
        return false;
    }

    if (profileOpt != nullptr) {
        items->ReorderItems(profileOpt);
    }

    items->ComputeLayout();

    if (maps != nullptr) {
        FillMap(maps, entities);
    }

    return EmitFunctions(items, program, entities, emitDebugInfo);
}

// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::Emit(Writer *writer, const Program &program, std::map<std::string, size_t> *stat,
                      PandaFileToPandaAsmMaps *maps, bool debugInfo, ark::panda_file::pgo::ProfileOptimizer *profileOpt)
{
    auto items = ItemContainer {};
    if (!Emit(&items, program, maps, debugInfo, profileOpt)) {
        return false;
    }

    if (stat != nullptr) {
        *stat = items.GetStat();
    }

    return items.Write(writer);
}

// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool AsmEmitter::Emit(const std::string &filename, const Program &program, std::map<std::string, size_t> *stat,
                      PandaFileToPandaAsmMaps *maps, bool debugInfo, ark::panda_file::pgo::ProfileOptimizer *profileOpt)
{
    auto writer = FileWriter(filename);
    if (!writer) {
        SetLastError("Unable to open " + filename + " for writing");
        return false;
    }
    return Emit(&writer, program, stat, maps, debugInfo, profileOpt);
}

std::unique_ptr<const panda_file::File> AsmEmitter::Emit(const Program &program, PandaFileToPandaAsmMaps *maps)
{
    auto items = ItemContainer {};
    if (!Emit(&items, program, maps)) {
        return nullptr;
    }

    size_t size = items.ComputeLayout();
    auto *buffer = new std::byte[size];

    auto writer = MemoryBufferWriter(reinterpret_cast<uint8_t *>(buffer), size);
    if (!items.Write(&writer)) {
        return nullptr;
    }

    os::mem::ConstBytePtr ptr(
        buffer, size, [](std::byte *bufferPtr, [[maybe_unused]] size_t paramSize) noexcept { delete[] bufferPtr; });
    return panda_file::File::OpenFromMemory(std::move(ptr));
}

bool AsmEmitter::AssignProfileInfo(Program *program)
{
    std::unordered_map<size_t, std::vector<Ins *>> instMap;
    constexpr auto SIZES = profiling::GetOrderedProfileSizes();

    /**
     * Since elements in the profile vector should be grouped by its size and ordered in
     * descending order, we first save instructions in map, where key is a profile size.
     * Then we iterate over this map in descending order - from biggest profile element size
     * to smallest. And assign profile index to all instructions with same size.
     */
    for (auto &func : program->functionTable) {
        for (auto &inst : func.second.ins) {
            auto profSize = INST_PROFILE_SIZES[static_cast<unsigned>(inst.opcode)];
            if (profSize != 0) {
                instMap[profSize].push_back(&inst);
            }
        }
        size_t index = 0;
        for (auto it = SIZES.rbegin(); it != SIZES.rend(); ++it) {
            std::vector<Ins *> &vec = instMap[*it];
            for (auto inst : vec) {
                inst->profileId = index;
                index += *it;
            }
            vec.clear();
        }

        func.second.profileSize = index;
    }
    return true;
}

TypeItem *AsmEmitter::GetTypeItem(
    ItemContainer *items, const std::unordered_map<panda_file::Type::TypeId, PrimitiveTypeItem *> &primitiveTypes,
    const Type &type, const Program &program)
{
    if (!type.IsObject()) {
        return Find(primitiveTypes, type.GetId());
    }

    if (type.IsArray()) {
        return items->GetOrCreateForeignClassItem(type.GetDescriptor());
    }

    const auto &name = type.GetName();
    auto iter = program.recordTable.find(name);
    if (iter == program.recordTable.end()) {
        return nullptr;
    }

    auto &rec = iter->second;

    if (rec.metadata->IsForeign()) {
        return items->GetOrCreateForeignClassItem(type.GetDescriptor());
    }

    return items->GetOrCreateClassItem(type.GetDescriptor());
}

// CC-OFFNXT(G.FUN.01-CPP) solid logic
bool Function::Emit(BytecodeEmitter &emitter, panda_file::MethodItem *method,
                    const std::unordered_map<std::string, panda_file::BaseMethodItem *> &methods,
                    const std::unordered_map<std::string, panda_file::BaseFieldItem *> &fields,
                    const std::unordered_map<std::string, panda_file::BaseClassItem *> &classes,
                    const std::unordered_map<std::string_view, panda_file::StringItem *> &strings,
                    const std::unordered_map<std::string, panda_file::LiteralArrayItem *> &literalarrays) const
{
    auto labels = std::unordered_map<std::string_view, ark::Label> {};

    for (const auto &insn : ins) {
        if (insn.setLabel) {
            labels.insert_or_assign(insn.label, emitter.CreateLabel());
        }
    }

    for (const auto &insn : ins) {
        if (insn.setLabel) {
            auto search = labels.find(insn.label);
            ASSERT(search != labels.end());
            emitter.Bind(search->second);
        }

        if (insn.opcode != Opcode::INVALID) {
            if (!insn.Emit(emitter, method, methods, fields, classes, strings, literalarrays, labels)) {
                return false;
            }
        }
    }

    return true;
}

void Function::EmitLocalVariable(panda_file::LineNumberProgramItem *program, ItemContainer *container,
                                 std::vector<uint8_t> *constantPool, uint32_t &pcInc, size_t instructionNumber) const
{
    auto tryEmitPc = [program, constantPool, &pcInc]() -> void {
        if (pcInc != 0) {
            program->EmitAdvancePc(constantPool, pcInc);
            pcInc = 0;
        }
    };
    for (auto &v : localVariableDebug) {
        if (IsParameter(v.reg)) {
            continue;
        }
        if (instructionNumber == v.start) {
            tryEmitPc();
            StringItem *variableName = container->GetOrCreateStringItem(v.name);
            StringItem *variableType = container->GetOrCreateStringItem(v.signature);
            if (v.signatureType.empty()) {
                program->EmitStartLocal(constantPool, v.reg, variableName, variableType);
            } else {
                StringItem *typeSignature = container->GetOrCreateStringItem(v.signatureType);
                program->EmitStartLocalExtended(constantPool, v.reg, variableName, variableType, typeSignature);
            }
        }
        if (instructionNumber == (v.start + v.length)) {
            tryEmitPc();
            program->EmitEndLocal(v.reg);
        }
    }
}

size_t Function::GetLineNumber(size_t i) const
{
    return static_cast<int32_t>(ins[i].insDebug.lineNumber);
}

uint32_t Function::GetColumnNumber(size_t i) const
{
    return static_cast<int32_t>(ins[i].insDebug.columnNumber);
}

void Function::EmitNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constantPool,
                          uint32_t pcInc, int32_t lineInc) const
{
    if (!program->EmitSpecialOpcode(pcInc, lineInc)) {
        if (pcInc != 0) {
            program->EmitAdvancePc(constantPool, pcInc);
            if (!program->EmitSpecialOpcode(0, lineInc)) {
                program->EmitAdvanceLine(constantPool, lineInc);
                program->EmitSpecialOpcode(0, 0);
            }
        } else {
            program->EmitAdvanceLine(constantPool, lineInc);
            program->EmitSpecialOpcode(0, 0);
        }
    }
}

void Function::EmitLineNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constantPool,
                              uint32_t &prevLineNumber, uint32_t &pcInc, size_t instructionNumber) const
{
    auto lineInc = GetLineNumber(instructionNumber) - prevLineNumber;
    if (lineInc != 0) {
        prevLineNumber = GetLineNumber(instructionNumber);
        EmitNumber(program, constantPool, pcInc, lineInc);
        pcInc = 0;
    }
}

void Function::EmitColumnNumber(panda_file::LineNumberProgramItem *program, std::vector<uint8_t> *constantPool,
                                uint32_t &prevColumnNumber, uint32_t &pcInc, size_t instructionNumber) const
{
    uint32_t cn = GetColumnNumber(instructionNumber);
    if (cn != prevColumnNumber) {
        program->EmitColumn(constantPool, pcInc, cn);
        pcInc = 0;
        prevColumnNumber = cn;
    }
}

void Function::BuildLineNumberProgram(panda_file::DebugInfoItem *debugItem, const std::vector<uint8_t> &bytecode,
                                      ItemContainer *container, std::vector<uint8_t> *constantPool,
                                      bool emitDebugInfo) const
{
    auto *program = debugItem->GetLineNumberProgram();

    if (ins.empty()) {
        program->EmitEnd();
        return;
    }

    uint32_t pcInc = 0;
    uint32_t prevLineNumber = GetLineNumber(0);
    uint32_t prevColumnNumber = std::numeric_limits<uint32_t>::max();
    BytecodeInstruction bi(bytecode.data());
    debugItem->SetLineNumber(prevLineNumber);

    for (size_t i = 0; i < ins.size(); i++) {
        if (emitDebugInfo) {
            EmitLocalVariable(program, container, constantPool, pcInc, i);
        }
        if (ins[i].opcode == Opcode::INVALID) {
            continue;
        }

        if (emitDebugInfo || ins[i].CanThrow()) {
            EmitLineNumber(program, constantPool, prevLineNumber, pcInc, i);
        }

        if (ark::panda_file::IsDynamicLanguage(language) && emitDebugInfo) {
            EmitColumnNumber(program, constantPool, prevColumnNumber, pcInc, i);
        }

        pcInc += bi.GetSize();
        bi = bi.GetNext();
    }

    program->EmitEnd();
}

Function::TryCatchInfo Function::MakeOrderAndOffsets(const std::vector<uint8_t> &bytecode) const
{
    std::unordered_map<std::string_view, size_t> tryCatchLabels;
    std::unordered_map<std::string, std::vector<const CatchBlock *>> tryCatchMap;
    std::vector<std::string> tryCatchOrder;

    for (auto &catchBlock : catchBlocks) {
        tryCatchLabels.insert_or_assign(catchBlock.tryBeginLabel, 0);
        tryCatchLabels.insert_or_assign(catchBlock.tryEndLabel, 0);
        tryCatchLabels.insert_or_assign(catchBlock.catchBeginLabel, 0);
        tryCatchLabels.insert_or_assign(catchBlock.catchEndLabel, 0);

        std::string tryKey = catchBlock.tryBeginLabel + ":" + catchBlock.tryEndLabel;
        auto it = tryCatchMap.find(tryKey);
        if (it == tryCatchMap.cend()) {
            std::tie(it, std::ignore) = tryCatchMap.try_emplace(tryKey);
            tryCatchOrder.push_back(tryKey);
        }
        it->second.push_back(&catchBlock);
    }

    BytecodeInstruction bi(bytecode.data());
    size_t pcOffset = 0;

    for (const auto &i : ins) {
        if (i.setLabel) {
            auto it = tryCatchLabels.find(i.label);
            if (it != tryCatchLabels.cend()) {
                tryCatchLabels[i.label] = pcOffset;
            }
        }
        if (i.opcode == Opcode::INVALID) {
            continue;
        }

        pcOffset += bi.GetSize();
        bi = bi.GetNext();
    }

    return Function::TryCatchInfo {tryCatchLabels, tryCatchMap, tryCatchOrder};
}

std::vector<CodeItem::TryBlock> Function::BuildTryBlocks(
    MethodItem *method, const std::unordered_map<std::string, BaseClassItem *> &classItems,
    const std::vector<uint8_t> &bytecode) const
{
    std::vector<CodeItem::TryBlock> tryBlocks;

    if (ins.empty()) {
        return tryBlocks;
    }

    Function::TryCatchInfo tcs = MakeOrderAndOffsets(bytecode);

    for (const auto &tKey : tcs.tryCatchOrder) {
        auto kv = tcs.tryCatchMap.find(tKey);
        ASSERT(kv != tcs.tryCatchMap.cend());
        auto &tryCatchBlocks = kv->second;

        ASSERT(!tryCatchBlocks.empty());

        std::vector<CodeItem::CatchBlock> catchBlockItems;

        for (auto *catchBlock : tryCatchBlocks) {
            auto className = catchBlock->exceptionRecord;

            BaseClassItem *classItem = nullptr;
            if (!className.empty()) {
                auto it = classItems.find(className);
                ASSERT(it != classItems.cend());
                classItem = it->second;
            }

            auto handlerPcOffset = tcs.tryCatchLabels[catchBlock->catchBeginLabel];
            auto handlerCodeSize = tcs.tryCatchLabels[catchBlock->catchEndLabel] - handlerPcOffset;
            catchBlockItems.emplace_back(method, classItem, handlerPcOffset, handlerCodeSize);
        }

        auto tryStartPcOffset = tcs.tryCatchLabels[tryCatchBlocks[0]->tryBeginLabel];
        auto tryEndPcOffset = tcs.tryCatchLabels[tryCatchBlocks[0]->tryEndLabel];
        ASSERT(tryEndPcOffset >= tryStartPcOffset);
        tryBlocks.emplace_back(tryStartPcOffset, tryEndPcOffset - tryStartPcOffset, catchBlockItems);
    }

    return tryBlocks;
}

void Function::DebugDump() const
{
    std::cerr << "name: " << name << std::endl;
    for (const auto &i : ins) {
        std::cerr << i.ToString("\n", true, regsNum);
    }
}

std::string GetOwnerName(std::string name)
{
    name = DeMangleName(name);
    auto superPos = name.find_last_of(PARSE_AREA_MARKER);
    if (superPos == std::string::npos) {
        return "";
    }

    return name.substr(0, superPos);
}

std::string GetItemName(std::string name)
{
    name = DeMangleName(name);
    auto superPos = name.find_last_of(PARSE_AREA_MARKER);
    if (superPos == std::string::npos) {
        return name;
    }

    return name.substr(superPos + 1);
}

}  // namespace ark::pandasm
