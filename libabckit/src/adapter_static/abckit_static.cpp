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

#include "libabckit/src/helpers_common.h"
#include "libabckit/c/metadata_core.h"
#include "libabckit/src/adapter_static/helpers_static.h"
#include "libabckit/c/ir_core.h"
#include "libabckit/src/statuses_impl.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/adapter_static/runtime_adapter_static.h"
#include "libabckit/src/logger.h"
#include "libarkfile/file.h"

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
constexpr std::string_view ETS_IMPLEMENTS = "ets.implements";
constexpr std::string_view STD_ANNOTATION_INTERFACE_OBJECT_LITERAL = "std.annotations.InterfaceObjectLiteral";
constexpr std::string_view ENUM_BASE = "std.core.BaseEnum";
constexpr std::string_view INTERFACE_GET_FUNCTION_PATTERN = ".*<get>(.*)";
constexpr std::string_view INTERFACE_SET_FUNCTION_PATTERN = ".*<set>(.*)";
constexpr std::string_view INTERFACE_FIELD_PREFIX = "<property>";
const std::unordered_set<std::string> GLOBAL_CLASS_NAMES = {"ETSGLOBAL", "_GLOBAL"};
}  // namespace

namespace libabckit {
// Container of vectors and maps of some kinds of instances
// Including modules, classes, namespaces, interfaces, enums and functions
struct Container {
    std::vector<std::unique_ptr<AbckitCoreClass>> classes;
    std::vector<std::unique_ptr<AbckitCoreInterface>> interfaces;
    std::vector<std::unique_ptr<AbckitCoreNamespace>> namespaces;
    std::vector<std::unique_ptr<AbckitCoreEnum>> enums;
    std::vector<std::unique_ptr<AbckitCoreFunction>> functions;
    std::vector<std::unique_ptr<AbckitCoreClass>> interfaceObjectLiterals;
    std::vector<std::unique_ptr<AbckitCoreAnnotationInterface>> annotationInterfaces;
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> nameToModule;
    std::unordered_map<std::string, AbckitCoreNamespace *> nameToNamespace;
    std::unordered_map<std::string, AbckitCoreClass *> nameToClass;
    std::unordered_map<std::string, AbckitCoreInterface *> nameToInterface;
    std::unordered_map<std::string, AbckitCoreAnnotationInterface *> nameToAnnotationInterface;
};

static bool IsAbstract(pandasm::ItemMetadata *meta)
{
    uint32_t flags = meta->GetAccessFlags();
    return (flags & ACC_ABSTRACT) != 0U;
}

static bool IsModuleName(const std::string &name)
{
    return GLOBAL_CLASS_NAMES.find(name) != GLOBAL_CLASS_NAMES.end();
}

AbckitString *CreateNameString(AbckitFile *file, const std::string &name)
{
    return CreateStringStatic(file, name.data(), name.size());
}

pandasm::Function *FunctionGetImpl(AbckitCoreFunction *function)
{
    return function->GetArkTSImpl()->GetStaticImpl();
}

static AbckitType *PandasmTypeToAbckitType(AbckitFile *file, const pandasm::Type &pandasmType)
{
    auto abckitName = CreateNameString(file, pandasmType.GetName());
    auto typeId = ArkPandasmTypeToAbckitTypeId(pandasmType);
    auto rank = pandasmType.GetRank();
    auto type = GetOrCreateType(file, typeId, rank, nullptr, abckitName);

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
        auto cName = c->GetArkTSImpl()->impl.GetStaticClass()->name;
        LIBABCKIT_LOG_NO_FUNC(DEBUG) << indent << cName << std::endl;
        for (auto &f : c->methods) {
            dumpFunc(f.get(), indent + "  ");
        }
    };

    std::function<void(AbckitCoreNamespace * n, const std::string &indent)> dumpNamespace =
        [&dumpFunc, &dumpClass, &dumpNamespace](AbckitCoreNamespace *n, const std::string &indent = "") {
            ASSERT(n->owningModule->target == ABCKIT_TARGET_ARK_TS_V1);
            auto &nName = FunctionGetImpl(n->GetArkTSImpl()->f.get())->name;
            LIBABCKIT_LOG_NO_FUNC(DEBUG) << indent << nName << std::endl;
            for (auto &n : n->namespaces) {
                dumpNamespace(n.get(), indent + "  ");
            }
            for (auto &c : n->classes) {
                dumpClass(c.get(), indent + "  ");
            }
            for (auto &f : n->functions) {
                dumpFunc(f.get(), indent + "  ");
            }
        };

