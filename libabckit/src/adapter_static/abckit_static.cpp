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

#include "abckit_static.h"

#include "name_util.h"
#include "string_util.h"
#include "libabckit/src/helpers_common.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/src/adapter_static/helpers_static.h"
#include "libabckit/src/statuses_impl.h"
#include "libabckit/src/adapter_static/runtime_adapter_static.h"
#include "libabckit/src/logger.h"
#include "src/adapter_static/metadata_modify_static.h"
#include "static_core/abc2program/abc2program_driver.h"
#include "static_core/assembler/assembly-emitter.h"

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <functional>
#include <regex>

// CC-OFFNXT(WordsTool.95 Google) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark;

namespace {
constexpr std::string_view ETS_ANNOTATION_CLASS = "ets.annotation.class";
constexpr std::string_view ETS_ANNOTATION_MODULE = "ets.annotation.Module";
constexpr std::string_view ETS_EXTENDS = "ets.extends";
constexpr std::string_view ETS_IMPLEMENTS = "ets.implements";
constexpr std::string_view ENUM_BASE = "std.core.BaseEnum";
constexpr std::string_view INTERFACE_GET_FUNCTION_PATTERN = ".*<get>(.*)";
constexpr std::string_view INTERFACE_SET_FUNCTION_PATTERN = ".*<set>(.*)";
constexpr std::string_view ASYNC_PREFIX = "%%async-";
constexpr std::string_view ARRAY_ENUM_SUFFIX = "[]";
constexpr std::string_view OBJECT_CLASS = "std.core.Object";
constexpr std::string_view OBJECT_LITERAL_NAME = "$ObjectLiteral";
constexpr std::string_view INTERFACE_FIELD_PREFIX = "<property>";
constexpr std::string_view ASYNC_ORIGINAL_RETURN = "std.core.Promise;";
const std::unordered_set<std::string> GLOBAL_CLASS_NAMES = {"ETSGLOBAL", "_GLOBAL"};

using CoreTypePointer = std::variant<AbckitCoreModule *, AbckitCoreNamespace *, AbckitCoreClass *,
                                     AbckitCoreInterface *, AbckitCoreEnum *, AbckitCoreAnnotationInterface *>;
}  // namespace

