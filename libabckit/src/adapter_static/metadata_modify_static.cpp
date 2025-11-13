/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "libabckit/src/adapter_static/metadata_modify_static.h"

#include "libabckit/c/metadata_core.h"
#include "libabckit/c/extensions/arkts/metadata_arkts.h"
#include "inst.h"
#include "libabckit/src/adapter_static/abckit_static.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/ir_impl.h"
#include "libabckit/src/logger.h"
#include "libabckit/src/statuses_impl.h"

#include "libabckit/src/codegen/codegen_static.h"

#include "optimizer/analysis/rpo.h"
#include "src/adapter_static/metadata_inspect_static.h"
#include "src/adapter_static/helpers_static.h"
#include "src/helpers_common.h"
#include "static_core/assembler/assembly-field.h"
#include "static_core/assembler/assembly-program.h"
#include "static_core/assembler/mangling.h"
#include "static_core/assembler/assembly-record.h"
#include "static_core/assembler/meta.h"

#include "static_core/compiler/optimizer/ir/graph_checker.h"
#include "static_core/compiler/optimizer/ir/graph_cloner.h"
#include "static_core/compiler/optimizer/optimizations/cleanup.h"
#include "static_core/compiler/optimizer/optimizations/move_constants.h"
#include "static_core/compiler/optimizer/optimizations/lowering.h"
#include "static_core/bytecode_optimizer/reg_encoder.h"

#include "modify_name_helper.h"
#include "inst_modifier.h"
#include "static_core/plugins/ets/assembler/extension/ets_meta.h"
#include "static_core/plugins/ets/runtime/types/ets_type.h"
#include "utils/function_util.h"

#include <cstdint>
#include <string>
#include <array>

static auto g_implI = AbckitGetInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);
static auto g_implArkI = AbckitGetArktsInspectApiImpl(ABCKIT_VERSION_RELEASE_1_0_0);

// CC-OFFNXT(WordsTool.95) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
namespace libabckit {

static constexpr std::string_view ASYNC_PREFIX = "%%async-";

static ark::pandasm::Type AbckitTypeToPandasmType(const AbckitType &abckitType)
{
    if (abckitType.name == nullptr) {
        LIBABCKIT_LOG(DEBUG) << "AbckitType name is null, inferring from typeId: " << (int)abckitType.id << '\n';
        switch (abckitType.id) {
            case ABCKIT_TYPE_ID_VOID:
                return ark::pandasm::Type::FromName("void");
            case ABCKIT_TYPE_ID_U1:
                return ark::pandasm::Type::FromName("u1");
            case ABCKIT_TYPE_ID_I8:
                return ark::pandasm::Type::FromName("i8");
            case ABCKIT_TYPE_ID_U8:
                return ark::pandasm::Type::FromName("u8");
            case ABCKIT_TYPE_ID_I16:
                return ark::pandasm::Type::FromName("i16");
            case ABCKIT_TYPE_ID_U16:
                return ark::pandasm::Type::FromName("u16");
            case ABCKIT_TYPE_ID_I32:
                return ark::pandasm::Type::FromName("i32");
            case ABCKIT_TYPE_ID_U32:
                return ark::pandasm::Type::FromName("u32");
            case ABCKIT_TYPE_ID_I64:
                return ark::pandasm::Type::FromName("i64");
            case ABCKIT_TYPE_ID_U64:
                return ark::pandasm::Type::FromName("u64");
            case ABCKIT_TYPE_ID_F32:
                return ark::pandasm::Type::FromName("f32");
            case ABCKIT_TYPE_ID_F64:
                return ark::pandasm::Type::FromName("f64");
            default:
                LIBABCKIT_LOG(ERROR) << "Cannot infer type name from typeId: " << (int)abckitType.id << '\n';
                return ark::pandasm::Type::FromName("void");
        }
    }

    auto typeName = abckitType.name->impl;
    if (typeName == "void") {
        return ark::pandasm::Type::FromName("void");
    }

    // process union type
    if (abckitType.types.size() > 1) {
        std::string unionName = "{U";
        for (const auto *type : abckitType.types) {
            if (type != nullptr && type->name != nullptr) {
                unionName += std::string(type->name->impl) + ",";
            }
        }
        if (!abckitType.types.empty()) {
            unionName.pop_back();
        }
        unionName += "}";
        return ark::pandasm::Type::FromName(unionName);
    }

    return ark::pandasm::Type(typeName, abckitType.rank);
}

static AbckitArktsFunction *CreateFunctionImpl(AbckitFile *file, AbckitCoreModule *owningModule,
                                               const std::string &fullFuncName, AbckitType *returnType,
                                               AbckitArktsFunctionParam **params, size_t paramsCount, bool isStatic,
                                               const std::string &visibility)
{
    auto *prog = file->GetStaticProgram();
    LIBABCKIT_INTERNAL_ERROR(prog, nullptr);

    // create AbckitCoreFunction object
    auto newCoreFunc = std::make_unique<AbckitCoreFunction>();
    newCoreFunc->owningModule = owningModule;
    newCoreFunc->returnType = returnType;

    // create pandasm::Function and set attributes
    ark::pandasm::Function pandasmFunc(fullFuncName, ark::panda_file::SourceLang::ETS);
    pandasmFunc.returnType = AbckitTypeToPandasmType(*returnType);
    LIBABCKIT_LOG(DEBUG) << "Return type: " << pandasmFunc.returnType.GetName() << '\n';

    if (pandasmFunc.metadata) {
        if (isStatic) {
            pandasmFunc.metadata->SetAttribute("static");
        }
        std::string accessAttr = "access.function=" + visibility;
        pandasmFunc.metadata->SetAttribute(accessAttr);
        LIBABCKIT_LOG(DEBUG) << "Set function attributes: " << (isStatic ? "static, " : "") << visibility << '\n';
    }

    // process parameters: add this parameter for instance method, then process user parameters
    size_t totalParamsCount = paramsCount;
    if (!isStatic) {
        totalParamsCount++;
    }

    if (totalParamsCount > 0) {
        LIBABCKIT_LOG(DEBUG) << "Processing " << totalParamsCount << " parameters (including this if needed)\n";
        newCoreFunc->parameters.reserve(totalParamsCount);
        pandasmFunc.params.reserve(totalParamsCount);

        if (!isStatic) {
            // create this parameter type using current class reference type
            std::string className = fullFuncName.substr(0, fullFuncName.find_last_of('.'));
            ark::pandasm::Type thisType(className, 0);
            ark::pandasm::Function::Parameter thisParam(thisType, ark::panda_file::SourceLang::ETS);
            pandasmFunc.params.push_back(std::move(thisParam));
            LIBABCKIT_LOG(DEBUG) << "Added this parameter with type: " << className << '\n';
        }

        // process user provided parameters
        if (params != nullptr && paramsCount > 0U) {
            for (size_t i = 0; i < paramsCount; ++i) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                auto *arktsParam = params[i];
                auto *coreParam = arktsParam->core;

                coreParam->function = newCoreFunc.get();

                // construct pandasm parameter
                ark::pandasm::Type type = AbckitTypeToPandasmType(*coreParam->type);
                ark::pandasm::Function::Parameter pandaParam(type, ark::panda_file::SourceLang::ETS);
                pandasmFunc.params.push_back(std::move(pandaParam));

                newCoreFunc->parameters.emplace_back(std::unique_ptr<AbckitCoreFunctionParam>(coreParam));
            }
        }
        LIBABCKIT_LOG(DEBUG) << "All parameters processed successfully\n";
    }

    // add basic function body (default return instruction)
    pandasmFunc.ins.emplace_back(ark::pandasm::Create_RETURN_VOID());
    LIBABCKIT_LOG(DEBUG) << "Added default return.void instruction\n";

    // insert into corresponding function table according to isStatic parameter
    const std::string mangleKey = abckit::util::GenerateFunctionMangleName(pandasmFunc.name, pandasmFunc);
    ark::pandasm::Function *implPtr = nullptr;

    if (isStatic) {
        auto emplaceRes = prog->functionStaticTable.emplace(mangleKey, std::move(pandasmFunc));
        if (!emplaceRes.second) {
            LIBABCKIT_LOG(ERROR) << "Static function already exists: " << mangleKey << '\n';
            statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
            return nullptr;
        }
        implPtr = &emplaceRes.first->second;
        file->nameToFunctionStatic.emplace(mangleKey, newCoreFunc.get());
    } else {
        auto emplaceRes = prog->functionInstanceTable.emplace(mangleKey, std::move(pandasmFunc));
        if (!emplaceRes.second) {
            LIBABCKIT_LOG(ERROR) << "Instance function already exists: " << mangleKey << '\n';
            statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
            return nullptr;
        }
        implPtr = &emplaceRes.first->second;
        file->nameToFunctionInstance.emplace(mangleKey, newCoreFunc.get());
    }

    newCoreFunc->impl = std::make_unique<AbckitArktsFunction>();
    newCoreFunc->GetArkTSImpl()->impl = implPtr;
    newCoreFunc->GetArkTSImpl()->core = newCoreFunc.get();

    owningModule->functions.emplace_back(std::move(newCoreFunc));

    return owningModule->functions.back()->GetArkTSImpl();
}

static constexpr std::string_view GET_FUNCTION_PATTERN = "<get>";
static constexpr std::string_view SET_FUNCTION_PATTERN = "<set>";
static constexpr std::string_view PROPERTY_FUNCTION_PATTERN = "<property>";
// ========================================
// Create / Update
// ========================================
AbckitString *CreateStringStatic(AbckitFile *file, const char *value, size_t len)
{
    LIBABCKIT_LOG(DEBUG) << "\"" << value << "\"" << '\n';
    auto *prog = file->GetStaticProgram();
    const auto &[progValueIter, _] = prog->strings.insert(std::string(value, len));
    const auto &progValue = *progValueIter;
    auto &strings = file->strings;

    if (strings.find(progValue) != strings.end()) {
        return strings.at(progValue).get();
    }

    auto s = std::make_unique<AbckitString>();
    s->impl = progValue;
    strings.insert({progValue, std::move(s)});
    return strings[progValue].get();
}

bool ModuleSetNameStatic(AbckitCoreModule *m, const char *newName)
{
    LIBABCKIT_LOG_FUNC;

    if (!ModifyNameHelper::ModuleRefreshName(m, newName)) {
        return false;
    }

    InstModifier modifier(m->file);
    modifier.Modify();

    return true;
}

bool ModuleFieldSetNameStatic(AbckitCoreModuleField *field, const char *newName)
{
    LIBABCKIT_LOG_FUNC;

    if (!ModifyNameHelper::FieldRefreshName(field, newName)) {
        return false;
    }

    InstModifier modifier(field->owner->file);
    modifier.Modify();

    return true;
}

bool NamespaceSetNameStatic(AbckitCoreNamespace *ns, const char *newName)
{
    LIBABCKIT_LOG_FUNC;

    if (!ModifyNameHelper::NamespaceRefreshName(ns, newName)) {
        return false;
    }

    InstModifier modifier(ns->owningModule->file);
    modifier.Modify();

    return true;
}

bool NamespaceFieldSetNameStatic(AbckitCoreNamespaceField *field, const char *newName)
{
    LIBABCKIT_LOG_FUNC;

    if (!ModifyNameHelper::FieldRefreshName(field, newName)) {
        return false;
    }

    InstModifier modifier(field->owner->owningModule->file);
    modifier.Modify();

    return true;
}