    for (auto &[mName, m] : file->localModules) {
        LIBABCKIT_LOG_NO_FUNC(DEBUG) << mName << std::endl;
        for (auto &n : m->namespaces) {
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
    std::variant<AbckitCoreClass *, AbckitCoreFunction *, AbckitCoreClassField *, AbckitCoreInterface *,
                 AbckitCoreInterfaceField *>
        owner,
    const std::string &name)
{
    LIBABCKIT_LOG(DEBUG) << "Found annotation :'" << name << "'\n";
    auto [_, annotationName] = ClassGetNames(name);
    auto anno = std::make_unique<AbckitCoreAnnotation>();
    anno->name = CreateNameString(file, annotationName);
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

    return param;
}

std::unique_ptr<AbckitCoreFunction> CollectFunction(
    AbckitFile *file, std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
    std::unordered_map<std::string, AbckitCoreNamespace *> &nameToNamespace, const std::string &functionName,
    ark::pandasm::Function &functionImpl)
{
    auto [moduleName, className] = FuncGetNames(functionName);
    auto function = std::make_unique<AbckitCoreFunction>();
    if (nameToNamespace.find(moduleName) != nameToNamespace.end()) {
        function->parentNamespace = nameToNamespace[moduleName];
        moduleName = ClassGetModuleNames(moduleName, nameToNamespace).first;
    }
    LIBABCKIT_LOG(DEBUG) << "  Found function. module: '" << moduleName << "' class: '" << className << "' function: '"
                         << functionName << "'\n";
    ASSERT(nameToModule.find(moduleName) != nameToModule.end());
    auto &functionModule = nameToModule[moduleName];
    function->owningModule = functionModule.get();
    function->impl = std::make_unique<AbckitArktsFunction>();
    function->GetArkTSImpl()->impl = &functionImpl;
    function->GetArkTSImpl()->core = function.get();
    auto name = GetMangleFuncName(function.get());
    auto &nameToFunction = functionImpl.IsStatic() ? file->nameToFunctionStatic : file->nameToFunctionInstance;
    ASSERT(nameToFunction.count(name) == 0);
    nameToFunction.insert({name, function.get()});

    for (const auto &anno : functionImpl.metadata->GetAttributeValues(ETS_ANNOTATION_CLASS.data())) {
        function->annotations.emplace_back(CreateAnnotation(file, function.get(), anno));
        function->annotationTable.emplace(anno, function->annotations.back().get());
    }

    for (auto &functionParam : functionImpl.params) {
        function->parameters.emplace_back(CreateFunctionParam(file, function, functionParam));
    }

    function->returnType = PandasmTypeToAbckitType(function->owningModule->file, functionImpl.returnType);

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
    field->type = PandasmTypeToAbckitType(file, returnType);

    for (const auto &[name, anno] : function->annotationTable) {
        auto annotation = CreateAnnotation(file, field.get(), name);
        annotation->ai = anno->ai;
        field->annotationTable.emplace(name, std::move(annotation));
    }
    return field;
}

template <typename AbckitCoreType, typename AbckitCoreTypeField>
static std::unique_ptr<AbckitCoreTypeField> CreateField(AbckitFile *file, AbckitCoreType *owner,
                                                        pandasm::Field &recordField)
{
    auto field = std::make_unique<AbckitCoreTypeField>(owner, &recordField);
    field->name = CreateNameString(file, recordField.name);
    field->type = PandasmTypeToAbckitType(file, recordField.type);

    if constexpr (std::is_same_v<AbckitCoreType, AbckitCoreClass>) {
        for (const auto &anno : recordField.metadata->GetAttributeValues(ETS_ANNOTATION_CLASS.data())) {
            field->annotationTable.emplace(anno, CreateAnnotation(file, field.get(), anno));
        }
    }
    return field;
}

static void CreateModule(AbckitFile *file,
                         std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
                         const std::string &recordName, pandasm::Record &record)
{
    auto [moduleName, className] = ClassGetNames(recordName);
    if (IsModuleName(className)) {
        if (nameToModule.find(moduleName) != nameToModule.end()) {
            LIBABCKIT_LOG(FATAL) << "Duplicated ETSGLOBAL for module: " << moduleName << '\n';
        }

        LIBABCKIT_LOG(DEBUG) << "Found module: '" << moduleName << "'\n";
        auto m = std::make_unique<AbckitCoreModule>();
        m->file = file;
        m->target = ABCKIT_TARGET_ARK_TS_V2;
        m->moduleName = CreateStringStatic(file, moduleName.data(), moduleName.size());
        m->impl = std::make_unique<AbckitArktsModule>();
        m->GetArkTSImpl()->core = m.get();

        for (auto &recordField : record.fieldList) {
            LIBABCKIT_LOG(DEBUG) << "Found Module Field: " << recordField.name << "\n";
            m->fields.emplace_back(CreateField<AbckitCoreModule, AbckitCoreModuleField>(file, m.get(), recordField));
        }
        nameToModule.insert({moduleName, std::move(m)});
    }
}

static void CreateExternalModule(AbckitFile *file,
                                 std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
                                 const std::string &moduleName, pandasm::Record &record)
{
    auto m = std::make_unique<AbckitCoreModule>();
    m->file = file;
    m->target = ABCKIT_TARGET_ARK_TS_V2;
    m->moduleName = CreateNameString(file, moduleName);
    m->impl = std::make_unique<AbckitArktsModule>();
    m->GetArkTSImpl()->core = m.get();
    m->isExternal = true;
    for (auto &recordField : record.fieldList) {
        LIBABCKIT_LOG(DEBUG) << "Found Module Field: " << recordField.name << "\n";
        m->fields.emplace_back(CreateField<AbckitCoreModule, AbckitCoreModuleField>(file, m.get(), recordField));
    }

    nameToModule.emplace(moduleName, std::move(m));
}

static std::unique_ptr<AbckitCoreNamespace> CreateNamespace(AbckitFile *file, pandasm::Record &record,
                                                            const std::string &namespaceName)
{
    LIBABCKIT_LOG(DEBUG) << "  Found namespace: '" << namespaceName << "'\n";
    auto ns = std::make_unique<AbckitCoreNamespace>(nullptr, AbckitArktsNamespace(&record));
    for (auto &recordField : record.fieldList) {
        LIBABCKIT_LOG(DEBUG) << "Found Namespace Field: " << recordField.name << "\n";
        auto field = CreateField<AbckitCoreNamespace, AbckitCoreNamespaceField>(file, ns.get(), recordField);
        ns->fields.emplace_back(std::move(field));
    }
    return ns;
}

template <typename AbckitCoreType, typename AbckitArktsType, typename AbckitCoreTypeField>
static std::unique_ptr<AbckitCoreType> CreateInstance(
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule, const std::string &moduleName,
    const std::string &instanceName, ark::pandasm::Record &record)
{
    LIBABCKIT_LOG(DEBUG) << "  Found instance. module: '" << moduleName << "' instance: '" << instanceName << "'\n";
    ASSERT(nameToModule.find(moduleName) != nameToModule.end());
    auto &instanceModule = nameToModule[moduleName];
    auto abckitRecord = &record;
    auto instance = std::make_unique<AbckitCoreType>(instanceModule.get(), AbckitArktsType(abckitRecord));

    if constexpr (!std::is_same_v<AbckitCoreType, AbckitCoreInterface>) {
        for (auto &recordField : record.fieldList) {
            LIBABCKIT_LOG(DEBUG) << "Found instance Field: " << recordField.name << "\n";
            auto field =
                CreateField<AbckitCoreType, AbckitCoreTypeField>(instanceModule->file, instance.get(), recordField);
            instance->fields.emplace_back(std::move(field));
        }
    }
    return instance;
}

static void AssignOwningModuleOfNamespace(
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
    const std::unordered_map<std::string, AbckitCoreNamespace *> &nameToNamespace,
    const std::vector<std::unique_ptr<AbckitCoreNamespace>> &namespaces)
{
    for (const auto &ns : namespaces) {
        auto fullName = ns->GetArkTSImpl()->impl.GetStaticClass()->name;
        auto moduleName = ClassGetModuleNames(fullName, nameToNamespace).first;
        ns->owningModule = nameToModule[moduleName].get();
    }
}

static void AssignAnnotationInterfaceToAnnotation(Container &container)
{
    auto assign = [&](const std::string &annotationName, AbckitCoreAnnotation *annotation) {
        if (container.nameToAnnotationInterface.find(annotationName) != container.nameToAnnotationInterface.end()) {
            auto &ai = container.nameToAnnotationInterface.at(annotationName);
            annotation->ai = ai;
        }
    };

    for (const auto &klass : container.classes) {
        for (auto &[annotationName, annotation] : klass->annotationTable) {
            assign(annotationName, annotation);
        }
        for (const auto &field : klass->fields) {
            for (const auto &[annotationName, annotation] : field->annotationTable) {
                assign(annotationName, annotation.get());
            }
        }
    }

    for (const auto &iface : container.interfaces) {
        for (const auto &[annotationName, annotation] : iface->annotationTable) {
            assign(annotationName, annotation.get());
        }
    }

    for (const auto &function : container.functions) {
        if (!function) {
            continue;
        }
        for (auto &[annotationName, annotation] : function->annotationTable) {
            assign(annotationName, annotation);
        }
    }
}

static void AssignSuperClass(std::unordered_map<std::string, AbckitCoreClass *> &nameToClass,
                             std::unordered_map<std::string, AbckitCoreInterface *> &nameToInterface,
                             const std::vector<std::unique_ptr<AbckitCoreClass>> &classes)
{
    for (const auto &klass : classes) {
        std::string fullName = klass->GetArkTSImpl()->impl.GetStaticClass()->name;
        std::string parentClassName = klass->GetArkTSImpl()->impl.GetStaticClass()->metadata->GetBase();
        if (nameToClass.find(parentClassName) != nameToClass.end()) {
            LIBABCKIT_LOG(DEBUG) << "  Found Super Class. Class: '" << fullName << "' Super Class: '" << parentClassName
                                 << "'\n";
            auto &parentClass = nameToClass[parentClassName];
            klass->superClass = parentClass;
            parentClass->subClasses.emplace_back(klass.get());
        }

        for (const auto &interfaceName : klass->GetArkTSImpl()->impl.GetStaticClass()->metadata->GetInterfaces()) {
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

static void AssignSuperInerface(std::unordered_map<std::string, AbckitCoreInterface *> &nameToInterface,
                                const std::vector<std::unique_ptr<AbckitCoreInterface>> &interfaces)
{
    for (const auto &interface : interfaces) {
        std::string fullName = interface->GetArkTSImpl()->impl.GetStaticClass()->name;
        for (const auto &superInterfaceName :
             interface->GetArkTSImpl()->impl.GetStaticClass()->metadata->GetInterfaces()) {
            LIBABCKIT_LOG(DEBUG) << "  Found Super Interface. Interface: '" << fullName << "' Super Interface: '"
                                 << superInterfaceName << "'\n";
            auto &superInterface = nameToInterface[superInterfaceName];
            interface->superInterfaces.emplace_back(superInterface);
            superInterface->subInterfaces.emplace_back(interface.get());
        }
    }
}

static void AssignInterfaceObjectLiteral(std::vector<std::unique_ptr<AbckitCoreClass>> &interfaceObjectLiterals,
                                         std::unordered_map<std::string, AbckitCoreInterface *> &nameToInterface)
{
    for (auto &objectLiteral : interfaceObjectLiterals) {
        auto record = objectLiteral->GetArkTSImpl()->impl.GetStaticClass();
        for (const auto &interfaceName : record->metadata->GetAttributeValues(ETS_IMPLEMENTS.data())) {
            if (nameToInterface.find(interfaceName) != nameToInterface.end()) {
                LIBABCKIT_LOG(DEBUG) << "Found Interface ObjectLiteral. Interface: '" << interfaceName
                                     << "'  ObjectLiteral: '" << record->name << "'\n";
                nameToInterface[interfaceName]->objectLiterals.emplace_back(std::move(objectLiteral));
            }
        }
    }
}

template <typename AbckitCoreType>
static void AssignInstance(const std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
                           const std::unordered_map<std::string, AbckitCoreNamespace *> &nameToNamespace,
                           std::vector<std::unique_ptr<AbckitCoreType>> &instances)
{
    for (auto &instance : instances) {
        std::string fullName;
        if constexpr (std::is_same_v<AbckitCoreType, AbckitCoreAnnotationInterface>) {
            fullName = instance->GetArkTSImpl()->GetStaticImpl()->name;
        } else {
            fullName = instance->GetArkTSImpl()->impl.GetStaticClass()->name;
        }
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

static void CreateAndAssignInterfaceField(std::unique_ptr<AbckitCoreInterface> &interface, AbckitCoreFunction *function,
                                          const std::string &functionName)
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

template <typename AbckitCoreType>
static void HandleGlobalFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                                 const std::string &className, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Module Function. module: '" << className << "' function: '" << functionName
                         << "'\n";
    owningInstance->functions.emplace_back(std::move(function));
}

template <typename AbckitCoreType>
static void HandleClassFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                                const std::string &className, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Class Function. class: '" << className << "' function: '" << functionName << "'\n";
    auto &klass = owningInstance->ct[className];
    function->parentClass = klass.get();
    klass->methods.emplace_back(std::move(function));
}

template <typename AbckitCoreType>
static void HandleNamespaceFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                                    const std::string &namespaceName, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Namespace Function. namespace: '" << namespaceName << "' function: '"
                         << functionName << "'\n";
    owningInstance->nt[namespaceName]->functions.emplace_back(std::move(function));
}

template <typename AbckitCoreType>
static void HandleInterfaceFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                                    const std::string &interfaceName, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Interface Function. interface: '" << interfaceName << "' function: '"
                         << functionName << "'\n";
    auto &interface = owningInstance->it[interfaceName];
    CreateAndAssignInterfaceField(interface, function.get(), functionName);
    interface->methods.emplace_back(std::move(function));
}

template <typename AbckitCoreType>
static void HandleEnumFunction(AbckitCoreType *owningInstance, std::unique_ptr<AbckitCoreFunction> &&function,
                               const std::string &enumName, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Enum Function. enum: '" << enumName << "' function: '" << functionName << "'\n";
    owningInstance->et[enumName]->methods.emplace_back(std::move(function));
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
                            std::vector<std::unique_ptr<AbckitCoreFunction>> &functions)
{
    for (auto &function : functions) {
        std::string functionName = FunctionGetImpl(function.get())->name;
        auto [moduleName, className] = FuncGetNames(functionName);
        if (nameToNamespace.find(moduleName) != nameToNamespace.end()) {
            ProcessFunction(nameToNamespace[moduleName], std::move(function), className, functionName);
            continue;
        }
        auto &functionModule = nameToModule[moduleName];
        ProcessFunction(functionModule.get(), std::move(function), className, functionName);
    }
}

static bool HasAnnotation(const pandasm::Record &record, const std::string_view &annotation)
{
    const auto &attributes = record.metadata->GetAttributeValues(ETS_ANNOTATION_CLASS.data());
    return std::any_of(attributes.begin(), attributes.end(),
                       [=](const std::string &name) { return name == annotation; });
}

static bool IsNamespace(const std::string &className, const pandasm::Record &record)
{
    if (IsModuleName(className)) {
        return false;
    }
    return HasAnnotation(record, ETS_ANNOTATION_MODULE);
}

static bool IsInterfaceObjectLiteral(const pandasm::Record &record)
{
    return HasAnnotation(record, STD_ANNOTATION_INTERFACE_OBJECT_LITERAL);
}

static bool IsInterface(const pandasm::Record &record)
{
    return (record.metadata->GetAccessFlags() & ACC_INTERFACE) != 0;
}

static bool IsEnum(const pandasm::Record &record)
{
    return record.metadata->GetBase() == ENUM_BASE;
}

static bool IsAnnotationInterface(const pandasm::Record &record)
{
    return (record.metadata->GetAccessFlags() & ACC_ANNOTATION) != 0;
}

static bool IsInterfaceFunction(const std::string &instanceName,
                                const std::unordered_map<std::string, AbckitCoreInterface *> &nameToInterface)
{
    for (const auto &[recordName, _] : nameToInterface) {
        auto [moduleName, interfaceName] = ClassGetNames(recordName);
        if (interfaceName == instanceName) {
            return true;
        }
    }
    return false;
}

static const std::string LAMBDA_RECORD_KEY = "%%lambda-";
static const std::string INIT_FUNC_NAME = "_$init$_";
static const std::string TRIGGER_CCTOR_FUNC_NAME = "_$trigger_cctor$_";

static bool ShouldCreateFuncWrapper(pandasm::Function &functionImpl, const std::string &className,
                                    const std::string &functionName,
                                    const std::unordered_map<std::string, AbckitCoreInterface *> &nameToInterface)
{
    if (functionImpl.metadata->IsForeign() || (className.substr(0, LAMBDA_RECORD_KEY.size()) == LAMBDA_RECORD_KEY) ||
        (functionName.find(INIT_FUNC_NAME, 0) != std::string::npos) ||
        (functionName.find(TRIGGER_CCTOR_FUNC_NAME, 0) != std::string::npos)) {
        // NOTE: find and fill AbckitCoreImportDescriptor
        return false;
    }

    if (IsAbstract(functionImpl.metadata.get())) {
        return IsInterfaceFunction(className, nameToInterface);
    }

    return true;
}

static std::vector<std::unique_ptr<AbckitCoreFunction>> CollectAllFunctions(
    pandasm::Program *prog, AbckitFile *file,
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
    std::unordered_map<std::string, AbckitCoreNamespace *> &nameToNamespace,
    const std::unordered_map<std::string, AbckitCoreInterface *> &nameToInterface)
{
    std::vector<std::unique_ptr<AbckitCoreFunction>> functions;
    for (auto &[functionName, functionImpl] : prog->functionStaticTable) {
        auto [moduleName, className] = FuncGetNames(functionName);

        if (ShouldCreateFuncWrapper(functionImpl, className, functionName, nameToInterface)) {
            functions.emplace_back(CollectFunction(file, nameToModule, nameToNamespace, functionName, functionImpl));
        }
    }
    for (auto &[functionName, functionImpl] : prog->functionInstanceTable) {
        auto [moduleName, className] = FuncGetNames(functionName);

        if (ShouldCreateFuncWrapper(functionImpl, className, functionName, nameToInterface)) {
            functions.emplace_back(CollectFunction(file, nameToModule, nameToNamespace, functionName, functionImpl));
        }
    }
    return functions;
}

static void CollectExternalModules(pandasm::Program *prog, AbckitFile *file, Container &container)
{
    for (auto &[recordName, record] : prog->recordTable) {
        if (!record.metadata->IsForeign()) {
            continue;
        }

        auto [moduleName, _] = ClassGetNames(recordName);
        if (container.nameToModule.find(moduleName) != container.nameToModule.end()) {
            continue;
        }

        if (container.nameToNamespace.find(moduleName) != container.nameToNamespace.end()) {
            continue;
        }

        LIBABCKIT_LOG(DEBUG) << "Record:" << recordName << '\n';
        if (prog->recordTable.find(moduleName) != prog->recordTable.end()) {
            LIBABCKIT_LOG(DEBUG) << "Found External Namespace: " << moduleName << '\n';
            container.namespaces.emplace_back(CreateNamespace(file, prog->recordTable.at(moduleName), moduleName));
            container.nameToNamespace.emplace(moduleName, container.namespaces.back().get());
            continue;
        }

        LIBABCKIT_LOG(DEBUG) << "Found External Module: " << moduleName << '\n';
        CreateExternalModule(file, container.nameToModule, moduleName, record);
    }
}

static void CollectAllInstances(AbckitFile *file, pandasm::Program *prog, Container &container)
{
    for (auto &[recordName, record] : prog->recordTable) {
        if (recordName.find(LAMBDA_RECORD_KEY) != std::string::npos) {
            // NOTE: find and fill AbckitCoreImportDescriptor
            continue;
        }

        auto [moduleName, className] = ClassGetModuleNames(recordName, container.nameToNamespace);
        if (IsModuleName(className) || container.nameToNamespace.find(recordName) != container.nameToNamespace.end()) {
            continue;
        }

        auto &nameToModule = container.nameToModule;
        // Collect Interface
        if (IsInterface(record)) {
            auto iface = CreateInstance<AbckitCoreInterface, AbckitArktsInterface, AbckitCoreInterfaceField>(
                nameToModule, moduleName, className, record);
            container.nameToInterface[recordName] = iface.get();
            for (auto &annotationName : record.metadata->GetAttributeValues(ETS_ANNOTATION_CLASS.data())) {
                iface->annotationTable.emplace(annotationName, CreateAnnotation(file, iface.get(), annotationName));
            }
            container.interfaces.emplace_back(std::move(iface));
            continue;
        }
        // Collect Enum
        if (IsEnum(record)) {
            container.enums.emplace_back(CreateInstance<AbckitCoreEnum, AbckitArktsEnum, AbckitCoreEnumField>(
                nameToModule, moduleName, className, record));
            continue;
        }
        // Collect AnnotationInterface
        if (IsAnnotationInterface(record)) {
            auto ai = CreateInstance<AbckitCoreAnnotationInterface, AbckitArktsAnnotationInterface,
                                     AbckitCoreAnnotationInterfaceField>(nameToModule, moduleName, className, record);
            container.nameToAnnotationInterface.emplace(recordName, ai.get());
            container.annotationInterfaces.emplace_back(std::move(ai));
            continue;
        }
        // Collect InterfaceObjectLiteral
        if (IsInterfaceObjectLiteral(record)) {
            container.interfaceObjectLiterals.emplace_back(
                CreateInstance<AbckitCoreClass, AbckitArktsClass, AbckitCoreClassField>(nameToModule, moduleName,
                                                                                        className, record));
            continue;
        }
        // Collect Class
        auto klass = CreateInstance<AbckitCoreClass, AbckitArktsClass, AbckitCoreClassField>(nameToModule, moduleName,
                                                                                             className, record);
        for (auto &annotationName : record.metadata->GetAttributeValues(ETS_ANNOTATION_CLASS.data())) {
            klass->annotations.emplace_back(CreateAnnotation(file, klass.get(), annotationName));
            klass->annotationTable.emplace(annotationName, klass->annotations.back().get());
        }
        container.classes.emplace_back(std::move(klass));
        container.nameToClass[recordName] = container.classes.back().get();
    }
}

static void CreateWrappers(pandasm::Program *prog, AbckitFile *file)
{
    file->program = prog;
    Container container;
    // Collect modules and namespaces
    for (auto &[recordName, record] : prog->recordTable) {
        if (record.metadata->IsForeign()) {
            // NOTE: Create AbckitCoreImportDescriptor and AbckitCoreModuleDescriptor
            continue;
        }
        CreateModule(file, container.nameToModule, recordName, record);

        auto [_, namespaceName] = ClassGetNames(recordName);
        if (IsModuleName(namespaceName)) {
            continue;
        }
        if (IsNamespace(namespaceName, record)) {
            container.namespaces.emplace_back(CreateNamespace(file, record, namespaceName));
            container.nameToNamespace.emplace(recordName, container.namespaces.back().get());
        }
    }
    CollectExternalModules(prog, file, container);
    auto &nameToModule = container.nameToModule;
    auto &nameToNamespace = container.nameToNamespace;
    AssignOwningModuleOfNamespace(nameToModule, nameToNamespace, container.namespaces);

    // Collect classes, interfaces and enums
    CollectAllInstances(file, prog, container);

    // Functions
    container.functions = CollectAllFunctions(prog, file, nameToModule, nameToNamespace, container.nameToInterface);

    AssignAnnotationInterfaceToAnnotation(container);
    AssignSuperClass(container.nameToClass, container.nameToInterface, container.classes);
    AssignSuperInerface(container.nameToInterface, container.interfaces);
    AssignInterfaceObjectLiteral(container.interfaceObjectLiterals, container.nameToInterface);
    AssignInstance(nameToModule, nameToNamespace, container.classes);
    AssignInstance(nameToModule, nameToNamespace, container.interfaces);
    AssignInstance(nameToModule, nameToNamespace, container.annotationInterfaces);
    AssignInstance(nameToModule, nameToNamespace, container.enums);
    AssignInstance(nameToModule, nameToNamespace, container.namespaces);
    AssignFunctions(nameToModule, nameToNamespace, container.functions);

    // NOTE: AbckitCoreExportDescriptor
    // NOTE: AbckitModulePayload

    for (auto &[moduleName, module] : nameToModule) {
        if (!module->isExternal) {
            file->localModules.emplace(moduleName, std::move(module));
        } else {
            file->externalModules.emplace(moduleName, std::move(module));
        }
    }

    DumpHierarchy(file);

    // Strings
    for (auto &sImpl : prog->strings) {
        auto s = std::make_unique<AbckitString>();
        s->impl = sImpl;
        file->strings.insert({sImpl, std::move(s)});
    }
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