namespace libabckit {
// Container of vectors and maps of some kinds of instances
// Including modules, classes, namespaces, interfaces, enums, functions, interfaceObjectLiterals and
// annotationInterfaces
struct Container {
    std::vector<std::unique_ptr<AbckitCoreClass>> classes;
    std::vector<std::unique_ptr<AbckitCoreInterface>> interfaces;
    std::vector<std::unique_ptr<AbckitCoreNamespace>> namespaces;
    std::vector<std::unique_ptr<AbckitCoreEnum>> enums;
    std::vector<std::unique_ptr<AbckitCoreFunction>> functions;
    std::vector<std::unique_ptr<AbckitCoreClass>> objectLiterals;
    std::vector<std::unique_ptr<AbckitCoreAnnotationInterface>> annotationInterfaces;
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> nameToModule;
    std::unordered_map<std::string, AbckitCoreNamespace *> nameToNamespace;
    std::unordered_map<std::string, AbckitCoreClass *> nameToClass;
    std::unordered_map<std::string, AbckitCoreClass *> nameToObjectLiteral;
    std::unordered_map<std::string, AbckitCoreInterface *> nameToInterface;
    std::unordered_map<std::string, AbckitCoreEnum *> nameToEnum;
    std::unordered_map<std::string, AbckitCoreAnnotationInterface *> nameToAnnotationInterface;
};

static void DestroyContainer(void *data)
{
    delete static_cast<Container *>(data);
}

static bool IsModuleName(const std::string &name)
{
    return GLOBAL_CLASS_NAMES.find(name) != GLOBAL_CLASS_NAMES.end();
}

static bool IsEnum(const pandasm::Record &record)
{
    return record.metadata->GetBase() == ENUM_BASE;
}

AbckitString *CreateNameString(AbckitFile *file, const std::string &name)
{
    return CreateStringStatic(file, name.data(), name.size());
}

pandasm::Function *FunctionGetImpl(AbckitCoreFunction *function)
{
    return function->GetArkTSImpl()->GetStaticImpl();
}

std::vector<std::string> GetAnnotationNames(
    const std::variant<pandasm::Function *, pandasm::Record *, pandasm::Field *> impl)
{
    return std::visit([](auto &&object) { return object->metadata->GetAttributeValues(ETS_ANNOTATION_CLASS.data()); },
                      impl);
}

static std::optional<CoreTypePointer> GetCoreTypeByName(AbckitFile *file, const std::string &name)
{
    const auto container = static_cast<Container *>(file->data);
    if (const auto it = container->nameToModule.find(name); it != container->nameToModule.end()) {
        return it->second.get();
    }

    if (const auto it = container->nameToNamespace.find(name); it != container->nameToNamespace.end()) {
        return it->second;
    }

    if (const auto it = container->nameToClass.find(name); it != container->nameToClass.end()) {
        return it->second;
    }

    if (const auto it = container->nameToInterface.find(name); it != container->nameToInterface.end()) {
        return it->second;
    }

    if (const auto it = container->nameToEnum.find(name); it != container->nameToEnum.end()) {
        return it->second;
    }

    if (const auto it = container->nameToAnnotationInterface.find(name);
        it != container->nameToAnnotationInterface.end()) {
        return it->second;
    }

    return std::nullopt;
}

static std::variant<AbckitCoreClass *, AbckitCoreInterface *, AbckitCoreEnum *, std::nullptr_t> GetTypeReference(
    AbckitFile *file, const std::string &typeName)
{
    const auto container = static_cast<Container *>(file->data);
    if (const auto it = container->nameToClass.find(typeName); it != container->nameToClass.end()) {
        return it->second;
    }

    if (const auto it = container->nameToInterface.find(typeName); it != container->nameToInterface.end()) {
        return it->second;
    }

    if (const auto it = container->nameToEnum.find(typeName); it != container->nameToEnum.end()) {
        return it->second;
    }

    if (const auto it = container->nameToObjectLiteral.find(typeName); it != container->nameToObjectLiteral.end()) {
        return it->second;
    }

    LIBABCKIT_LOG(DEBUG) << "no reference found for type:" << typeName << std::endl;

    return nullptr;
}

static AbckitType *PandasmTypeToAbckitType(AbckitFile *file, const pandasm::Type &pandasmType)
{
    auto abckitName = CreateNameString(file, pandasmType.GetName());
    auto typeId = ArkPandasmTypeToAbckitTypeId(pandasmType);
    auto rank = pandasmType.GetRank();
    std::variant<AbckitCoreClass *, AbckitCoreInterface *, AbckitCoreEnum *, std::nullptr_t> reference = nullptr;
    if (pandasmType.IsObject()) {
        reference = GetTypeReference(file, pandasmType.GetName());
    }
    auto type = GetOrCreateType(file, typeId, rank, reference, abckitName);

    if (pandasmType.IsUnion()) {
        auto typeNames = pandasmType.GetComponentNames();
        for (const auto &typeName : typeNames) {
            auto abckitTypeName = CreateNameString(file, typeName);
            std::variant<AbckitCoreClass *, AbckitCoreInterface *, AbckitCoreEnum *, std::nullptr_t> unionReference =
                nullptr;
            if (pandasmType.IsObject()) {
                unionReference = GetTypeReference(file, typeName);
            }
            type->types.emplace_back(GetOrCreateType(file, typeId, rank, unionReference, abckitTypeName));
        }
    }
    return type;
}

static std::pair<std::string, std::string> ClassGetModuleNames(
    const std::string &fullName, const std::unordered_map<std::string, AbckitCoreNamespace *> &nameToNamespace)
{
    auto [moduleName, className] = ClassGetNames(fullName);
    if (nameToNamespace.find(moduleName) != nameToNamespace.end()) {
        moduleName = ClassGetModuleNames(moduleName, nameToNamespace).first;
    }
    return {moduleName, className};
}

static void DumpHierarchy(AbckitFile *file)
{
    std::function<void(AbckitCoreFunction * f, const std::string &indent)> dumpFunc;
    std::function<void(AbckitCoreClass * c, const std::string &indent)> dumpClass;

    dumpFunc = [&dumpFunc, &dumpClass](AbckitCoreFunction *f, const std::string &indent = "") {
        auto fName = FunctionGetImpl(f)->name;
        LIBABCKIT_LOG_NO_FUNC(DEBUG) << indent << fName << std::endl;
        for (auto &cNested : f->nestedClasses) {
            dumpClass(cNested.get(), indent + "  ");
        }
        for (auto &fNested : f->nestedFunction) {
            dumpFunc(fNested.get(), indent + "  ");
        }
    };

    dumpClass = [&dumpFunc](AbckitCoreClass *c, const std::string &indent = "") {
        auto cName = GetStaticImplRecord(c)->name;
        LIBABCKIT_LOG_NO_FUNC(DEBUG) << indent << cName << std::endl;
        for (auto &f : c->methods) {
            dumpFunc(f.get(), indent + "  ");
        }
    };

    std::function<void(AbckitCoreNamespace * n, const std::string &indent)> dumpNamespace =
        [&dumpFunc, &dumpClass, &dumpNamespace](AbckitCoreNamespace *n, const std::string &indent = "") {
            auto &nName = GetStaticImplRecord(n)->name;
            LIBABCKIT_LOG_NO_FUNC(DEBUG) << indent << nName << std::endl;
            for (auto &[_, n] : n->nt) {
                dumpNamespace(n.get(), indent + "  ");
            }
            for (auto &[_, c] : n->ct) {
                dumpClass(c.get(), indent + "  ");
            }
            for (auto &f : n->functions) {
                dumpFunc(f.get(), indent + "  ");
            }
        };

    for (auto &[mName, m] : file->localModules) {
        LIBABCKIT_LOG_NO_FUNC(DEBUG) << mName << std::endl;
        for (auto &[_, n] : m->nt) {
            dumpNamespace(n.get(), "");
        }
        for (auto &[cName, c] : m->ct) {
            dumpClass(c.get(), "");
        }
        for (auto &f : m->functions) {
            dumpFunc(f.get(), "");
        }
    }
}

static std::unique_ptr<AbckitCoreAnnotation> CreateAnnotation(
    AbckitFile *file,
    const std::variant<AbckitCoreClass *, AbckitCoreFunction *, AbckitCoreClassField *, AbckitCoreInterface *,
                       AbckitCoreInterfaceField *>
        owner,
    const std::string &name)
{
    LIBABCKIT_LOG(DEBUG) << "Found annotation :'" << name << "'\n";
    auto anno = std::make_unique<AbckitCoreAnnotation>();
    anno->name = CreateNameString(file, name);
    anno->owner = owner;
    anno->impl = std::make_unique<AbckitArktsAnnotation>();
    anno->GetArkTSImpl()->core = anno.get();
    return anno;
}

static std::unique_ptr<AbckitCoreFunctionParam> CreateFunctionParam(AbckitFile *file,
                                                                    const std::unique_ptr<AbckitCoreFunction> &function,
                                                                    const pandasm::Function::Parameter &parameter)
{
    auto param = std::make_unique<AbckitCoreFunctionParam>();
    param->function = function.get();
    param->type = PandasmTypeToAbckitType(file, parameter.type);
    AddFunctionUserToAbckitType(param->type, function.get());
    param->impl = std::make_unique<AbckitArktsFunctionParam>();
    param->GetArkTSImpl()->core = param.get();

    return param;
}

std::unique_ptr<AbckitCoreFunction> CollectFunction(AbckitFile *file, const std::string &functionName,
                                                    ark::pandasm::Function &functionImpl)
{
    const auto container = static_cast<Container *>(file->data);
    auto [moduleName, className] = FuncGetNames(functionName);
    auto function = std::make_unique<AbckitCoreFunction>();
    if (container->nameToNamespace.find(moduleName) != container->nameToNamespace.end()) {
        moduleName = ClassGetModuleNames(moduleName, container->nameToNamespace).first;
    }
    LIBABCKIT_LOG(DEBUG) << "  Found function. module: '" << moduleName << "' class: '" << className << "' function: '"
                         << functionName << "'\n";
    ASSERT(container->nameToModule.find(moduleName) != container->nameToModule.end());
    function->owningModule = container->nameToModule[moduleName].get();
    function->impl = std::make_unique<AbckitArktsFunction>();
    function->GetArkTSImpl()->impl = &functionImpl;
    function->GetArkTSImpl()->core = function.get();
    auto name = GetMangleFuncName(function.get());
    auto &nameToFunction = functionImpl.IsStatic() ? file->nameToFunctionStatic : file->nameToFunctionInstance;
    ASSERT(nameToFunction.count(name) == 0);
    nameToFunction.insert({name, function.get()});

    for (const auto &anno : GetAnnotationNames(&functionImpl)) {
        function->annotations.emplace_back(CreateAnnotation(file, function.get(), anno));
        function->annotationTable.emplace(anno, function->annotations.back().get());
    }

    for (auto &functionParam : functionImpl.params) {
        function->parameters.emplace_back(CreateFunctionParam(file, function, functionParam));
    }

    function->returnType = PandasmTypeToAbckitType(function->owningModule->file, functionImpl.returnType);
    AddFunctionUserToAbckitType(function->returnType, function.get());

    return function;
}

static std::unique_ptr<AbckitCoreInterfaceField> CreateInterfaceField(
    const std::unique_ptr<AbckitCoreInterface> &interface, AbckitCoreFunction *function, const std::string &fieldName)
{
    auto field = std::make_unique<AbckitCoreInterfaceField>();
    auto file = interface->owningModule->file;
    field->name = CreateNameString(file, fieldName);
    field->impl = std::make_unique<AbckitArktsInterfaceField>();
    field->GetArkTSImpl()->core = field.get();
    field->owner = interface.get();
    field->flag = (ACC_PUBLIC | ACC_READONLY);

    auto returnType = function->GetArkTSImpl()->GetStaticImpl()->returnType;
    // Interface field is accessed by getter and setter, so we don't need to bind interface field type
    field->type = PandasmTypeToAbckitType(file, returnType);

    for (const auto &anno : function->annotations) {
        auto annotation = CreateAnnotation(file, field.get(), anno->name->impl.data());
        annotation->ai = anno->ai;
        annotation->ai->annotations.emplace_back(annotation.get());
        field->annotations.emplace_back(std::move(annotation));
    }
    return field;
}

template <typename AbckitCoreType, typename AbckitCoreTypeField>
static std::unique_ptr<AbckitCoreTypeField> CreateField(AbckitFile *file, AbckitCoreType *owner,
                                                        pandasm::Field &recordField)
{
    auto field = std::make_unique<AbckitCoreTypeField>(owner, &recordField);
    if constexpr (std::is_same_v<AbckitCoreType, AbckitCoreAnnotationInterface>) {
        field->name = CreateNameString(file, recordField.name);
    }
    field->type = PandasmTypeToAbckitType(file, recordField.type);
    auto optionalValue = recordField.metadata->GetValue();
    field->value = optionalValue.has_value() ? FindOrCreateValueStatic(file, optionalValue.value()) : nullptr;
    AddFieldUserToAbckitType(field->type, field.get());

    if (recordField.metadata->GetValue().has_value()) {
        field->value = FindOrCreateValueStatic(file, recordField.metadata->GetValue().value());
    }
    if constexpr (std::is_same_v<AbckitCoreType, AbckitCoreClass>) {
        for (const auto &anno : GetAnnotationNames(&recordField)) {
            field->annotations.emplace_back(CreateAnnotation(file, field.get(), anno));
            field->annotationTable.emplace(anno, CreateAnnotation(file, field.get(), anno));
        }
    }
    return field;
}

std::unique_ptr<AbckitCoreEnumField> EnumCreateField(AbckitFile *file, AbckitCoreEnum *owner,
                                                     ark::pandasm::Field &recordField)
{
    return CreateField<AbckitCoreEnum, AbckitCoreEnumField>(file, owner, recordField);
}

std::unique_ptr<AbckitCoreClassField> ClassCreateField(AbckitFile *file, AbckitCoreClass *owner,
                                                       ark::pandasm::Field &recordField)
{
    return CreateField<AbckitCoreClass, AbckitCoreClassField>(file, owner, recordField);
}

std::unique_ptr<AbckitCoreModuleField> ModuleCreateField(AbckitFile *file, AbckitCoreModule *owner,
                                                         ark::pandasm::Field &recordField)
{
    return CreateField<AbckitCoreModule, AbckitCoreModuleField>(file, owner, recordField);
}

static void CreateModule(AbckitFile *file, const std::string &moduleName, pandasm::Record &record)
{
    auto *container = static_cast<Container *>(file->data);
    auto &nameToModule = container->nameToModule;
    if (nameToModule.find(moduleName) != nameToModule.end()) {
        LIBABCKIT_LOG(FATAL) << "Duplicated ETSGLOBAL for module: " << moduleName << '\n';
    }

    LIBABCKIT_LOG(DEBUG) << "Found module: '" << moduleName << "'\n";
    auto m = std::make_unique<AbckitCoreModule>();
    m->file = file;
    m->target = ABCKIT_TARGET_ARK_TS_V2;
    m->moduleName = CreateNameString(file, moduleName);
    m->impl = std::make_unique<AbckitArktsModule>(&record, m.get());

    nameToModule.insert({moduleName, std::move(m)});
}

static void CreateExternalModule(AbckitFile *file, const std::string &moduleName, pandasm::Record &record)
{
    auto *container = static_cast<Container *>(file->data);
    auto &nameToModule = container->nameToModule;
    auto m = std::make_unique<AbckitCoreModule>();
    m->file = file;
    m->target = ABCKIT_TARGET_ARK_TS_V2;
    m->moduleName = CreateNameString(file, moduleName);
    m->impl = std::make_unique<AbckitArktsModule>(&record, m.get());
    m->GetArkTSImpl()->core = m.get();
    m->isExternal = true;

    nameToModule.emplace(moduleName, std::move(m));
}

static std::unique_ptr<AbckitCoreNamespace> CreateNamespace(pandasm::Record &record, const std::string &namespaceName)
{
    LIBABCKIT_LOG(DEBUG) << "  Found namespace: '" << namespaceName << "'\n";
    return std::make_unique<AbckitCoreNamespace>(nullptr, AbckitArktsNamespace(&record));
}

template <typename AbckitCoreType, typename AbckitArktsType>
static std::unique_ptr<AbckitCoreType> CreateInstance(
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule, const std::string &moduleName,
    const std::string &instanceName, ark::pandasm::Record &record)
{
    LIBABCKIT_LOG(DEBUG) << "  Found instance. module: '" << moduleName << "' instance: '" << instanceName << "'\n";
    ASSERT(nameToModule.find(moduleName) != nameToModule.end());
    const auto &module = nameToModule[moduleName];
    return std::make_unique<AbckitCoreType>(module.get(), AbckitArktsType(&record));
}

static std::string AsyncFunctionGetOriginalName(const std::string &functionName)
{
    auto pos1 = functionName.find(ASYNC_PREFIX);
    auto pos2 = functionName.rfind(OBJECT_CLASS);
    auto size = ASYNC_PREFIX.size();
    return functionName.substr(0, pos1) + functionName.substr(pos1 + size, pos2 - pos1 - size) +
           std::string {ASYNC_ORIGINAL_RETURN};
}

static void AssignAsyncFunction(AbckitFile *file)
{
    const auto container = static_cast<Container *>(file->data);
    for (auto item = container->functions.begin(); item != container->functions.end();) {
        auto name = GetMangleFuncName(item->get());
        if (name.size() <= ASYNC_PREFIX.size() || name.find(ASYNC_PREFIX) == std::string::npos) {
            ++item;
            continue;
        }

        auto originalName = AsyncFunctionGetOriginalName(name);
        auto &nameToFunction = item->get()->GetArkTSImpl()->GetStaticImpl()->IsStatic() ? file->nameToFunctionStatic
                                                                                        : file->nameToFunctionInstance;
        ASSERT(nameToFunction.find(originalName) != nameToFunction.end());
        nameToFunction.at(originalName)->asyncImpl = std::move(*item);
        item = container->functions.erase(item);
    }
}

static void AssignArrayEnum(AbckitFile *file)
{
    const auto container = static_cast<Container *>(file->data);
    for (auto item = container->classes.begin(); item != container->classes.end();) {
        auto name = GetStaticImplRecord(item->get())->name;
        if (!StringUtil::IsEndWith(name, ARRAY_ENUM_SUFFIX.data())) {
            ++item;
            continue;
        }

        auto originEnumName = name.substr(0, name.size() - ARRAY_ENUM_SUFFIX.length());
        auto enumImpl = container->nameToEnum.find(originEnumName);
        if (enumImpl == container->nameToEnum.end()) {
            ++item;
            continue;
        }

        LIBABCKIT_LOG(DEBUG) << "Found array enum:" << name << "\n";
        enumImpl->second->arrayEnum = std::move(*item);
        item = container->classes.erase(item);
    }
}

static void AssignOwningModuleOfNamespace(AbckitFile *file)
{
    const auto container = static_cast<Container *>(file->data);

    for (const auto &ns : container->namespaces) {
        auto fullName = GetStaticImplRecord(ns.get())->name;
        auto moduleName = ClassGetModuleNames(fullName, container->nameToNamespace).first;
        ns->owningModule = container->nameToModule[moduleName].get();
    }
}

static void AssignAnnotationInterfaceToAnnotation(AbckitFile *file)
{
    const auto container = static_cast<Container *>(file->data);

    auto assign = [container](const std::string &annotationName, AbckitCoreAnnotation *annotation) {
        if (container->nameToAnnotationInterface.find(annotationName) != container->nameToAnnotationInterface.end()) {
            auto &ai = container->nameToAnnotationInterface.at(annotationName);
            annotation->ai = ai;
            ai->annotations.emplace_back(annotation);
        }
    };

    for (const auto &klass : container->classes) {
        for (auto &[annotationName, annotation] : klass->annotationTable) {
            assign(annotationName, annotation);
        }
        for (const auto &field : klass->fields) {
            for (const auto &[annotationName, annotation] : field->annotationTable) {
                assign(annotationName, annotation.get());
            }
        }
    }

    for (const auto &iface : container->interfaces) {
        for (const auto &[annotationName, annotation] : iface->annotationTable) {
            assign(annotationName, annotation.get());
        }
    }

    for (const auto &function : container->functions) {
        for (auto &[annotationName, annotation] : function->annotationTable) {
            assign(annotationName, annotation);
        }
    }
}

static void AssignRecordToValues(AbckitFile *file, pandasm::Record *record, const std::vector<std::string> &values)
{
    for (auto &value : values) {
        auto coreType = GetCoreTypeByName(file, value);
        if (!coreType.has_value()) {
            continue;
        }
        std::visit([record](auto &&type) { type->recordUsers.emplace_back(record); }, coreType.value());
    }
}

static void AssignRecordUsers(AbckitFile *file, pandasm::Program *program)
{
    for (auto &[_, record] : program->recordTable) {
        if (record.metadata->IsForeign()) {
            continue;
        }
        for (auto &[_, values] : record.metadata->GetAttributes()) {
            AssignRecordToValues(file, &record, values);
        }
    }
}

static void AssignSuperClass(std::unordered_map<std::string, AbckitCoreClass *> &nameToClass,
                             std::unordered_map<std::string, AbckitCoreInterface *> &nameToInterface,
                             const std::vector<std::unique_ptr<AbckitCoreClass>> &classes)
{
    for (const auto &klass : classes) {
        auto record = GetStaticImplRecord(klass.get());
        std::string fullName = record->name;
        std::string parentClassName = record->metadata->GetBase();
        if (nameToClass.find(parentClassName) != nameToClass.end()) {
            LIBABCKIT_LOG(DEBUG) << "  Found Super Class. Class: '" << fullName << "' Super Class: '" << parentClassName
                                 << "'\n";
            auto &parentClass = nameToClass[parentClassName];
            klass->superClass = parentClass;
            parentClass->subClasses.emplace_back(klass.get());
        }

        for (const auto &interfaceName : record->metadata->GetInterfaces()) {
            if (nameToInterface.find(interfaceName) != nameToInterface.end()) {
                LIBABCKIT_LOG(DEBUG) << "  Found Implemented Interface. Class: '" << fullName
                                     << "' Implemented Interface: '" << interfaceName << "'\n";
                auto &implementedInterface = nameToInterface[interfaceName];
                klass->interfaces.emplace_back(implementedInterface);
                implementedInterface->classes.emplace_back(klass.get());
            }
        }
    }
}

static void AssignSuperInterface(std::unordered_map<std::string, AbckitCoreInterface *> &nameToInterface,
                                 const std::vector<std::unique_ptr<AbckitCoreInterface>> &interfaces)
{
    for (const auto &interface : interfaces) {
        auto record = GetStaticImplRecord(interface.get());
        std::string fullName = record->name;
        for (const auto &superInterfaceName : record->metadata->GetInterfaces()) {
            LIBABCKIT_LOG(DEBUG) << "  Found Super Interface. Interface: '" << fullName << "' Super Interface: '"
                                 << superInterfaceName << "'\n";
            auto &superInterface = nameToInterface[superInterfaceName];
            interface->superInterfaces.emplace_back(superInterface);
            superInterface->subInterfaces.emplace_back(interface.get());
        }
    }
}

static void AssignObjectLiteral(std::vector<std::unique_ptr<AbckitCoreClass>> &objectLiterals,
                                std::unordered_map<std::string, AbckitCoreClass *> &nameToClass,
                                std::unordered_map<std::string, AbckitCoreInterface *> &nameToInterface)
{
    for (auto &objectLiteral : objectLiterals) {
        const auto record = GetStaticImplRecord(objectLiteral.get());
        auto implements = record->metadata->GetAttributeValues(ETS_IMPLEMENTS.data());
        if (!implements.empty()) {
            const auto &interfaceName = implements.front();
            ASSERT(nameToInterface.find(interfaceName) != nameToInterface.end());
            LIBABCKIT_LOG(DEBUG) << "Assign Interface Object Literal: " << record->name << '\n';
            nameToInterface[interfaceName]->objectLiterals.emplace_back(std::move(objectLiteral));
            continue;
        }
        for (const auto &className : record->metadata->GetAttributeValues(ETS_EXTENDS.data())) {
            if (className == OBJECT_CLASS) {
                continue;
            }
            ASSERT(nameToClass.find(className) != nameToClass.end());
            LIBABCKIT_LOG(DEBUG) << "Assign Class Object Literal: " << record->name << '\n';
            nameToClass[className]->objectLiterals.emplace_back(std::move(objectLiteral));
        }
    }
}

template <typename AbckitCoreType>
static void AssignInstance(const std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
                           const std::unordered_map<std::string, AbckitCoreNamespace *> &nameToNamespace,
                           std::vector<std::unique_ptr<AbckitCoreType>> &instances)
{
    for (auto &instance : instances) {
        std::string fullName = GetStaticImplRecord(instance.get())->name;
        auto [moduleName, instanceName] = ClassGetNames(fullName);
        if (nameToNamespace.find(moduleName) != nameToNamespace.end()) {
            LIBABCKIT_LOG(DEBUG) << "namespaceName, className, fullName: " << moduleName << ", " << instanceName << ", "
                                 << fullName << '\n';
            instance->parentNamespace = nameToNamespace.at(moduleName);
            nameToNamespace.at(moduleName)->InsertInstance(instanceName, std::move(instance));
            continue;
        }
        LIBABCKIT_LOG(DEBUG) << "moduleName, className, fullName: " << moduleName << ", " << instanceName << ", "
                             << fullName << '\n';
        auto &instanceModule = nameToModule.at(moduleName);
        instanceModule->InsertInstance(instanceName, std::move(instance));
    }
}

static bool IsInterfaceGetFunction(const std::string &functionName, std::smatch &match)
{
    return std::regex_match(functionName, match, std::regex(INTERFACE_GET_FUNCTION_PATTERN.data()));
}

static bool IsInterfaceSetFunction(const std::string &functionName, std::smatch &match)
{
    return std::regex_match(functionName, match, std::regex(INTERFACE_SET_FUNCTION_PATTERN.data()));
}

static void CreateAndAssignInterfaceField(const std::unique_ptr<AbckitCoreInterface> &interface,
                                          AbckitCoreFunction *function, const std::string &functionName)
{
    std::smatch match;
    if (IsInterfaceGetFunction(functionName, match)) {
        const auto &fieldName = INTERFACE_FIELD_PREFIX.data() + match[1].str();
        LIBABCKIT_LOG(DEBUG) << "Found interface Field: " << fieldName << "\n";
        interface->fields.emplace(fieldName, CreateInterfaceField(interface, function, fieldName));
    } else if (IsInterfaceSetFunction(functionName, match)) {
        const auto &fieldName = INTERFACE_FIELD_PREFIX.data() + match[1].str();
        interface->fields[fieldName]->flag ^= ACC_READONLY;
    }
}

template <typename AbckitCoreType>
using HandlerFunc = std::function<void(AbckitCoreType *, std::unique_ptr<AbckitCoreFunction> &&, const std::string &,
                                       const std::string &)>;

template <typename AbckitCoreType>
struct HandlerInfo {
    // handler to the function if condition is true
    HandlerFunc<AbckitCoreType> handler;
    // Tell if function belong to some vector
    std::function<bool(AbckitCoreType *, const std::string &)> condition;
};

static void AssignAsyncImplParent(const AbckitCoreFunction *function,
                                  const std::variant<AbckitCoreModule *, AbckitCoreNamespace *, AbckitCoreClass *,
                                                     AbckitCoreInterface *, AbckitCoreEnum *> &parent)
{
    if (function->asyncImpl) {
        function->asyncImpl->parent = parent;
    }
}

template <typename AbckitCoreType>
static void HandleGlobalFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                                 const std::string &className, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Module Function. module: '" << className << "' function: '" << functionName
                         << "'\n";
    function->parent = owningInstance;
    AssignAsyncImplParent(function.get(), owningInstance);
    owningInstance->functions.emplace_back(std::move(function));
}