bool AnnotationInterfaceSetNameStatic(AbckitCoreAnnotationInterface *ai, const char *newName)
{
    LIBABCKIT_LOG_FUNC;

    return ModifyNameHelper::AnnotationInterfaceRefreshName(ai, newName);
}

bool AnnotationSetNameStatic(AbckitCoreAnnotation *anno, const char *newName)
{
    LIBABCKIT_LOG_FUNC;

    return ModifyNameHelper::AnnotationRefreshName(anno, newName);
}

bool ClassSetNameStatic(AbckitCoreClass *klass, const char *newName)
{
    LIBABCKIT_LOG_FUNC;

    if (!ModifyNameHelper::ClassRefreshName(klass, newName)) {
        return false;
    }

    InstModifier modifier(klass->owningModule->file);
    modifier.Modify();

    return true;
}

bool ClassFieldSetNameStatic(AbckitCoreClassField *field, const char *newName)
{
    LIBABCKIT_LOG_FUNC;

    if (!ModifyNameHelper::FieldRefreshName(field, newName)) {
        return false;
    }

    InstModifier modifier(field->owner->owningModule->file);
    modifier.Modify();

    return true;
}

bool InterfaceSetNameStatic(AbckitCoreInterface *iface, const char *newName)
{
    LIBABCKIT_LOG_FUNC;

    if (!ModifyNameHelper::InterfaceRefreshName(iface, newName)) {
        return false;
    }

    InstModifier modifier(iface->owningModule->file);
    modifier.Modify();

    return true;
}

bool InterfaceFieldSetNameStatic(AbckitCoreInterfaceField *field, const char *newName)
{
    LIBABCKIT_LOG_FUNC;

    if (!ModifyNameHelper::InterfaceFieldRefreshName(field, newName)) {
        return false;
    }

    InstModifier modifier(field->owner->owningModule->file);
    modifier.Modify();

    return true;
}

bool EnumSetNameStatic(AbckitCoreEnum *enm, const char *newName)
{
    LIBABCKIT_LOG_FUNC;

    if (!ModifyNameHelper::EnumRefreshName(enm, newName)) {
        return false;
    }

    InstModifier modifier(enm->owningModule->file);
    modifier.Modify();

    return true;
}

bool EnumFieldSetNameStatic(AbckitCoreEnumField *field, const char *newName)
{
    LIBABCKIT_LOG_FUNC;

    if (!ModifyNameHelper::FieldRefreshName(field, newName)) {
        return false;
    }

    InstModifier modifier(field->owner->owningModule->file);
    modifier.Modify();

    return true;
}

bool FunctionSetNameStatic(AbckitCoreFunction *function, const char *name)
{
    LIBABCKIT_LOG_FUNC;

    if (!ModifyNameHelper::FunctionRefreshName(function, name)) {
        return false;
    }

    InstModifier modifier(function->owningModule->file);
    modifier.Modify();

    return true;
}