template <typename AbckitCoreType>
static void HandleClassFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                                const std::string &className, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Class Function. class: '" << className << "' function: '" << functionName << "'\n";
    auto &klass = owningInstance->ct[className];
    function->parent = klass.get();
    AssignAsyncImplParent(function.get(), klass.get());
    klass->methods.emplace_back(std::move(function));
}

template <typename AbckitCoreType>
static void HandleNamespaceFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                                    const std::string &namespaceName, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Namespace Function. namespace: '" << namespaceName << "' function: '"
                         << functionName << "'\n";
    auto &ns = owningInstance->nt[namespaceName];
    function->parent = ns.get();
    AssignAsyncImplParent(function.get(), ns.get());
    ns->functions.emplace_back(std::move(function));
}

template <typename AbckitCoreType>
static void HandleInterfaceFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                                    const std::string &interfaceName, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Interface Function. interface: '" << interfaceName << "' function: '"
                         << functionName << "'\n";
    auto &interface = owningInstance->it[interfaceName];
    CreateAndAssignInterfaceField(interface, function.get(), functionName);
    function->parent = interface.get();
    AssignAsyncImplParent(function.get(), interface.get());
    interface->methods.emplace_back(std::move(function));
}

template <typename AbckitCoreType>
static void HandleEnumFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                               const std::string &enumName, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Enum Function. enum: '" << enumName << "' function: '" << functionName << "'\n";
    auto &enm = owningInstance->et[enumName];
    function->parent = enm.get();
    AssignAsyncImplParent(function.get(), enm.get());
    enm->methods.emplace_back(std::move(function));
}

template <typename AbckitCoreType>
static void ProcessFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                            const std::string &className, const std::string &functionName)
{
    std::vector<HandlerInfo<AbckitCoreType>> handlers = {
        {HandleGlobalFunction<AbckitCoreType>,
         [](AbckitCoreType *, const std::string &className) { return IsModuleName(className); }},
        {HandleNamespaceFunction<AbckitCoreType>,
         [](AbckitCoreType *owningInstance, const std::string &className) {
             return owningInstance->nt.find(className) != owningInstance->nt.end();
         }},
        {HandleClassFunction<AbckitCoreType>,
         [](AbckitCoreType *owningInstance, const std::string &className) {
             return owningInstance->ct.find(className) != owningInstance->ct.end();
         }},
        {HandleInterfaceFunction<AbckitCoreType>,
         [](AbckitCoreType *owningInstance, const std::string &className) {
             return owningInstance->it.find(className) != owningInstance->it.end();
         }},
        {HandleEnumFunction<AbckitCoreType>, [](AbckitCoreType *owningInstance, const std::string &className) {
             return owningInstance->et.find(className) != owningInstance->et.end();
         }}};

    for (const auto &handlerInfo : handlers) {
        if (handlerInfo.condition(owningInstance, className)) {
            handlerInfo.handler(owningInstance, std::move(function), className, functionName);
            return;
        }
    }
}