void FunctionSetGraphStatic(AbckitCoreFunction *function, AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;

    auto *func = function->GetArkTSImpl()->GetStaticImpl();

    auto graphImpl =
        compiler::GraphCloner(graph->impl, graph->impl->GetAllocator(), graph->impl->GetLocalAllocator()).CloneGraph();

    graphImpl->RemoveUnreachableBlocks();
    GraphInvalidateAnalyses(graphImpl);
    CheckInvalidOpcodes(graphImpl, false);

    LIBABCKIT_LOG(DEBUG) << "======================== BEFORE CODEGEN ========================\n";
    LIBABCKIT_LOG_DUMP(func->DebugDump(), DEBUG);
    LIBABCKIT_LOG(DEBUG) << "============================================\n";

    LIBABCKIT_LOG_DUMP(graphImpl->Dump(&std::cerr), DEBUG);

    LIBABCKIT_LOG(DEBUG) << "============================================\n";

    if (!ark::compiler::GraphChecker(graphImpl).Check()) {
        LIBABCKIT_LOG(DEBUG) << func->name << ": Graph Verifier failed!\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }

    graphImpl->RunPass<compiler::Lowering>();

    graphImpl->RunPass<compiler::Cleanup>(false);

    graphImpl->RunPass<compiler::MoveConstants>();

    if (!AllocateRegisters(graphImpl, CodeGenStatic::RESERVED_REG)) {
        LIBABCKIT_LOG(DEBUG) << func->name << ": RegAllocGraphColoring failed!\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return;
    }

    if (!graphImpl->RunPass<bytecodeopt::RegEncoder>()) {
        LIBABCKIT_LOG(DEBUG) << func->name << ": RegEncoder failed!\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return;
    }

    if (!graphImpl->RunPass<CodeGenStatic>(func, graph->irInterface)) {
        LIBABCKIT_LOG(DEBUG) << func->name << ": Code generation failed!\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return;
    }

    func->valueOfFirstParam = static_cast<int64_t>(graphImpl->GetStackSlotsCount()) - 1L;
    func->regsNum = static_cast<size_t>(func->valueOfFirstParam + 1U);

    LIBABCKIT_LOG(DEBUG) << "======================== AFTER CODEGEN ========================\n";
    LIBABCKIT_LOG_DUMP(func->DebugDump(), DEBUG);
    LIBABCKIT_LOG(DEBUG) << "============================================\n";
}

AbckitLiteral *FindOrCreateLiteralBoolStatic(AbckitFile *file, bool value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralBoolStaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralU8Static(AbckitFile *file, uint8_t value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralU8StaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralU16Static(AbckitFile *file, uint16_t value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralU16StaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralMethodAffiliateStatic(AbckitFile *file, uint16_t value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralMethodAffiliateStaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralU32Static(AbckitFile *file, uint32_t value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralU32StaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralU64Static(AbckitFile *file, uint64_t value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralU64StaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralFloatStatic(AbckitFile *file, float value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralFloatStaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralDoubleStatic(AbckitFile *file, double value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralDoubleStaticImpl(file, value);
}

AbckitLiteral *FindOrCreateLiteralStringStatic(AbckitFile *file, const char *value, size_t len)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralStringStaticImpl(file, std::string(value, len));
}

AbckitLiteral *FindOrCreateLiteralMethodStatic(AbckitFile *file, AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateLiteralMethodStaticImpl(file, GetMangleFuncName(function));
}

static void EmplaceType(ark::pandasm::Program *prog, AbckitLiteral *value)
{
    switch (LiteralGetTagStatic(value)) {
        case ABCKIT_LITERAL_TAG_BOOL:
            prog->arrayTypes.emplace(pandasm::Type("u1", 1));
            break;
        case ABCKIT_LITERAL_TAG_BYTE:
            prog->arrayTypes.emplace(pandasm::Type("u8", 1));
            break;
        case ABCKIT_LITERAL_TAG_SHORT:
            prog->arrayTypes.emplace(pandasm::Type("u16", 1));
            break;
        case ABCKIT_LITERAL_TAG_INTEGER:
            prog->arrayTypes.emplace(pandasm::Type("u32", 1));
            break;
        case ABCKIT_LITERAL_TAG_LONG:
            prog->arrayTypes.emplace(pandasm::Type("u64", 1));
            break;
        case ABCKIT_LITERAL_TAG_FLOAT:
            prog->arrayTypes.emplace(pandasm::Type("f32", 1));
            break;
        case ABCKIT_LITERAL_TAG_DOUBLE:
            prog->arrayTypes.emplace(pandasm::Type("f64", 1));
            break;
        case ABCKIT_LITERAL_TAG_STRING:
            prog->arrayTypes.emplace(pandasm::Type("reference", 1));
            break;
        default:
            UNREACHABLE();
    }
}

AbckitLiteralArray *CreateLiteralArrayStatic(AbckitFile *file, AbckitLiteral **value, size_t size)
{
    LIBABCKIT_LOG_FUNC;
    auto *prog = file->GetStaticProgram();
    EmplaceType(prog, *value);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto lit = *reinterpret_cast<pandasm::LiteralArray::Literal *>(value[0]->val.get());
    pandasm::LiteralArray::Literal tagLit;
    tagLit.tag = panda_file::LiteralTag::TAGVALUE;
    tagLit.value = static_cast<uint8_t>(lit.tag);
    std::vector<pandasm::LiteralArray::Literal> stdLitArr;
    stdLitArr.emplace_back(tagLit);

    pandasm::LiteralArray::Literal len;
    len.tag = panda_file::LiteralTag::INTEGER;
    len.value = static_cast<uint32_t>(size);
    stdLitArr.emplace_back(len);

    prog->arrayTypes.emplace(pandasm::Type("u32", 1));
    for (size_t i = 0; i < size; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto lit = *reinterpret_cast<pandasm::LiteralArray::Literal *>(value[i]->val.get());
        stdLitArr.emplace_back(lit);
    }

    // NOLINTNEXTLINE(cert-msc51-cpp)
    uint32_t arrayOffset = 0;
    while (prog->literalarrayTable.find(std::to_string(arrayOffset)) != prog->literalarrayTable.end()) {
        arrayOffset++;
    }
    auto arrayName = std::to_string(arrayOffset);

    prog->literalarrayTable.emplace(arrayName, pandasm::LiteralArray());
    pandasm::LiteralArray &arrImpl = prog->literalarrayTable[arrayName];
    arrImpl.literals = std::move(stdLitArr);

    auto litarr = std::make_unique<AbckitLiteralArray>(file, &arrImpl);
    return file->litarrs.emplace_back(std::move(litarr)).get();
}

AbckitValue *FindOrCreateValueU1Static(AbckitFile *file, bool value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateValueU1StaticImpl(file, value);
}

AbckitValue *FindOrCreateValueIntStatic(AbckitFile *file, int value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateValueIntStaticImpl(file, value);
}

AbckitValue *FindOrCreateValueDoubleStatic(AbckitFile *file, double value)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateValueDoubleStaticImpl(file, value);
}

AbckitValue *FindOrCreateValueStringStatic(AbckitFile *file, const char *value, size_t len)
{
    LIBABCKIT_LOG_FUNC;
    return FindOrCreateValueStringStaticImpl(file, std::string(value, len));
}

static std::unique_ptr<AbckitCoreAnnotation> CreateAnnotation(
    AbckitArktsAnnotationInterface *ai,
    std::variant<AbckitCoreClass *, AbckitCoreFunction *, AbckitCoreClassField *, AbckitCoreInterface *,
                 AbckitCoreInterfaceField *>
        owner,
    AbckitString *annoName)
{
    auto anno = std::make_unique<AbckitCoreAnnotation>();
    anno->ai = ai->core;
    anno->name = annoName;
    anno->owner = owner;
    anno->impl = std::make_unique<AbckitArktsAnnotation>();
    anno->GetArkTSImpl()->core = anno.get();
    return anno;
}

static uint32_t GetFieldCreateFlag(const struct AbckitArktsFieldCreateParams *params)
{
    uint32_t flag = 0;
    if (params->isStatic) {
        flag |= ACC_STATIC;
    }
    switch (params->fieldVisibility) {
        case PUBLIC:
            flag |= ACC_PUBLIC;
            break;
        case PRIVATE:
            flag |= ACC_PRIVATE;
            break;
        case PROTECTED:
            flag |= ACC_PROTECTED;
            break;
        default:
            break;
    }
    return flag;
}

static uint32_t GetInterfaceFieldCreateFlag(const struct AbckitArktsInterfaceFieldCreateParams *params)
{
    uint32_t flag = 0;
    if (params->isReadOnly) {
        flag |= ACC_READONLY;
    }
    switch (params->fieldVisibility) {
        case PUBLIC:
            flag |= ACC_PUBLIC;
            break;
        case PRIVATE:
            flag |= ACC_PRIVATE;
            break;
        case PROTECTED:
            flag |= ACC_PROTECTED;
            break;
        default:
            break;
    }
    return flag;
}

// ========================================
// Enum
// ========================================
AbckitArktsEnumField *EnumAddFieldStatic(AbckitCoreEnum *enm, const struct AbckitArktsFieldCreateParams *params)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(enm, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->type, nullptr);

    if (params->value != nullptr && params->type->id != ValueGetTypeStatic(params->value)->id) {
        LIBABCKIT_LOG(ERROR) << "field type is not same value type\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return nullptr;
    }

    std::string name(params->name);
    auto record = enm->GetArkTSImpl()->impl.GetStaticClass();
    auto iter = std::find_if(record->fieldList.begin(), record->fieldList.end(),
                             [&name](auto &field) { return field.name == name; });
    if (iter != record->fieldList.end()) {
        LIBABCKIT_LOG(DEBUG) << "field name already exist.\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    auto prog = enm->owningModule->file->GetStaticProgram();
    auto progField = pandasm::Field(prog->lang);
    progField.name = name;
    progField.type = pandasm::Type(TypeToNameStatic(params->type), params->type->rank);
    if (params->value != nullptr) {
        progField.metadata->SetValue(*reinterpret_cast<pandasm::ScalarValue *>(params->value->val.get()));
    }
    progField.metadata->SetAccessFlags(GetFieldCreateFlag(params));
    record->fieldList.emplace_back(std::move(progField));

    // record->fieldList's emplace_back operation may lead to memory reallocation, pandasm::Field's address may change
    enm->fields.clear();
    AbckitArktsEnumField *retPtr = nullptr;
    for (auto &tmpField : record->fieldList) {
        auto coreField = EnumCreateField(enm->owningModule->file, enm, tmpField);
        if (EnumFieldGetNameStatic(coreField.get())->impl == name) {
            retPtr = coreField->GetArkTSImpl();
        }
        enm->fields.emplace_back(std::move(coreField));
    }

    return retPtr;
}

// ========================================
// ModuleField
// ========================================
bool ModuleFieldSetTypeStatic(AbckitArktsModuleField *field, AbckitType *type)
{
    field->core->type = type;
    field->GetStaticImpl()->type = pandasm::Type(TypeToNameStatic(type), type->rank);

    field->core->value = nullptr;
    field->GetStaticImpl()->metadata->ResetValue();
    return true;
}

bool ModuleFieldSetValueStatic(AbckitArktsModuleField *field, AbckitValue *value)
{
    if (field->core->type->id != ValueGetTypeStatic(value)->id) {
        LIBABCKIT_LOG(ERROR) << "field type is not same value type\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    field->core->value = value;
    field->GetStaticImpl()->metadata->SetValue(*reinterpret_cast<pandasm::ScalarValue *>(value->val.get()));
    return true;
}

// ========================================
// ClassField
// ========================================
bool ClassFieldAddAnnotationStatic(AbckitArktsClassField *field, const AbckitArktsAnnotationCreateParams *params)
{
    auto ai = params->ai;
    auto progFunc = field->GetStaticImpl();
    auto name = ai->GetStaticImpl()->name;
    std::vector<pandasm::AnnotationData> vec;
    vec.emplace_back(name);
    progFunc->metadata->AddAnnotations(vec);

    auto annoName = AnnotationInterfaceGetNameStatic(ai->core);
    std::string annotationName(annoName->impl);

    field->core->annotations.emplace_back(CreateAnnotation(ai, field->core->owner, annoName));
    field->core->annotationTable.emplace(annotationName, CreateAnnotation(ai, field->core->owner, annoName));
    return true;
}

bool ClassFieldRemoveAnnotationStatic(AbckitArktsClassField *field, AbckitArktsAnnotation *anno)
{
    auto name = anno->core->name->impl;
    auto &annotations = field->core->annotations;
    auto iter = std::find_if(annotations.begin(), annotations.end(),
                             [&name](auto &annoIt) { return name == annoIt.get()->name->impl; });
    if (iter == annotations.end()) {
        LIBABCKIT_LOG(ERROR) << "Can not find the annotation in annotations to delete\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    annotations.erase(iter);

    auto it = field->core->annotationTable.find(std::string(name));
    if (it == field->core->annotationTable.end()) {
        LIBABCKIT_LOG(ERROR) << "Can not find the annotation in annotationTable to delete\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    field->core->annotationTable.erase(it);

    auto progFunc = field->GetStaticImpl();
    progFunc->metadata->DeleteAnnotationByName(name);
    return true;
}

bool ClassFieldSetTypeStatic(AbckitArktsClassField *field, AbckitType *type)
{
    field->core->type = type;
    field->GetStaticImpl()->type = pandasm::Type(TypeToNameStatic(type), type->rank);

    field->core->value = nullptr;
    field->GetStaticImpl()->metadata->ResetValue();
    return true;
}

bool ClassFieldSetValueStatic(AbckitArktsClassField *field, AbckitValue *value)
{
    if (field->core->type->id != ValueGetTypeStatic(value)->id) {
        LIBABCKIT_LOG(ERROR) << "field type is not same value type\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }

    field->core->value = value;
    field->GetStaticImpl()->metadata->SetValue(*reinterpret_cast<pandasm::ScalarValue *>(value->val.get()));
    return true;
}

// ========================================
// Module
// ========================================
AbckitArktsAnnotationInterface *ModuleAddAnnotationInterfaceStatic(
    AbckitCoreModule *m, const struct AbckitArktsAnnotationInterfaceCreateParams *params)
{
    std::string name = m->moduleName->impl.data();
    name += ".";
    name += params->name;
    auto prog = m->file->GetStaticProgram();
    auto progRecord = pandasm::Record {name, prog->lang};
    progRecord.metadata->SetAccessFlags(ACC_ANNOTATION | ACC_ABSTRACT | ACC_PUBLIC);
    prog->recordTable.emplace(name, std::move(progRecord));
    pandasm::Record &record = prog->recordTable.find(name)->second;
    auto ai = std::make_unique<AbckitCoreAnnotationInterface>();
    ai->owningModule = m;
    ai->impl = std::make_unique<AbckitArktsAnnotationInterface>();
    ai->GetArkTSImpl()->impl = &record;
    ai->GetArkTSImpl()->core = ai.get();
    return m->at.emplace(name, std::move(ai)).first->second->GetArkTSImpl();
}

AbckitArktsModuleField *ModuleAddFieldStatic(AbckitCoreModule *m, const struct AbckitArktsFieldCreateParams *params)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(m, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->type, nullptr);

    if (params->value != nullptr && params->type->id != ValueGetTypeStatic(params->value)->id) {
        LIBABCKIT_LOG(ERROR) << "field type is not same value type\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return nullptr;
    }

    std::string name(params->name);
    auto record = m->GetArkTSImpl()->impl.GetStatModule().record;
    auto iter = std::find_if(record->fieldList.begin(), record->fieldList.end(),
                             [&name](auto &field) { return field.name == name; });
    if (iter != record->fieldList.end()) {
        LIBABCKIT_LOG(DEBUG) << "field name already exist.\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    auto prog = m->file->GetStaticProgram();
    auto progField = pandasm::Field(prog->lang);
    progField.name = name;
    progField.type = pandasm::Type(TypeToNameStatic(params->type), params->type->rank);
    if (params->value != nullptr) {
        progField.metadata->SetValue(*reinterpret_cast<pandasm::ScalarValue *>(params->value->val.get()));
    }
    progField.metadata->SetAccessFlags(GetFieldCreateFlag(params));
    record->fieldList.emplace_back(std::move(progField));

    // record->fieldList's emplace_back operation may lead to memory reallocation, pandasm::Field's address may change
    m->fields.clear();
    AbckitArktsModuleField *retPtr = nullptr;
    for (auto &tmpField : record->fieldList) {
        auto coreField = ModuleCreateField(m->file, m, tmpField);
        if (ModuleFieldGetNameStatic(coreField.get())->impl == name) {
            retPtr = coreField->GetArkTSImpl();
        }
        m->fields.emplace_back(std::move(coreField));
    }

    return retPtr;
}

// ========================================
// AnnotationInterface
// ========================================
AbckitArktsAnnotationInterfaceField *AnnotationInterfaceAddFieldStatic(
    AbckitCoreAnnotationInterface *ai, const AbckitArktsAnnotationInterfaceFieldCreateParams *params)
{
    auto name = params->name;
    auto type = params->type;
    auto value = params->defaultValue;

    auto field = std::make_unique<AbckitCoreAnnotationInterfaceField>();
    field->ai = ai;
    field->name = CreateStringStatic(ai->owningModule->file, name, strlen(name));
    field->type = type;
    field->value = value;

    field->impl = std::make_unique<AbckitArktsAnnotationInterfaceField>();
    field->GetArkTSImpl()->core = field.get();
    ai->fields.emplace_back(std::move(field));

    auto record = ai->GetArkTSImpl()->GetStaticImpl();

    auto prog = ai->owningModule->file->GetStaticProgram();
    auto progField = pandasm::Field(prog->lang);
    progField.metadata->SetAccessFlags(ACC_PUBLIC);
    progField.name = name;
    std::string typeName;
    progField.type = pandasm::Type(TypeToNameStatic(type), type->rank);
    progField.metadata->SetValue(*reinterpret_cast<pandasm::ScalarValue *>(value->val.get()));
    record->fieldList.emplace_back(std::move(progField));

    return ai->fields.back()->GetArkTSImpl();
}

void AnnotationInterfaceRemoveFieldStatic(AbckitCoreAnnotationInterface *ai,
                                          AbckitCoreAnnotationInterfaceField *aiField)
{
    if (aiField->ai != ai) {
        statuses::SetLastError(ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }
    auto str = aiField->name;
    if (str == nullptr) {
        statuses::SetLastError(ABCKIT_STATUS_INTERNAL_ERROR);
        return;
    }
    auto name = str->impl;
    auto record = ai->GetArkTSImpl()->GetStaticImpl();
    auto &fields = record->fieldList;
    auto fieldsIter = std::find_if(fields.begin(), fields.end(), [&name](auto &field) { return name == field.name; });
    if (fieldsIter == fields.end()) {
        LIBABCKIT_LOG(DEBUG) << "Can not find the field to delete\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }
    fields.erase(fieldsIter);
    auto &aiFields = ai->fields;
    auto iter = std::find_if(aiFields.begin(), aiFields.end(),
                             [&name](auto &field) { return name == field.get()->name->impl; });
    if (iter == aiFields.end()) {
        LIBABCKIT_LOG(DEBUG) << "Can not find the field to delete\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }
    aiFields.erase(iter);
}

// ========================================
// InterfaceField
// ========================================
static std::string ReplacePropertyWithId(const std::string &input, const std::string &id)
{
    size_t pos = input.find(PROPERTY_FUNCTION_PATTERN.data());
    if (pos != std::string::npos) {
        std::string result = input;
        result.replace(pos, std::string(PROPERTY_FUNCTION_PATTERN.data()).length(), id);
        return result;
    }

    return input;
}

static bool InterfaceFieldSetTypeModifyFuncSig(AbckitCoreFunction *func, std::string &getMethodName,
                                               std::string &setMethodName, AbckitType *type)
{
    std::string methodName = FunctionGetNameStatic(func)->impl.data();
    if (methodName.compare(0, getMethodName.length(), getMethodName) == 0) {
        if (!FunctionSetReturnTypeStatic(func->GetArkTSImpl(), type)) {
            LIBABCKIT_LOG(DEBUG) << "Modify get func signature error\n";
            return false;
        }
    } else if (methodName.compare(0, setMethodName.length(), setMethodName) == 0) {
        auto newParam = std::make_unique<AbckitCoreFunctionParam>();
        newParam->function = func;
        newParam->type = type;
        AddFunctionUserToAbckitType(type, func);
        newParam->impl = std::make_unique<AbckitArktsFunctionParam>();
        newParam->GetArkTSImpl()->core = newParam.get();

        if (!FunctionRemoveParameterStatic(func->GetArkTSImpl(), 1)) {
            LIBABCKIT_LOG(DEBUG) << "Modify set function Signature error, remove parameter error\n";
            return false;
        }

        if (!FunctionAddParameterStatic(func->GetArkTSImpl(), newParam->GetArkTSImpl())) {
            LIBABCKIT_LOG(DEBUG) << "Modify set function Signature error, add parameter error\n";
            return false;
        }
    }
    return true;
}

static bool ClassModifyTypeAndMethodSig(AbckitCoreClassField *field, std::string &getMethodName,
                                        std::string &setMethodName, AbckitType *type)
{
    for (auto &method : field->owner->methods) {
        if (!InterfaceFieldSetTypeModifyFuncSig(method.get(), getMethodName, setMethodName, type)) {
            LIBABCKIT_LOG(DEBUG) << "Modify function Signature error\n";
            return false;
        }
    }
    if (!ClassFieldSetTypeStatic(field->GetArkTSImpl(), type)) {
        LIBABCKIT_LOG(DEBUG) << "Modify field type error\n";
        return false;
    }
    return true;
}

static bool InterfaceFieldSetTypeDealForObjectLiteral(AbckitCoreInterfaceField *field, AbckitType *type)
{
    for (auto &objectLiteral : field->owner->objectLiterals) {
        for (auto &objectLiteralField : objectLiteral->fields) {
            if (ClassFieldGetNameStatic(objectLiteralField.get())->impl.data() == field->name->impl.data()) {
                std::string objectLiteralFullName = std::string(objectLiteral->owningModule->moduleName->impl.data()) +
                                                    "." + ClassGetNameStatic(objectLiteral.get())->impl.data();
                std::string objectLiteralGetMethodName =
                    ReplacePropertyWithId(field->name->impl.data(), GET_FUNCTION_PATTERN.data()) + ":" +
                    objectLiteralFullName;
                std::string objectLiteralSetMethodName =
                    ReplacePropertyWithId(field->name->impl.data(), SET_FUNCTION_PATTERN.data()) + ":" +
                    objectLiteralFullName;
                return ClassModifyTypeAndMethodSig(objectLiteralField.get(), objectLiteralGetMethodName,
                                                   objectLiteralSetMethodName, type);
            }
        }
    }
    return true;
}

static bool InterfaceFieldSetTypeDealForInterface(AbckitCoreInterfaceField *field, AbckitType *type)
{
    std::string interfaceFullName(field->owner->GetArkTSImpl()->impl.GetStaticClass()->name);
    std::string interfaceGetMethodName =
        ReplacePropertyWithId(field->name->impl.data(), GET_FUNCTION_PATTERN.data()) + ":" + interfaceFullName;
    std::string interfaceSetMethodName =
        ReplacePropertyWithId(field->name->impl.data(), SET_FUNCTION_PATTERN.data()) + ":" + interfaceFullName;
    for (auto &method : field->owner->methods) {
        if (!InterfaceFieldSetTypeModifyFuncSig(method.get(), interfaceGetMethodName, interfaceSetMethodName, type)) {
            LIBABCKIT_LOG(DEBUG) << "Modify function Signature error\n";
            return false;
        }
    }
    return true;
}

static bool InterfaceFieldSetTypeDealForClass(AbckitCoreInterfaceField *field, AbckitType *type)
{
    for (auto &cls : field->owner->classes) {
        for (auto &clsField : cls->fields) {
            if (ClassFieldGetNameStatic(clsField.get())->impl.data() == field->name->impl.data()) {
                std::string classFullName = std::string(clsField->owner->owningModule->moduleName->impl.data()) + "." +
                                            ClassGetNameStatic(clsField->owner)->impl.data();
                std::string classGetMethodName =
                    ReplacePropertyWithId(field->name->impl.data(), GET_FUNCTION_PATTERN.data()) + ":" + classFullName;
                std::string classSetMethodName =
                    ReplacePropertyWithId(field->name->impl.data(), SET_FUNCTION_PATTERN.data()) + ":" + classFullName;
                return ClassModifyTypeAndMethodSig(clsField.get(), classGetMethodName, classSetMethodName, type);
            }
        }
    }
    return true;
}

bool InterfaceFieldSetTypeStatic(AbckitArktsInterfaceField *field, AbckitType *type)
{
    if (!InterfaceFieldSetTypeDealForObjectLiteral(field->core, type)) {
        LIBABCKIT_LOG(DEBUG) << "InterfaceFieldSetNameDealForObjectLiteral error\n";
        statuses::SetLastError(ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }

    if (!InterfaceFieldSetTypeDealForInterface(field->core, type)) {
        LIBABCKIT_LOG(DEBUG) << "InterfaceFieldSetNameDealForInterface error\n";
        statuses::SetLastError(ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }

    if (!InterfaceFieldSetTypeDealForClass(field->core, type)) {
        LIBABCKIT_LOG(DEBUG) << "InterfaceFieldSetNameDealForClass error\n";
        statuses::SetLastError(ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }

    field->core->type = type;
    return true;
}

bool InterfaceFieldAddAnnotationStatic(AbckitArktsInterfaceField *field,
                                       const AbckitArktsAnnotationCreateParams *params)
{
    auto ai = params->ai;
    auto name = ai->GetStaticImpl()->name;
    std::vector<pandasm::AnnotationData> vec;
    vec.emplace_back(name);

    std::string interfaceFullName(field->core->owner->GetArkTSImpl()->impl.GetStaticClass()->name);
    std::string interfaceGetMethodName =
        ReplacePropertyWithId(field->core->name->impl.data(), GET_FUNCTION_PATTERN.data()) + ":" + interfaceFullName;
    std::string interfaceSetMethodName =
        ReplacePropertyWithId(field->core->name->impl.data(), SET_FUNCTION_PATTERN.data()) + ":" + interfaceFullName;
    for (auto &method : field->core->owner->methods) {
        std::string methodName = FunctionGetNameStatic(method.get())->impl.data();
        if (methodName.compare(0, interfaceGetMethodName.length(), interfaceGetMethodName) == 0) {
            auto progFunc = method->GetArkTSImpl()->GetStaticImpl();
            progFunc->metadata->AddAnnotations(vec);
        } else if (methodName.compare(0, interfaceSetMethodName.length(), interfaceSetMethodName) == 0) {
            auto progFunc = method->GetArkTSImpl()->GetStaticImpl();
            progFunc->metadata->AddAnnotations(vec);
        }
    }

    auto annoName = AnnotationInterfaceGetNameStatic(ai->core);

    field->core->annotations.emplace_back(CreateAnnotation(ai, field->core->owner, annoName));
    return true;
}

bool InterfaceFieldRemoveAnnotationStatic(AbckitArktsInterfaceField *field, AbckitArktsAnnotation *anno)
{
    auto name = anno->core->name->impl;
    auto &annotations = field->core->annotations;
    auto iter = std::find_if(annotations.begin(), annotations.end(),
                             [&name](auto &annoIt) { return name == annoIt.get()->name->impl; });
    if (iter == annotations.end()) {
        LIBABCKIT_LOG(ERROR) << "Can not find the annotation to delete\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return false;
    }
    annotations.erase(iter);

    std::string interfaceFullName(field->core->owner->GetArkTSImpl()->impl.GetStaticClass()->name);
    std::string interfaceGetMethodName =
        ReplacePropertyWithId(field->core->name->impl.data(), GET_FUNCTION_PATTERN.data()) + ":" + interfaceFullName;
    std::string interfaceSetMethodName =
        ReplacePropertyWithId(field->core->name->impl.data(), SET_FUNCTION_PATTERN.data()) + ":" + interfaceFullName;
    for (auto &method : field->core->owner->methods) {
        std::string methodName = FunctionGetNameStatic(method.get())->impl.data();
        if (methodName.compare(0, interfaceGetMethodName.length(), interfaceGetMethodName) == 0) {
            auto progFunc = method->GetArkTSImpl()->GetStaticImpl();
            progFunc->metadata->DeleteAnnotationByName(name);
        } else if (methodName.compare(0, interfaceSetMethodName.length(), interfaceSetMethodName) == 0) {
            auto progFunc = method->GetArkTSImpl()->GetStaticImpl();
            progFunc->metadata->DeleteAnnotationByName(name);
        }
    }

    return true;
}

// ========================================
// Function
// ========================================

bool FunctionAddParameterStatic(AbckitArktsFunction *func, AbckitArktsFunctionParam *param)
{
    LIBABCKIT_LOG_FUNC;

    AbckitCoreFunction *coreFunc = func->core;
    auto *funcImpl = func->GetStaticImpl();
    LIBABCKIT_INTERNAL_ERROR(funcImpl, false);

    auto *prog = coreFunc->owningModule->file->GetStaticProgram();
    LIBABCKIT_INTERNAL_ERROR(prog, false);

    std::string oldMangleName = abckit::util::GenerateFunctionMangleName(funcImpl->name, *funcImpl);

    const auto *paramCore = param->core;
    const auto *paramType = paramCore->type;

    std::string componentName = abckit::util::GetTypeName(coreFunc, paramType, false);
    abckit::util::AddFunctionParameterImpl(coreFunc, funcImpl, paramCore, componentName);

    std::string newMangleName = abckit::util::GenerateFunctionMangleName(funcImpl->name, *funcImpl);
    if (!abckit::util::UpdateFunctionTableKey(prog, funcImpl, funcImpl->name, oldMangleName, newMangleName)) {
        LIBABCKIT_LOG(ERROR) << "[FunctionAddParameter] Failed to update function table key" << std::endl;
        return false;
    }

    abckit::util::ReplaceInstructionIds(prog, funcImpl, oldMangleName, newMangleName);
    abckit::util::UpdateFileMap(coreFunc->owningModule->file, oldMangleName, newMangleName);

    return true;
}

bool FunctionRemoveParameterStatic(AbckitArktsFunction *func, size_t index)
{
    LIBABCKIT_LOG_FUNC;

    AbckitCoreFunction *coreFunc = func->core;
    auto *funcImpl = func->GetStaticImpl();
    LIBABCKIT_INTERNAL_ERROR(funcImpl, false);

    auto *prog = std::get<ark::pandasm::Program *>(coreFunc->owningModule->file->program);
    LIBABCKIT_INTERNAL_ERROR(prog, false);

    std::string oldMangleName = abckit::util::GenerateFunctionMangleName(funcImpl->name, *funcImpl);

    if (!abckit::util::RemoveFunctionParameterByIndexImpl(coreFunc, funcImpl, index)) {
        LIBABCKIT_LOG(ERROR) << "[FunctionRemoveParameter] Failed to remove parameter at index " << index << std::endl;
        return false;
    }
    std::string newMangleName = abckit::util::GenerateFunctionMangleName(funcImpl->name, *funcImpl);
    if (!abckit::util::UpdateFunctionTableKey(prog, funcImpl, funcImpl->name, oldMangleName, newMangleName)) {
        LIBABCKIT_LOG(ERROR) << "[FunctionRemoveParameter] Failed to update function table key" << std::endl;
        return false;
    }

    abckit::util::ReplaceInstructionIds(prog, funcImpl, oldMangleName, newMangleName);
    abckit::util::UpdateFileMap(coreFunc->owningModule->file, oldMangleName, newMangleName);
    return true;
}

bool FunctionSetReturnTypeStatic(AbckitArktsFunction *func, AbckitType *abckitType)
{
    LIBABCKIT_LOG_FUNC;

    AbckitCoreFunction *coreFunc = func->core;
    auto *funcImpl = func->GetStaticImpl();
    LIBABCKIT_INTERNAL_ERROR(funcImpl, false);

    auto *prog = std::get<ark::pandasm::Program *>(func->core->owningModule->file->program);
    LIBABCKIT_INTERNAL_ERROR(prog, false);

    std::string oldMangleName = abckit::util::GenerateFunctionMangleName(funcImpl->name, *funcImpl);
    ark::pandasm::Type type(abckit::util::GetTypeName(coreFunc, abckitType, false), abckitType->rank);
    funcImpl->returnType = type;
    std::string newMangleName = abckit::util::GenerateFunctionMangleName(funcImpl->name, *funcImpl);
    if (!abckit::util::UpdateFunctionTableKey(prog, funcImpl, funcImpl->name, oldMangleName, newMangleName)) {
        LIBABCKIT_LOG(ERROR) << "[FunctionSetReturnType] Failed to update function table key" << std::endl;
        return false;
    }

    abckit::util::ReplaceInstructionIds(prog, funcImpl, oldMangleName, newMangleName);
    abckit::util::UpdateFileMap(coreFunc->owningModule->file, oldMangleName, newMangleName);
    return true;
}

// ========================================
// Class
// ========================================
AbckitArktsClass *CreateClassStatic(AbckitCoreModule *m, const char *name)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_INTERNAL_ERROR(m->file->GetStaticProgram(), nullptr);
    LIBABCKIT_INTERNAL_ERROR(m->moduleName, nullptr);

    auto fullName = std::string(m->moduleName->impl) + '.' + name;
    auto prog = m->file->GetStaticProgram();
    auto progRecord = pandasm::Record(fullName, prog->lang);
    progRecord.metadata->SetAttributeValue("ets.extends", "std.core.Object");
    progRecord.metadata->SetAttributeValue("access.record", "public");
    prog->recordTable.try_emplace(fullName, std::move(progRecord));
    pandasm::Record &record = prog->recordTable.find(fullName)->second;
    auto klass = std::make_unique<AbckitCoreClass>(m, AbckitArktsClass(&record));

    m->ct.try_emplace(name, std::move(klass));
    return (m->ct)[name]->GetArkTSImpl();
}

template <typename TblType>
static void RemoveFuncInTable(TblType &table, const std::string &name)
{
    for (auto iter = table.begin(); iter != table.end();) {
        if (iter->first.find(name) != std::string::npos) {
            iter = table.erase(iter);
        } else {
            ++iter;
        }
    }
}

bool ClassRemoveMethodStatic(AbckitCoreClass *klass, AbckitCoreFunction *method)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_INTERNAL_ERROR(klass->owningModule->file->GetStaticProgram(), false);

    auto funcNameWithParas = GetMangleFuncName(method);
    auto &methods = klass->methods;
    auto func = method->GetArkTSImpl()->GetStaticImpl();
    auto isStatic = func->IsStatic();
    auto iter = std::find_if(methods.begin(), methods.end(),
                             [&func](auto &funcIt) { return funcIt->GetArkTSImpl()->GetStaticImpl() == func; });
    if (iter == methods.end()) {
        LIBABCKIT_LOG(ERROR) << "Can not find the methods to delete\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    methods.erase(iter);

    auto prog = klass->owningModule->file->GetStaticProgram();
    auto file = klass->owningModule->file;
    if (isStatic) {
        RemoveFuncInTable(prog->functionStaticTable, funcNameWithParas);
        RemoveFuncInTable(file->nameToFunctionStatic, funcNameWithParas);
    } else {
        RemoveFuncInTable(prog->functionInstanceTable, funcNameWithParas);
        RemoveFuncInTable(file->nameToFunctionInstance, funcNameWithParas);
    }

    return true;
}

bool ClassAddInterfaceStatic(AbckitArktsClass *klass, AbckitArktsInterface *iface)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_INTERNAL_ERROR(klass->impl.GetStaticClass(), false);
    LIBABCKIT_INTERNAL_ERROR(klass->impl.GetStaticClass()->metadata, false);
    LIBABCKIT_INTERNAL_ERROR(klass->core->owningModule->file->GetStaticProgram(), false);
    LIBABCKIT_INTERNAL_ERROR(iface->impl.GetStaticClass(), false);

    AbckitCoreInterface *ifaceCore = iface->core;
    ark::pandasm::Record *record = klass->impl.GetStaticClass();
    std::string fullName = iface->impl.GetStaticClass()->name;

    // if the target interface is not in the program, add it
    auto ifaceRecord = iface->impl.GetStaticClass();
    ark::pandasm::Program *prog = klass->core->owningModule->file->GetStaticProgram();
    if (prog->recordTable.find(fullName) == prog->recordTable.end()) {
        prog->recordTable.emplace(fullName, std::move(*ifaceRecord));
        iface->impl.cl = &(prog->recordTable.find(fullName)->second);
    }

    auto it = record->metadata->GetInterfaces();
    if (std::find(it.begin(), it.end(), fullName) == it.end()) {
        auto res = record->metadata->AddInterface(fullName);
        if (!res) {
            LIBABCKIT_LOG(ERROR) << "Failed to add interface\n";
            statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        }
    } else {
        LIBABCKIT_LOG(ERROR) << "The interface already exists\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
    }

    auto &interfaces = klass->core->interfaces;
    auto interfaceName = std::get<1>(ClassGetNames(fullName));
    auto iter = std::find_if(interfaces.begin(), interfaces.end(), [&interfaceName](auto &interIt) {
        return interfaceName == InterfaceGetNameStatic(interIt)->impl;
    });
    if (iter == interfaces.end()) {
        interfaces.push_back(ifaceCore);
    } else {
        LIBABCKIT_LOG(ERROR) << "The interface already exists\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
    }

    return statuses::GetLastError() == AbckitStatus::ABCKIT_STATUS_NO_ERROR;
}

bool ClassRemoveInterfaceStatic(AbckitArktsClass *klass, AbckitArktsInterface *iface)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_INTERNAL_ERROR(klass->impl.GetStaticClass(), false);
    LIBABCKIT_INTERNAL_ERROR(klass->impl.GetStaticClass()->metadata, false);
    LIBABCKIT_INTERNAL_ERROR(iface->impl.GetStaticClass(), false);

    ark::pandasm::Record *record = klass->impl.GetStaticClass();
    std::string fullName = iface->impl.GetStaticClass()->name;
    auto interfaceName = std::get<1>(ClassGetNames(fullName));

    auto &interfaces = klass->core->interfaces;
    auto iter = std::find_if(interfaces.begin(), interfaces.end(), [&interfaceName](auto &interIt) {
        return interfaceName == InterfaceGetNameStatic(interIt)->impl;
    });
    if (iter != interfaces.end()) {
        interfaces.erase(iter);
    } else {
        LIBABCKIT_LOG(ERROR) << "Can not find the interface to delete\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
    }

    auto res = record->metadata->RemoveInterface(fullName);
    if (!res) {
        LIBABCKIT_LOG(ERROR) << "Failed to remove the interface from metadata\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
    }

    return statuses::GetLastError() == AbckitStatus::ABCKIT_STATUS_NO_ERROR;
}

bool ClassSetSuperClassStatic(AbckitArktsClass *klass, AbckitArktsClass *superClass)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_INTERNAL_ERROR(klass->impl.GetStaticClass(), false);
    LIBABCKIT_INTERNAL_ERROR(klass->impl.GetStaticClass()->metadata, false);
    LIBABCKIT_INTERNAL_ERROR(superClass->impl.GetStaticClass(), false);

    if ((klass->core->superClass != nullptr) &&
        (klass->core->superClass->GetArkTSImpl()->impl.GetStaticClass()->name != "std.core.Object")) {
        LIBABCKIT_LOG(ERROR) << "Class already has a super class\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }

    klass->core->superClass = superClass->core;

    auto superClassName = superClass->impl.GetStaticClass()->name;
    ark::pandasm::Record *record = klass->impl.GetStaticClass();
    auto res = record->metadata->SetBase(superClassName);
    if (!res) {
        LIBABCKIT_LOG(ERROR) << "Failed to set super class in metadata\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }

    return true;
}

static void UpdateClassNameInFuncParams(std::vector<ark::pandasm::Function::Parameter> &params,
                                        const std::string &oldName, const std::string &setNewName)
{
    for (auto &param : params) {
        if (param.type.GetComponentName().find(oldName) != std::string::npos) {
            param.type = pandasm::Type(setNewName, param.type.GetRank());
        }
    }
}

bool CheckIsNeedAddMethod(ark::pandasm::Program *program, AbckitArktsFunction *method,
                          const std::string &moduleClassName)
{
    LIBABCKIT_INTERNAL_ERROR(program, false);
    LIBABCKIT_INTERNAL_ERROR(method, false);
    auto funcNameCropModule = std::string(g_implI->functionGetName(method->core)->impl.data());
    auto isStatic = method->GetStaticImpl()->IsStatic();
    if (isStatic) {
        auto funcName = std::string(moduleClassName) + "." + funcNameCropModule;
        auto &funcStaticTable = program->functionStaticTable;
        if (funcStaticTable.find(funcName) != funcStaticTable.end()) {
            return false;
        }
    } else {
        size_t colonPos = funcNameCropModule.find(':');
        if (colonPos != std::string::npos) {
            std::string paras = funcNameCropModule.substr(colonPos + 1);
            size_t semicolonPos = paras.find(';');
            if (semicolonPos != std::string::npos) {
                paras.replace(0, semicolonPos, moduleClassName);
            }
            auto funcName = moduleClassName + "." + funcNameCropModule.substr(0, colonPos) + ":" + paras;
            auto &funcInstanceTable = program->functionInstanceTable;
            if (funcInstanceTable.find(funcName) != funcInstanceTable.end()) {
                return false;
            }
        }
    }
    return true;
}

static AbckitType *PandasmTypeToAbckitType(AbckitFile *file, const pandasm::Type &pandasmType)
{
    auto typeId = ArkPandasmTypeToAbckitTypeId(pandasmType);
    auto rank = pandasmType.GetRank();
    auto abckitName = CreateStringStatic(file, pandasmType.GetName().c_str(), pandasmType.GetName().length());
    return GetOrCreateType(file, typeId, rank, nullptr, abckitName);
}

static std::unique_ptr<AbckitCoreFunctionParam> CreateFunctionParam(AbckitCoreFunction *function,
                                                                    const pandasm::Function::Parameter &parameter)
{
    auto param = std::make_unique<AbckitCoreFunctionParam>();
    param->function = function;
    param->type = PandasmTypeToAbckitType(function->owningModule->file, parameter.type);

    return param;
}

static std::unique_ptr<AbckitCoreAnnotation> CreateAnnotation(AbckitFile *file, AbckitCoreFunction *function,
                                                              const std::string &name)
{
    auto [_, annotationName] = ClassGetNames(name);
    auto anno = std::make_unique<AbckitCoreAnnotation>();
    anno->name = CreateStringStatic(file, annotationName.data(), annotationName.size());
    anno->owner = function;
    anno->impl = std::make_unique<AbckitArktsAnnotation>();
    anno->GetArkTSImpl()->core = anno.get();
    return anno;
}

std::unique_ptr<AbckitCoreFunction> CreateCoreFunction(AbckitCoreModule *md, ark::pandasm::Function &functionImpl)
{
    auto function = std::make_unique<AbckitCoreFunction>();
    function->owningModule = md;
    function->impl = std::make_unique<AbckitArktsFunction>();
    function->GetArkTSImpl()->impl = &functionImpl;
    function->GetArkTSImpl()->core = function.get();

    auto annoNames = functionImpl.metadata->GetAttributeValues("ets.annotation.class");
    for (const auto &annoName : annoNames) {
        function->annotations.emplace_back(CreateAnnotation(md->file, function.get(), annoName));
        function->annotationTable.emplace(annoName, function->annotations.back().get());
    }

    for (auto &functionParam : functionImpl.params) {
        function->parameters.emplace_back(CreateFunctionParam(function.get(), functionParam));
    }

    function->returnType = PandasmTypeToAbckitType(md->file, functionImpl.returnType);
    AddFunctionUserToAbckitType(function->returnType, function.get());

    return function;
}

template <typename AbckitArktsType>
std::unique_ptr<ark::pandasm::Function> CreatePandasmFunction(AbckitArktsType *instance, AbckitArktsFunction *method,
                                                              const std::string &newFuncName)
{
    auto funcData = method->GetStaticImpl();
    size_t colonPos = newFuncName.find(':');
    auto newFunction = std::make_unique<pandasm::Function>(newFuncName.substr(0, colonPos), funcData->language);
    newFunction->returnType = funcData->returnType;
    newFunction->sourceFile = funcData->sourceFile;
    newFunction->metadata->SetAccessFlags(funcData->metadata->GetAccessFlags());

    for (auto &param : funcData->params) {
        newFunction->params.emplace_back(param.type, param.lang);
    }
    if (!funcData->IsStatic()) {
        auto thisType = pandasm::Type(instance->impl.GetStaticClass()->name, 0);
        newFunction->params[0] = pandasm::Function::Parameter(thisType, instance->impl.GetStaticClass()->language);
    }
    auto &setAttributes = funcData->metadata->GetBoolAttributes();
    for (auto &sa : setAttributes) {
        newFunction->metadata->SetAttribute(sa);
    }
    auto &attributes = funcData->metadata->GetAttributes();
    for (auto &[key, valueVec] : attributes) {
        for (auto &value : valueVec) {
            newFunction->metadata->SetAttributeValue(key, value);
        }
    }
    if (funcData->HasImplementation()) {
        for (auto &tmpIns : funcData->ins) {
            ark::pandasm::Ins clone = tmpIns.Clone();
            newFunction->ins.emplace_back(std::move(clone));
        }
        newFunction->regsNum = funcData->regsNum;
        for (auto &tmpBlock : funcData->catchBlocks) {
            newFunction->catchBlocks.emplace_back(tmpBlock);
        }
    }
    return newFunction;
}

static void UpdateClassName(std::string &input, const std::string &oldName, const std::string &newName)
{
    size_t pos = 0;
    while ((pos = input.find(oldName, pos)) != std::string::npos) {
        input.replace(pos, oldName.length(), newName);
        pos += newName.length();
    }
}

static void UpdateClassNameInFuncIns(std::vector<ark::pandasm::Ins> &ins, const std::string &oldName,
                                     const std::string &setNewName)
{
    for (auto &tmpIns : ins) {
#if defined(NOT_OPTIMIZE_PERFORMANCE)
        for (auto &tmpIds : tmpIns.ids) {
            if (tmpIds.find(oldName) != std::string::npos) {
                UpdateClassName(tmpIds, oldName, setNewName);
            }
        }
#else
        std::size_t count = tmpIns.IDSize();
        for (std::size_t i = 0; i < count; i++) {
            if (tmpIns.GetID(i).find(oldName) != std::string::npos) {
                auto &str = const_cast<std::string &>(tmpIns.GetID(i));
                UpdateClassName(str, oldName, setNewName);
                tmpIns.SetID(i, str);
            }
        }
#endif
    }
}

bool ClassRemoveFieldStatic(AbckitArktsClass *klass, AbckitCoreClassField *field)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_INTERNAL_ERROR(klass, false);
    LIBABCKIT_INTERNAL_ERROR(klass->core, false);
    LIBABCKIT_INTERNAL_ERROR(klass->impl.GetStaticClass(), false);

    LIBABCKIT_INTERNAL_ERROR(field, false);

    auto nameStr = field->GetArkTSImpl()->GetStaticImpl()->name;
    auto iter = std::find_if(klass->core->fields.begin(), klass->core->fields.end(), [&nameStr](auto &tempfieldIter) {
        return nameStr == tempfieldIter->GetArkTSImpl()->GetStaticImpl()->name;
    });
    if (iter == klass->core->fields.end()) {
        LIBABCKIT_LOG(ERROR) << "Can not find the field to delete\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    klass->core->fields.erase(iter);

    auto fieldIter =
        std::find_if(klass->impl.GetStaticClass()->fieldList.begin(), klass->impl.GetStaticClass()->fieldList.end(),
                     [&nameStr](auto &tempIter) { return nameStr == tempIter.name; });
    if (fieldIter == klass->impl.GetStaticClass()->fieldList.end()) {
        LIBABCKIT_LOG(ERROR) << "Can not find the field to delete\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    klass->impl.GetStaticClass()->fieldList.erase(fieldIter);
    return true;
}

template <typename FuncTblProgram>
static void UpdateClassNameInFuncTable(FuncTblProgram &table, const std::string &oldName, const std::string &setNewName)
{
    FuncTblProgram tmpTable;
    for (auto iter = table.begin(); iter != table.end();) {
        if (iter->first.find(oldName) != std::string::npos) {
            auto node = table.extract(iter++);
            UpdateClassName(node.key(), oldName, setNewName);
            UpdateClassName(node.mapped().name, oldName, setNewName);
            UpdateClassNameInFuncParams(node.mapped().params, oldName, setNewName);
            UpdateClassNameInFuncIns(node.mapped().ins, oldName, setNewName);
            tmpTable.insert(std::move(node));
        } else {
            UpdateClassNameInFuncParams(iter->second.params, oldName, setNewName);
            UpdateClassNameInFuncIns(iter->second.ins, oldName, setNewName);
            ++iter;
        }
    }

    for (auto it = tmpTable.begin(); it != tmpTable.end();) {
        auto node = tmpTable.extract(it++);
        table.insert(std::move(node));
    }
}

template <typename CommonTbl>
static void UpdateClassNameInCommonTbl(CommonTbl &table, const std::string &oldName, const std::string &setNewName)
{
    CommonTbl tmpTable;
    for (auto iter = table.begin(); iter != table.end();) {
        if (iter->first.find(oldName) != std::string::npos) {
            auto node = table.extract(iter++);
            UpdateClassName(node.key(), oldName, setNewName);
            tmpTable.insert(std::move(node));
        } else {
            ++iter;
        }
    }
    table.insert(std::make_move_iterator(tmpTable.begin()), std::make_move_iterator(tmpTable.end()));
}

static void UpdateModuleAndClassName(AbckitFile *file, const std::string &oldName, const std::string &setNewName)
{
    LIBABCKIT_INTERNAL_ERROR_VOID(file);
    auto program = file->GetStaticProgram();
    auto iter = std::find_if(program->recordTable.begin(), program->recordTable.end(),
                             [&oldName](const auto &pair) { return pair.first == oldName; });
    if (iter != program->recordTable.end()) {
        auto node = program->recordTable.extract(iter);
        node.key() = setNewName;
        node.mapped().name = setNewName;
        program->recordTable.insert(std::move(node));
    }
    UpdateClassNameInFuncTable(program->functionStaticTable, oldName, setNewName);
    UpdateClassNameInFuncTable(program->functionInstanceTable, oldName, setNewName);
    UpdateClassNameInCommonTbl(file->nameToFunctionStatic, oldName, setNewName);
    UpdateClassNameInCommonTbl(file->nameToFunctionInstance, oldName, setNewName);
}

static void UpdateExtendsInfo(AbckitFile *file, const std::string &oldName, const std::string &setNewName)
{
    LIBABCKIT_INTERNAL_ERROR_VOID(file);
    for (auto &[name, record] : file->GetStaticProgram()->recordTable) {
        auto baseStr = record.metadata->GetBase();
        if (baseStr == oldName) {
            record.metadata->RemoveAttributeValue("ets.extends", oldName);
            record.metadata->SetAttributeValue("ets.extends", setNewName);
        }
    }
}

AbckitArktsClassField *ClassAddFieldStatic(AbckitCoreClass *klass, const struct AbckitArktsFieldCreateParams *params)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(klass, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->type, nullptr);

    if (params->value != nullptr && params->type->id != ValueGetTypeStatic(params->value)->id) {
        LIBABCKIT_LOG(ERROR) << "field type is not same value type\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return nullptr;
    }

    std::string name(params->name);
    auto record = klass->GetArkTSImpl()->impl.GetStaticClass();
    auto it = std::find_if(record->fieldList.begin(), record->fieldList.end(),
                           [&name](auto &field) { return field.name == name; });
    if (it != record->fieldList.end()) {
        LIBABCKIT_LOG(ERROR) << "field name already exist.\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    auto prog = klass->owningModule->file->GetStaticProgram();
    auto progField = pandasm::Field(prog->lang);
    progField.name = name;
    progField.type = pandasm::Type(TypeToNameStatic(params->type), params->type->rank);
    if (params->value != nullptr) {
        progField.metadata->SetValue(*reinterpret_cast<pandasm::ScalarValue *>(params->value->val.get()));
    }
    progField.metadata->SetAccessFlags(GetFieldCreateFlag(params));
    record->fieldList.emplace_back(std::move(progField));

    // record->fieldList's emplace_back operation may lead to memory reallocation, pandasm::Field's address may change
    klass->fields.clear();
    AbckitArktsClassField *retPtr = nullptr;
    for (auto &tmpField : record->fieldList) {
        auto classField = ClassCreateField(klass->owningModule->file, klass, tmpField);
        if (ClassFieldGetNameStatic(classField.get())->impl == name) {
            retPtr = classField->GetArkTSImpl();
        }
        klass->fields.emplace_back(std::move(classField));
    }
    return retPtr;
}

template <typename TblType>
static void UpdateClassInfoToNewModule(TblType &table1, TblType &table2, const std::string &name)
{
    for (auto iter = table1.begin(); iter != table1.end();) {
        if (iter->first.find(name) != std::string::npos) {
            auto node = table1.extract(iter++);
            table2.insert(std::move(node));
        } else {
            ++iter;
        }
    }
}

template <typename VecType>
static void UpdateOwningModuleInfo(VecType &vec, AbckitCoreModule *module)
{
    for (auto &item : vec) {
        item->owningModule = module;
    }
}

bool ClassSetOwningModuleStatic(AbckitCoreClass *klass, AbckitCoreModule *module)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(klass, false);
    LIBABCKIT_BAD_ARGUMENT(module, false);
    LIBABCKIT_INTERNAL_ERROR(klass->owningModule, false);

    if ((klass->owningModule->file) != (module->file)) {
        LIBABCKIT_LOG(ERROR) << "new module and current module should be in same AbckitFile.\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }
    auto className = g_implI->abckitStringToString(g_implI->classGetName(klass));
    ark::pandasm::Record *record = klass->GetArkTSImpl()->impl.GetStaticClass();
    auto [tmpModuleName, tmpClassName] = ClassGetNames(record->name);
    auto oldModuleName = tmpModuleName;
    auto newModuleName = g_implI->abckitStringToString(module->moduleName);
    auto setNewName = std::string(newModuleName) + "." + className;
    auto oldName = std::string(oldModuleName) + "." + className;
    if (newModuleName == oldModuleName) {
        LIBABCKIT_LOG(ERROR) << "same module. No need to setOwningModule\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return false;
    }
    auto file = module->file;
    UpdateModuleAndClassName(file, oldName, setNewName);
    UpdateClassInfoToNewModule(klass->owningModule->ct, module->ct, className);
    UpdateExtendsInfo(klass->owningModule->file, oldName, setNewName);
    UpdateOwningModuleInfo(klass->methods, module);
    UpdateOwningModuleInfo(klass->interfaces, module);
    klass->owningModule = module;
    return true;
}

// ========================================
// Interface
// ========================================
template <typename TblType>
static void RemoveFuncInTable(TblType &table, const std::string &interfaceName, const std::string &name)
{
    std::string cleanFieldName = name;
    const std::string propertyPrefix = "<property>";
    if (cleanFieldName.find(propertyPrefix) == 0) {
        cleanFieldName = cleanFieldName.substr(propertyPrefix.length());
    }

    for (auto iter = table.begin(); iter != table.end();) {
        if (iter->first.find(cleanFieldName + ":") != std::string::npos &&
            iter->first.find(std::string(interfaceName)) != std::string::npos) {
            iter = table.erase(iter);
        } else {
            ++iter;
        }
    }
}

bool InterfaceRemoveFieldStatic(AbckitArktsInterface *iface, AbckitCoreInterfaceField *field)
{
    LIBABCKIT_LOG_FUNC;

    if (field->name == nullptr) {
        LIBABCKIT_LOG(ERROR) << "Field name is nullptr\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    auto fieldName = field->name->impl;
    auto fieldIt = std::find_if(iface->core->fields.begin(), iface->core->fields.end(),
                                [&fieldName](auto &field) { return field.first == fieldName; });
    if (fieldIt == iface->core->fields.end()) {
        LIBABCKIT_LOG(ERROR) << "Field does not exist\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    iface->core->fields.erase(fieldIt);

    auto interfaceName = g_implI->interfaceGetName(iface->core)->impl;
    RemoveFuncInTable(iface->core->owningModule->file->GetStaticProgram()->functionInstanceTable,
                      std::string(interfaceName), std::string(fieldName));
    RemoveFuncInTable(iface->core->owningModule->file->nameToFunctionInstance, std::string(interfaceName),
                      std::string(fieldName));
    return true;
}

AbckitArktsInterface *CreateInterfaceStatic(AbckitArktsModule *m, const char *name)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_INTERNAL_ERROR(m->core->file->GetStaticProgram(), nullptr);
    LIBABCKIT_INTERNAL_ERROR(m->core->moduleName, nullptr);

    auto prog = m->core->file->GetStaticProgram();
    auto fullName = std::string(m->core->moduleName->impl) + '.' + name;
    auto record = pandasm::Record(fullName, prog->lang);
    record.metadata->SetAttribute("ets.interface");
    record.metadata->SetAttribute("ets.abstract");
    record.metadata->SetAttributeValue("ets.extends", "std.core.Object");
    record.metadata->SetAttributeValue("access.record", "public");
    prog->recordTable.try_emplace(fullName, std::move(record));

    auto recordPtr = &prog->recordTable.find(fullName)->second;
    auto interface = std::make_unique<AbckitCoreInterface>(m->core, AbckitArktsInterface(recordPtr));
    interface->name = CreateStringStatic(m->core->file, name, strlen(name));
    auto &interfaces = m->core->it;
    interfaces.try_emplace(name, std::move(interface));

    return interfaces[name]->GetArkTSImpl();
}

void UpdateImplementList(std::map<std::string, pandasm::Record> &records, const std::string &ifaceNameOrigin,
                         const std::string &ifaceNameNew)
{
    for (auto &[_, record] : records) {
        if (record.metadata->RemoveInterface(ifaceNameOrigin)) {
            record.metadata->AddInterface(ifaceNameNew);
        }
    }
}

std::unique_ptr<AbckitCoreInterfaceField> CloneInterfaceField(AbckitArktsInterface *iface,
                                                              AbckitCoreInterfaceField *field)
{
    auto newField = std::make_unique<AbckitCoreInterfaceField>();
    newField->name = field->name;
    newField->owner = iface->core;
    newField->flag = field->flag;
    newField->type = field->type;
    newField->impl = std::make_unique<AbckitArktsInterfaceField>();
    newField->GetArkTSImpl()->core = newField.get();
    return newField;
}

std::string InterfaceFieldNameSplit(AbckitCoreInterfaceField *field)
{
    auto fieldName = field->name->impl;
    std::string splitedFieldName;

    if (fieldName.find(ark::ets::PROPERTY) == 0) {
        splitedFieldName = fieldName.substr(ark::ets::PROPERTY_PREFIX_LENGTH);
    } else {
        LIBABCKIT_LOG(ERROR) << "The input field's name is invalid.\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return "";
    }
    return splitedFieldName;
}

std::unique_ptr<pandasm::Function> CreateAccessorFunction(std::string &fieldTypeName, std::string &accessorName,
                                                          std::string &ifaceName, bool isSetter)
{
    auto function = std::make_unique<ark::pandasm::Function>(accessorName, ark::panda_file::SourceLang::ETS);
    function->metadata->SetAttribute("ets.abstract");
    function->metadata->SetAttribute("noimpl");
    function->metadata->SetAttributeValue("access.function", "public");
    function->params.emplace_back(ark::pandasm::Type::FromName(ifaceName), ark::panda_file::SourceLang::ETS);
    if (isSetter) {
        function->params.emplace_back(ark::pandasm::Type::FromName(fieldTypeName), ark::panda_file::SourceLang::ETS);
        function->returnType = ark::pandasm::Type::FromName("void");
    } else {
        function->returnType = ark::pandasm::Type::FromName(fieldTypeName);
    }

    return function;
}

void InterfaceAddFieldAccessor(AbckitArktsInterface *iface, AbckitCoreInterfaceField *field, std::string &accessorName,
                               bool isSetter)
{
    std::string type = TypeToNameStatic(field->type);
    auto function = CreateAccessorFunction(type, accessorName, iface->impl.GetStaticClass()->name, isSetter);
    auto funcMangledName =
        MangleFunctionName(ark::pandasm::DeMangleName(function->name), function->params, function->returnType);
    auto &funcInstanceTab = iface->core->owningModule->file->GetStaticProgram()->functionInstanceTable;
    funcInstanceTab.emplace(funcMangledName, std::move(*function));
    auto &functionImpl = funcInstanceTab.at(funcMangledName);

    auto accessor = CreateCoreFunction(iface->core->owningModule, functionImpl);
    accessor->parent = iface->core;
    auto &nameToFunction = iface->core->owningModule->file->nameToFunctionInstance;
    nameToFunction.emplace(funcMangledName, accessor.get());
    iface->core->methods.emplace_back(std::move(accessor));
}

bool InterfaceAddFieldAccessors(AbckitArktsInterface *iface, AbckitCoreInterfaceField *field)
{
    auto splitedFieldName = InterfaceFieldNameSplit(field);
    if (splitedFieldName.empty()) {
        return false;
    }

    auto prefix = iface->impl.GetStaticClass()->name + ".";
    auto addGetter = ((field->flag & ACC_PUBLIC) != 0U);
    auto addSetter = ((field->flag & ACC_READONLY) == 0U);

    if (addGetter) {
        auto getterName = prefix + ark::ets::GETTER_BEGIN + splitedFieldName;
        InterfaceAddFieldAccessor(iface, field, getterName, false);
    }

    if (addSetter) {
        auto setterName = prefix + ark::ets::SETTER_BEGIN + splitedFieldName;
        InterfaceAddFieldAccessor(iface, field, setterName, true);
    }

    return true;
}

AbckitArktsInterfaceField *InterfaceAddFieldStatic(AbckitArktsInterface *iface,
                                                   const struct AbckitArktsInterfaceFieldCreateParams *params)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_INTERNAL_ERROR(iface->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(iface->impl.GetStaticClass(), nullptr);

    LIBABCKIT_BAD_ARGUMENT(params, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->type, nullptr);

    std::string fieldName = std::string(ark::ets::PROPERTY) + params->name;
    auto &fields = iface->core->fields;
    auto it = fields.find(fieldName);
    if (it != fields.end()) {
        LIBABCKIT_LOG(ERROR) << "The field already exists in the interface\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return nullptr;
    }

    auto coreField = std::make_unique<AbckitCoreInterfaceField>();
    coreField->name = CreateStringStatic(iface->core->owningModule->file, fieldName.c_str(), fieldName.length());
    coreField->type = params->type;
    coreField->owner = iface->core;
    coreField->impl = std::make_unique<AbckitArktsInterfaceField>();
    coreField->GetArkTSImpl()->core = coreField.get();
    coreField->flag = GetInterfaceFieldCreateFlag(params);

    if (!InterfaceAddFieldAccessors(iface, coreField.get())) {
        LIBABCKIT_LOG(ERROR) << "Create set/get function error\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return nullptr;
    }

    iface->core->fields.emplace(fieldName, std::move(coreField));
    return iface->core->fields[fieldName]->GetArkTSImpl();
}

bool InterfaceAddMethodStatic(AbckitArktsInterface *iface, AbckitArktsFunction *method)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_INTERNAL_ERROR(iface->core, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core->owningModule->file, false);
    LIBABCKIT_INTERNAL_ERROR(iface->core->owningModule->file->GetStaticProgram(), false);
    LIBABCKIT_INTERNAL_ERROR(iface->impl.GetStaticClass(), false);

    LIBABCKIT_INTERNAL_ERROR(method->GetStaticImpl(), false);
    LIBABCKIT_INTERNAL_ERROR(method->core, false);
    LIBABCKIT_INTERNAL_ERROR(method->core->owningModule, false);
    LIBABCKIT_INTERNAL_ERROR(method->core->owningModule->file, false);
    LIBABCKIT_INTERNAL_ERROR(method->core->owningModule->file->GetStaticProgram(), false);

    if (method->GetStaticImpl()->IsStatic()) {
        LIBABCKIT_LOG(ERROR) << "Static method should not be added to the interface.\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }

    if (!CheckIsNeedAddMethod(iface->core->owningModule->file->GetStaticProgram(), method,
                              iface->impl.GetStaticClass()->name)) {
        LIBABCKIT_LOG(ERROR) << "Method already exists in the interface\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }

    std::string funcMangledName = GetMangleFuncName(method->core);
    ASSERT(funcMangledName != "__ABCKIT_INVALID__");

    auto funcModuleName = funcMangledName.substr(0, funcMangledName.find(':'));
    auto oldModuleClassName = funcModuleName.substr(0, funcModuleName.rfind('.'));
    UpdateClassName(funcMangledName, oldModuleClassName, iface->impl.GetStaticClass()->name);

    auto funcData = CreatePandasmFunction(iface, method, funcMangledName);
    auto &funcInstanceTab = iface->core->owningModule->file->GetStaticProgram()->functionInstanceTable;
    funcInstanceTab.emplace(funcMangledName, std::move(*funcData));
    auto newCoreFunc = CreateCoreFunction(iface->core->owningModule, funcInstanceTab.at(funcMangledName));
    newCoreFunc->parent = iface->core;
    iface->core->owningModule->file->nameToFunctionInstance.emplace(funcMangledName, newCoreFunc.get());
    iface->core->methods.emplace_back(std::move(newCoreFunc));
    return true;
}

bool InterfaceAddSuperInterfaceStatic(AbckitCoreInterface *iface, AbckitCoreInterface *superIface)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(iface, false);
    LIBABCKIT_BAD_ARGUMENT(superIface, false);

    auto superIfaceName = g_implI->abckitStringToString(g_implI->interfaceGetName(superIface));
    auto it = std::find_if(iface->superInterfaces.begin(), iface->superInterfaces.end(),
                           [&superIfaceName](auto &tmpSuperIface) {
                               auto name = g_implI->abckitStringToString(g_implI->interfaceGetName(tmpSuperIface));
                               return std::string(name) == std::string(superIfaceName);
                           });
    if (it != iface->superInterfaces.end()) {
        LIBABCKIT_LOG(ERROR) << "same superInterface already exits.\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    iface->superInterfaces.emplace_back(superIface);
    auto record = iface->GetArkTSImpl()->impl.GetStaticClass();
    auto superIfaceFulldName = superIface->GetArkTSImpl()->impl.GetStaticClass()->name;
    auto res = record->metadata->AddInterface(superIfaceFulldName);
    if (!res) {
        LIBABCKIT_LOG(ERROR) << "failed to add superiface in metadata.\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    return true;
}

bool InterfaceRemoveSuperInterfaceStatic(AbckitCoreInterface *iface, AbckitCoreInterface *superIface)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(iface, false);
    LIBABCKIT_BAD_ARGUMENT(superIface, false);

    auto superIfaceName = g_implI->abckitStringToString(g_implI->interfaceGetName(superIface));
    auto it = std::find_if(iface->superInterfaces.begin(), iface->superInterfaces.end(),
                           [&superIfaceName](auto &tmpSuperIface) {
                               auto name = g_implI->abckitStringToString(g_implI->interfaceGetName(tmpSuperIface));
                               return std::string(name) == std::string(superIfaceName);
                           });
    if (it == iface->superInterfaces.end()) {
        LIBABCKIT_LOG(ERROR) << "same superInterface not exits.\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    iface->superInterfaces.erase(it);

    auto record = iface->GetArkTSImpl()->impl.GetStaticClass();
    auto superIfaceFulldName = superIface->GetArkTSImpl()->impl.GetStaticClass()->name;
    auto res = record->metadata->RemoveInterface(superIfaceFulldName);
    if (!res) {
        LIBABCKIT_LOG(ERROR) << "failed to remove superiface in metadata.\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    return true;
}

bool InterfaceRemoveMethodStatic(AbckitCoreInterface *iface, AbckitCoreFunction *method)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(iface, false);
    LIBABCKIT_BAD_ARGUMENT(method, false);
    LIBABCKIT_INTERNAL_ERROR(iface->owningModule, false);

    auto funcNameWithParas = GetMangleFuncName(method);
    auto file = iface->owningModule->file;
    auto it = std::find_if(iface->methods.begin(), iface->methods.end(), [&funcNameWithParas](auto &tmpMethod) {
        return GetMangleFuncName(tmpMethod.get()) == funcNameWithParas;
    });
    if (it == iface->methods.end()) {
        LIBABCKIT_LOG(ERROR) << "same method not exits.\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return false;
    }
    iface->methods.erase(it);
    RemoveFuncInTable(file->GetStaticProgram()->functionInstanceTable, funcNameWithParas);
    RemoveFuncInTable(file->nameToFunctionInstance, funcNameWithParas);
    return true;
}

bool InterfaceSetOwningModuleStatic(AbckitCoreInterface *iface, AbckitCoreModule *module)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_BAD_ARGUMENT(iface, false);
    LIBABCKIT_BAD_ARGUMENT(module, false);
    LIBABCKIT_INTERNAL_ERROR(iface->owningModule, false);

    if ((iface->owningModule->file) != (module->file)) {
        LIBABCKIT_LOG(ERROR) << "new module and current module should be in same AbckitFile.\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_UNSUPPORTED);
        return false;
    }
    auto ifaceModuleName = g_implI->abckitStringToString(iface->owningModule->moduleName);
    auto newModuleName = g_implI->abckitStringToString(module->moduleName);
    if (ifaceModuleName == newModuleName) {
        LIBABCKIT_LOG(ERROR) << "same module. No need to setOwningModule\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return false;
    }
    auto fullInterfaceName = iface->GetArkTSImpl()->impl.GetStaticClass()->name;
    auto setNewInterfaceName = fullInterfaceName;
    auto ifaceName = g_implI->abckitStringToString(g_implI->interfaceGetName(iface));
    auto file = iface->owningModule->file;
    UpdateClassName(setNewInterfaceName, ifaceModuleName, newModuleName);
    UpdateModuleAndClassName(file, fullInterfaceName, setNewInterfaceName);
    UpdateClassInfoToNewModule(iface->owningModule->it, module->it, ifaceName);
    UpdateOwningModuleInfo(iface->methods, module);
    // modify derived class/interface's info
    for (auto &[name, record] : file->GetStaticProgram()->recordTable) {
        auto interfaceVec = record.metadata->GetInterfaces();
        if (std::find(interfaceVec.begin(), interfaceVec.end(), fullInterfaceName) != interfaceVec.end()) {
            record.metadata->RemoveInterface(fullInterfaceName);
            record.metadata->AddInterface(setNewInterfaceName);
        }
    }
    iface->owningModule = module;
    return true;
}

AbckitArktsFunction *ModuleAddFunctionStatic(AbckitArktsModule *module, struct AbckitArktsFunctionCreateParams *params)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << "=== ModuleAddFunctionStatic START ===\n";

    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->returnType, nullptr);
    LIBABCKIT_BAD_ARGUMENT(module->core->moduleName, nullptr);

    auto *file = module->core->file;
    LIBABCKIT_INTERNAL_ERROR(file, nullptr);

    const std::string moduleName = std::string(module->core->moduleName->impl);
    if (moduleName.empty()) {
        LIBABCKIT_LOG(ERROR) << "Module name is empty\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    size_t paramsCount = 0;
    if (params->params != nullptr) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        while (params->params[paramsCount] != nullptr) {
            paramsCount++;
        }
    }

    std::string funcName = params->isAsync ? std::string(ASYNC_PREFIX) + params->name : params->name;
    std::string fullFuncName = moduleName + ".ETSGLOBAL." + funcName;

    LIBABCKIT_LOG(DEBUG) << "Function name: " << params->name << '\n';
    LIBABCKIT_LOG(DEBUG) << "Params count: " << paramsCount << '\n';
    LIBABCKIT_LOG(DEBUG) << "Is async: " << (params->isAsync ? "true" : "false") << '\n';
    LIBABCKIT_LOG(DEBUG) << "Full function name: " << fullFuncName << '\n';

    return CreateFunctionImpl(file, module->core, fullFuncName, params->returnType, params->params, paramsCount, true,
                              "public");
}

AbckitArktsFunction *ClassAddMethodStatic(AbckitArktsClass *klass, struct ArktsMethodCreateParams *params)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << "=== ClassAddMethodCreatedStatic START ===\n";

    LIBABCKIT_BAD_ARGUMENT(params->name, nullptr);
    LIBABCKIT_BAD_ARGUMENT(params->returnType, nullptr);
    LIBABCKIT_INTERNAL_ERROR(klass->core, nullptr);
    LIBABCKIT_INTERNAL_ERROR(klass->core->owningModule, nullptr);

    auto *file = klass->core->owningModule->file;
    LIBABCKIT_INTERNAL_ERROR(file, nullptr);

    LIBABCKIT_BAD_ARGUMENT(klass->core->owningModule->moduleName, nullptr);
    const std::string moduleName = std::string(klass->core->owningModule->moduleName->impl);
    if (moduleName.empty()) {
        LIBABCKIT_LOG(ERROR) << "Module name is empty\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    auto *record = klass->impl.GetStaticClass();
    LIBABCKIT_INTERNAL_ERROR(record, nullptr);
    const std::string className = record->name;
    if (className.empty()) {
        LIBABCKIT_LOG(ERROR) << "Class name is empty\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return nullptr;
    }

    std::string methodName = params->isAsync ? std::string(ASYNC_PREFIX) + params->name : params->name;
    std::string fullMethodName = className + "." + methodName;

    constexpr std::array<const char *, 3U> VISIBILITY_STR = {"public", "protected", "private"};
    auto visibilityIndex = static_cast<size_t>(params->methodVisibility);
    if (visibilityIndex >= VISIBILITY_STR.size()) {
        visibilityIndex = 0U;
    }
    std::string visibility = VISIBILITY_STR[visibilityIndex];

    size_t paramsCount = 0;
    if (params->params != nullptr) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        while (params->params[paramsCount] != nullptr) {
            paramsCount++;
        }
    }

    LIBABCKIT_LOG(DEBUG) << "Method name: " << params->name << '\n';
    LIBABCKIT_LOG(DEBUG) << "Params count: " << paramsCount << '\n';
    LIBABCKIT_LOG(DEBUG) << "Is static: " << (params->isStatic ? "true" : "false") << '\n';
    LIBABCKIT_LOG(DEBUG) << "Is async: " << (params->isAsync ? "true" : "false") << '\n';
    LIBABCKIT_LOG(DEBUG) << "Visibility: " << visibility << '\n';
    LIBABCKIT_LOG(DEBUG) << "Full method name: " << fullMethodName << '\n';

    return CreateFunctionImpl(file, klass->core->owningModule, fullMethodName, params->returnType, params->params,
                              paramsCount, params->isStatic, visibility);
}

}  // namespace libabckit