static void AssignFunctions(std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
                            std::unordered_map<std::string, AbckitCoreNamespace *> &nameToNamespace,
                            std::unordered_map<std::string, AbckitCoreClass *> &nameToObjectLiteral,
                            std::vector<std::unique_ptr<AbckitCoreFunction>> &functions)
{
    LIBABCKIT_LOG_FUNC;
    for (auto &function : functions) {
        std::string functionName = FunctionGetImpl(function.get())->name;
        auto [moduleName, className] = FuncGetNames(functionName);
        auto recordName = moduleName;
        recordName.append(".").append(className);
        if (nameToObjectLiteral.find(recordName) != nameToObjectLiteral.end()) {
            auto &objectLiteral = nameToObjectLiteral.at(recordName);
            function->parent = objectLiteral;
            objectLiteral->methods.emplace_back(std::move(function));
            continue;
        }
        if (nameToNamespace.find(moduleName) != nameToNamespace.end()) {
            ProcessFunction(nameToNamespace[moduleName], std::move(function), className, functionName);
            continue;
        }
        auto &functionModule = nameToModule[moduleName];
        ProcessFunction(functionModule.get(), std::move(function), className, functionName);
        ASSERT(function == nullptr);
    }
}

static bool IsNamespace(const std::string &className, pandasm::Record &record)
{
    if (IsModuleName(className)) {
        return false;
    }
    const auto &attributes = GetAnnotationNames(&record);
    return std::any_of(attributes.begin(), attributes.end(),
                       [=](const std::string &name) { return name == ETS_ANNOTATION_MODULE; });
}

static bool IsObjectLiteral(const std::string &name)
{
    return name.find(OBJECT_LITERAL_NAME) != std::string::npos;
}

static bool IsInterface(const pandasm::Record &record)
{
    return (record.metadata->GetAccessFlags() & ACC_INTERFACE) != 0;
}

static bool IsAnnotationInterface(const pandasm::Record &record)
{
    return (record.metadata->GetAccessFlags() & ACC_ANNOTATION) != 0;
}

static const std::string LAMBDA_RECORD_KEY = "%%lambda-";
[[maybe_unused]] static const std::string INIT_FUNC_NAME = "_$init$_";
static const std::string TRIGGER_CCTOR_FUNC_NAME = "_$trigger_cctor$_";

static bool ShouldCreateFuncWrapper(const pandasm::Function &functionImpl, const std::string &functionName)
{
    return (!functionImpl.metadata->IsForeign() &&
            (functionName.find(TRIGGER_CCTOR_FUNC_NAME, 0) == std::string::npos));
}

static void CollectAllFunctions(pandasm::Program *prog, AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;
    const auto container = static_cast<Container *>(file->data);

    std::vector<std::unique_ptr<AbckitCoreFunction>> functions;
    for (auto &[functionName, functionImpl] : prog->functionStaticTable) {
        LIBABCKIT_LOG(DEBUG) << "function function key:" << functionName << std::endl;
        LIBABCKIT_LOG(DEBUG) << "function function value:" << functionImpl.name << std::endl;

        if (ShouldCreateFuncWrapper(functionImpl, functionName)) {
            container->functions.emplace_back(CollectFunction(file, functionName, functionImpl));
        }
    }
    for (auto &[functionName, functionImpl] : prog->functionInstanceTable) {
        LIBABCKIT_LOG(DEBUG) << "function function at instance table:" << functionName << std::endl;

        if (ShouldCreateFuncWrapper(functionImpl, functionName)) {
            container->functions.emplace_back(CollectFunction(file, functionName, functionImpl));
        }
    }
}

static void CollectExternalModules(pandasm::Program *prog, AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;

    const auto container = static_cast<Container *>(file->data);

    for (auto &[recordName, record] : prog->recordTable) {
        if (!record.metadata->IsForeign()) {
            continue;
        }

        auto [moduleName, _] = ClassGetNames(recordName);
        if (container->nameToModule.find(moduleName) != container->nameToModule.end()) {
            continue;
        }

        if (container->nameToNamespace.find(moduleName) != container->nameToNamespace.end()) {
            continue;
        }

        if (prog->recordTable.find(moduleName) != prog->recordTable.end()) {
            LIBABCKIT_LOG(DEBUG) << "Found External Namespace: " << moduleName << '\n';
            container->namespaces.emplace_back(CreateNamespace(prog->recordTable.at(moduleName), moduleName));
            container->nameToNamespace.emplace(moduleName, container->namespaces.back().get());
            continue;
        }

        LIBABCKIT_LOG(DEBUG) << "Found External Module: " << moduleName << '\n';
        CreateExternalModule(file, moduleName, record);
    }
}

static void CollectExternalFunctions(pandasm::Program *prog, AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;
    const auto container = static_cast<Container *>(file->data);

    for (auto &[functionName, functionImpl] : prog->functionStaticTable) {
        if (!functionImpl.metadata->IsForeign()) {
            continue;
        }

        LIBABCKIT_LOG(DEBUG) << "Found External Static Function: " << functionName << std::endl;

        auto [moduleName, className] = FuncGetNames(functionName);
        LIBABCKIT_LOG(DEBUG) << "moduleName: " << moduleName << std::endl;
        LIBABCKIT_LOG(DEBUG) << "className: " << className << std::endl;
        auto function = std::make_unique<AbckitCoreFunction>();

        AbckitCoreModule *resolvedModule = nullptr;
        if (container->nameToModule.find(moduleName) != container->nameToModule.end()) {
            resolvedModule = container->nameToModule.at(moduleName).get();
        } else if (container->nameToNamespace.find(moduleName) != container->nameToNamespace.end()) {
            auto rootModuleName = ClassGetModuleNames(moduleName, container->nameToNamespace).first;
            if (container->nameToModule.find(rootModuleName) != container->nameToModule.end()) {
                resolvedModule = container->nameToModule.at(rootModuleName).get();
            }
        }

        function->owningModule = resolvedModule;

        function->impl = std::make_unique<AbckitArktsFunction>();
        function->GetArkTSImpl()->impl = &functionImpl;
        function->GetArkTSImpl()->core = function.get();

        function->returnType = PandasmTypeToAbckitType(file, functionImpl.returnType);
        AddFunctionUserToAbckitType(function->returnType, function.get());

        for (auto &functionParam : functionImpl.params) {
            function->parameters.emplace_back(CreateFunctionParam(file, function, functionParam));
        }

        for (const auto &anno : GetAnnotationNames(&functionImpl)) {
            function->annotations.emplace_back(CreateAnnotation(file, function.get(), anno));
            function->annotationTable.emplace(anno, function->annotations.back().get());
        }

        auto name = GetMangleFuncName(function.get());
        file->nameToExternalFunction.insert({name, function.get()});
        file->externalFunctions.emplace_back(std::move(function));
    }

    for (auto &[functionName, functionImpl] : prog->functionInstanceTable) {
        if (!functionImpl.metadata->IsForeign()) {
            continue;
        }

        LIBABCKIT_LOG(DEBUG) << "Found External Instance Function: " << functionName << std::endl;

        auto [moduleName, className] = FuncGetNames(functionName);
        LIBABCKIT_LOG(DEBUG) << "moduleName: " << moduleName << std::endl;
        LIBABCKIT_LOG(DEBUG) << "className: " << className << std::endl;
        auto function = std::make_unique<AbckitCoreFunction>();

        AbckitCoreModule *resolvedModule = nullptr;
        if (container->nameToModule.find(moduleName) != container->nameToModule.end()) {
            resolvedModule = container->nameToModule.at(moduleName).get();
        } else if (container->nameToNamespace.find(moduleName) != container->nameToNamespace.end()) {
            auto rootModuleName = ClassGetModuleNames(moduleName, container->nameToNamespace).first;
            if (container->nameToModule.find(rootModuleName) != container->nameToModule.end()) {
                resolvedModule = container->nameToModule.at(rootModuleName).get();
            }
        }

        function->owningModule = resolvedModule;

        function->impl = std::make_unique<AbckitArktsFunction>();
        function->GetArkTSImpl()->impl = &functionImpl;
        function->GetArkTSImpl()->core = function.get();

        function->returnType = PandasmTypeToAbckitType(file, functionImpl.returnType);
        AddFunctionUserToAbckitType(function->returnType, function.get());

        for (auto &functionParam : functionImpl.params) {
            function->parameters.emplace_back(CreateFunctionParam(file, function, functionParam));
        }

        for (const auto &anno : GetAnnotationNames(&functionImpl)) {
            function->annotations.emplace_back(CreateAnnotation(file, function.get(), anno));
            function->annotationTable.emplace(anno, function->annotations.back().get());
        }

        auto name = GetMangleFuncName(function.get());
        file->nameToExternalFunction.insert({name, function.get()});
        file->externalFunctions.emplace_back(std::move(function));
    }
}

static void CollectAnnotations(AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;
    const auto container = static_cast<Container *>(file->data);
    for (const auto &klass : container->classes) {
        for (auto &annotationName : GetAnnotationNames(GetStaticImplRecord(klass.get()))) {
            klass->annotations.emplace_back(CreateAnnotation(file, klass.get(), annotationName));
            klass->annotationTable.emplace(annotationName, klass->annotations.back().get());
        }
    }
    for (const auto &iface : container->interfaces) {
        for (auto &annotationName : GetAnnotationNames(GetStaticImplRecord(iface.get()))) {
            iface->annotationTable.emplace(annotationName, CreateAnnotation(file, iface.get(), annotationName));
        }
    }
}

template <typename AbckitCoreType, typename AbckitCoreTypeField>
static void AssignField(AbckitFile *file, AbckitCoreType *coreInstance)
{
    auto record = GetStaticImplRecord(coreInstance);
    for (auto &recordField : record->fieldList) {
        LIBABCKIT_LOG(DEBUG) << "Found Field: " << recordField.name << "\n";
        auto field = CreateField<AbckitCoreType, AbckitCoreTypeField>(file, coreInstance, recordField);
        file->nameToField.emplace(NameUtil::GetFieldFullName(record, &recordField), field.get());
        coreInstance->fields.emplace_back(std::move(field));
    }
}

static void CollectAllInstanceFields(AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;

    const auto container = static_cast<Container *>(file->data);

    for (const auto &[_, module] : container->nameToModule) {
        AssignField<AbckitCoreModule, AbckitCoreModuleField>(file, module.get());
    }

    for (const auto &[_, ns] : container->nameToNamespace) {
        AssignField<AbckitCoreNamespace, AbckitCoreNamespaceField>(file, ns);
    }

    for (const auto &[_, enm] : container->nameToEnum) {
        AssignField<AbckitCoreEnum, AbckitCoreEnumField>(file, enm);
    }

    for (const auto &[_, ai] : container->nameToAnnotationInterface) {
        AssignField<AbckitCoreAnnotationInterface, AbckitCoreAnnotationInterfaceField>(file, ai);
    }

    for (const auto &[_, objectLiteral] : container->nameToObjectLiteral) {
        AssignField<AbckitCoreClass, AbckitCoreClassField>(file, objectLiteral);
    }

    for (const auto &[_, klass] : container->nameToClass) {
        AssignField<AbckitCoreClass, AbckitCoreClassField>(file, klass);
    }
}

static void CollectAllInstances(AbckitFile *file, pandasm::Program *prog)
{
    LIBABCKIT_LOG_FUNC;

    const auto container = static_cast<Container *>(file->data);

    for (auto &[recordName, record] : prog->recordTable) {
        auto [moduleName, className] = ClassGetModuleNames(recordName, container->nameToNamespace);
        if (IsModuleName(className) ||
            container->nameToNamespace.find(recordName) != container->nameToNamespace.end()) {
            // NOTE: find and fill AbckitCoreImportDescriptor
            continue;
        }

        auto &nameToModule = container->nameToModule;
        // Collect Interface
        if (IsInterface(record)) {
            auto iface =
                CreateInstance<AbckitCoreInterface, AbckitArktsInterface>(nameToModule, moduleName, className, record);
            container->nameToInterface[recordName] = iface.get();
            file->nameToClass.emplace(recordName, iface.get());
            container->interfaces.emplace_back(std::move(iface));
            continue;
        }

        // Collect Enum
        if (IsEnum(record)) {
            auto enm = CreateInstance<AbckitCoreEnum, AbckitArktsEnum>(nameToModule, moduleName, className, record);
            container->nameToEnum.emplace(recordName, enm.get());
            file->nameToClass.emplace(recordName, enm.get());
            container->enums.emplace_back(std::move(enm));
            continue;
        }

        // Collect AnnotationInterface
        if (IsAnnotationInterface(record)) {
            auto ai = CreateInstance<AbckitCoreAnnotationInterface, AbckitArktsAnnotationInterface>(
                nameToModule, moduleName, className, record);
            container->nameToAnnotationInterface.emplace(recordName, ai.get());
            container->annotationInterfaces.emplace_back(std::move(ai));
            continue;
        }

        // Collect Local ObjectLiteral
        if (IsObjectLiteral(recordName) && !record.metadata->IsForeign()) {
            auto objectLiteral =
                CreateInstance<AbckitCoreClass, AbckitArktsClass>(nameToModule, moduleName, className, record);
            container->nameToObjectLiteral.emplace(recordName, objectLiteral.get());
            file->nameToClass.emplace(recordName, objectLiteral.get());
            container->objectLiterals.emplace_back(std::move(objectLiteral));
            continue;
        }

        // Collect Class
        auto klass = CreateInstance<AbckitCoreClass, AbckitArktsClass>(nameToModule, moduleName, className, record);
        container->nameToClass.emplace(recordName, klass.get());
        file->nameToClass.emplace(recordName, klass.get());
        container->classes.emplace_back(std::move(klass));
    }
    CollectAnnotations(file);
    CollectAllInstanceFields(file);
}

static void CollectAllStrings(pandasm::Program *prog, AbckitFile *file)
{
    for (auto &sImpl : prog->strings) {
        auto s = std::make_unique<AbckitString>();
        s->impl = sImpl;
        file->strings.insert({sImpl, std::move(s)});
    }
}

static void CreateWrappers(pandasm::Program *prog, AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;

    file->program = prog;
    auto container = std::make_unique<Container>();
    Container *containerPtr = container.get();
    file->data = container.release();
    file->destoryData = &DestroyContainer;
    // Collect modules and namespaces
    for (auto &[recordName, record] : prog->recordTable) {
        if (record.metadata->IsForeign()) {
            // NOTE: Create AbckitCoreImportDescriptor and AbckitCoreModuleDescriptor
            continue;
        }
        auto [moduleName, className] = ClassGetNames(recordName);
        if (IsModuleName(className)) {
            CreateModule(file, moduleName, record);
            continue;
        }

        if (IsNamespace(className, record)) {
            containerPtr->namespaces.emplace_back(CreateNamespace(record, className));
            containerPtr->nameToNamespace.emplace(recordName, containerPtr->namespaces.back().get());
        }
    }
    CollectExternalModules(prog, file);
    CollectExternalFunctions(prog, file);
    AssignOwningModuleOfNamespace(file);

    // Collect classes, interfaces, enums, interfaceObjectLiteral and annotationInterface
    CollectAllInstances(file, prog);

    // Functions
    CollectAllFunctions(prog, file);

    // Assign
    AssignAsyncFunction(file);
    AssignArrayEnum(file);
    AssignAnnotationInterfaceToAnnotation(file);
    AssignRecordUsers(file, prog);
    AssignSuperClass(containerPtr->nameToClass, containerPtr->nameToInterface, containerPtr->classes);
    AssignSuperInterface(containerPtr->nameToInterface, containerPtr->interfaces);
    AssignObjectLiteral(containerPtr->objectLiterals, containerPtr->nameToClass, containerPtr->nameToInterface);
    AssignInstance(containerPtr->nameToModule, containerPtr->nameToNamespace, containerPtr->classes);
    AssignInstance(containerPtr->nameToModule, containerPtr->nameToNamespace, containerPtr->interfaces);
    AssignInstance(containerPtr->nameToModule, containerPtr->nameToNamespace, containerPtr->annotationInterfaces);
    AssignInstance(containerPtr->nameToModule, containerPtr->nameToNamespace, containerPtr->enums);
    AssignInstance(containerPtr->nameToModule, containerPtr->nameToNamespace, containerPtr->namespaces);
    AssignFunctions(containerPtr->nameToModule, containerPtr->nameToNamespace, containerPtr->nameToObjectLiteral,
                    containerPtr->functions);

    // NOTE: AbckitCoreExportDescriptor
    // NOTE: AbckitModulePayload

    for (auto &[moduleName, module] : containerPtr->nameToModule) {
        if (module->isExternal) {
            file->externalModules.emplace(moduleName, std::move(module));
        } else {
            file->localModules.emplace(moduleName, std::move(module));
        }
    }

    DumpHierarchy(file);

    // Strings
    CollectAllStrings(prog, file);
}

struct CtxIInternal {
    ark::abc2program::Abc2ProgramDriver *driver = nullptr;
};

AbckitFile *OpenAbcStatic(const char *path, size_t len)
{
    LIBABCKIT_LOG_FUNC;
    auto spath = std::string(path, len);
    LIBABCKIT_LOG(DEBUG) << spath << '\n';
    auto *abc2program = new ark::abc2program::Abc2ProgramDriver();
    if (!abc2program->Compile(spath)) {
        LIBABCKIT_LOG(DEBUG) << "Failed to open " << spath << "\n";
        delete abc2program;
        return nullptr;
    }
    pandasm::Program &prog = abc2program->GetProgram();
    auto file = new AbckitFile();
    file->frontend = Mode::STATIC;
    CreateWrappers(&prog, file);

    auto pf = panda_file::File::Open(spath);
    if (pf == nullptr) {
        LIBABCKIT_LOG(DEBUG) << "Failed to panda_file::File::Open\n";
        delete abc2program;
        delete file;
        return nullptr;
    }

    auto pandaFileVersion = pf->GetHeader()->version;
    uint8_t *fileVersion = pandaFileVersion.data();

    file->version = new uint8_t[ABCKIT_VERSION_SIZE];
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::copy(fileVersion, fileVersion + sizeof(uint8_t) * ABCKIT_VERSION_SIZE, file->version);

    file->internal = new CtxIInternal {abc2program};
    return file;
}

void CloseFileStatic(AbckitFile *file)
{
    LIBABCKIT_LOG_FUNC;

    if ((file->destoryData != nullptr) && (file->data != nullptr)) {
        file->destoryData(file->data);
    }
    file->data = nullptr;
    auto *ctxIInternal = reinterpret_cast<struct CtxIInternal *>(file->internal);
    delete ctxIInternal->driver;
    delete ctxIInternal;
    delete[] file->version;
    delete file;
}

void WriteAbcStatic(AbckitFile *file, const char *path, size_t len)
{
    LIBABCKIT_LOG_FUNC;
    auto spath = std::string(path, len);
    LIBABCKIT_LOG(DEBUG) << spath << '\n';

    auto program = file->GetStaticProgram();

    auto emitDebugInfo = false;
    std::map<std::string, size_t> *statp = nullptr;
    ark::pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp = nullptr;

    if (!pandasm::AsmEmitter::Emit(spath, *program, statp, mapsp, emitDebugInfo)) {
        LIBABCKIT_LOG(DEBUG) << "FAILURE: " << pandasm::AsmEmitter::GetLastError() << '\n';
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_INTERNAL_ERROR);
        return;
    }

    LIBABCKIT_LOG(DEBUG) << "SUCCESS\n";
}

void DestroyGraphStatic(AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;
    auto *ctxGInternal = reinterpret_cast<CtxGInternal *>(graph->internal);
    // dirty hack to obtain PandaFile pointer
    // NOTE(mshimenkov): refactor it
    auto *fileWrapper =
        reinterpret_cast<panda_file::File *>(ctxGInternal->runtimeAdapter->GetBinaryFileForMethod(nullptr));
    delete fileWrapper;
    delete ctxGInternal->runtimeAdapter;
    delete ctxGInternal->irInterface;
    delete ctxGInternal->localAllocator;
    delete ctxGInternal->allocator;
    delete ctxGInternal;
    delete graph;
}

}  // namespace libabckit
