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

#include "../metadata_inspect_impl.h"
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
constexpr std::string_view ENUM_BASE = "std.core.BaseEnum";
constexpr std::string_view INTERFACE_GET_FUNCTION_PATTERN = ".*<get>(.*)";
constexpr std::string_view INTERFACE_SET_FUNCTION_PATTERN = ".*<set>(.*)";
const std::unordered_set<std::string> GLOBAL_CLASS_NAMES = {"ETSGLOBAL", "_GLOBAL"};
}  // namespace

namespace libabckit {

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

std::unique_ptr<AbckitCoreFunction> CollectFunction(
    AbckitFile *file, std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
    const std::string &functionName, ark::pandasm::Function &functionImpl)
{
    auto [moduleName, className] = FuncGetNames(functionName);
    LIBABCKIT_LOG(DEBUG) << "  Found function. module: '" << moduleName << "' class: '" << className << "' function: '"
                         << functionName << "'\n";
    ASSERT(nameToModule.find(moduleName) != nameToModule.end());
    auto &functionModule = nameToModule[moduleName];
    auto function = std::make_unique<AbckitCoreFunction>();
    function->owningModule = functionModule.get();
    function->impl = std::make_unique<AbckitArktsFunction>();
    function->GetArkTSImpl()->impl = &functionImpl;
    function->GetArkTSImpl()->core = function.get();
    auto name = GetMangleFuncName(function.get());
    auto &nameToFunction = functionImpl.IsStatic() ? file->nameToFunctionStatic : file->nameToFunctionInstance;
    ASSERT(nameToFunction.count(name) == 0);
    nameToFunction.insert({name, function.get()});

    for (auto &annoImpl : functionImpl.metadata->GetAnnotations()) {
        auto anno = std::make_unique<AbckitCoreAnnotation>();
        anno->impl = std::make_unique<AbckitArktsAnnotation>();
        anno->GetArkTSImpl()->core = anno.get();

        for (auto &annoElemImpl : annoImpl.GetElements()) {
            auto annoElem = std::make_unique<AbckitCoreAnnotationElement>();
            annoElem->ann = anno.get();
            auto annoElemImplName = annoElemImpl.GetName();
            annoElem->name = CreateStringStatic(file, annoElemImplName.data(), annoElemImplName.size());
            annoElem->impl = std::make_unique<AbckitArktsAnnotationElement>();
            annoElem->GetArkTSImpl()->core = annoElem.get();
            auto value = FindOrCreateValueStatic(file, *annoElemImpl.GetValue());
            annoElem->value = value;

            anno->elements.emplace_back(std::move(annoElem));
        }
        function->annotations.emplace_back(std::move(anno));
    }

    return function;
}

static std::unique_ptr<AbckitCoreModuleField> CreateModuleField(AbckitCoreModule *owningModule,
                                                                const pandasm::Field *recordField)
{
    auto field = std::make_unique<AbckitCoreModuleField>();
    field->owner = owningModule;
    field->name = CreateStringStatic(owningModule->file, recordField->name.data(), recordField->name.size());
    return field;
}

static std::unique_ptr<AbckitCoreInterfaceField> CreateInterfaceField(
    const std::unique_ptr<AbckitCoreInterface> &interface, const std::string &fieldName)
{
    auto field = std::make_unique<AbckitCoreInterfaceField>();
    field->name = CreateNameString(interface->owningModule->file, fieldName);
    field->impl = std::make_unique<AbckitArktsInterfaceField>();
    field->GetArkTSImpl()->core = field.get();
    field->owner = interface.get();
    field->flag = (ACC_PUBLIC | ACC_READONLY);
    return field;
}

template <typename AbckitCoreType, typename AbckitCoreTypeField, typename AbckitArktsTypeField>
static std::unique_ptr<AbckitCoreTypeField> CreateField(AbckitCoreModule *owningModule, AbckitCoreType *owner,
                                                        const pandasm::Field &recordField)
{
    auto field = std::make_unique<AbckitCoreTypeField>();
    field->name = CreateNameString(owningModule->file, recordField.name);
    field->owner = owner;
    field->impl = std::make_unique<AbckitArktsTypeField>();
    field->GetArkTSImpl()->core = field.get();
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

        for (const auto &recordField : record.fieldList) {
            LIBABCKIT_LOG(DEBUG) << "Found Module Field: " << recordField.name << "\n";
            auto field = CreateModuleField(m.get(), &recordField);
            m->fields.emplace_back(std::move(field));
        }
        nameToModule.insert({moduleName, std::move(m)});
    }
}

static std::unique_ptr<AbckitCoreNamespace> CreateNamespace(pandasm::Record &record, const std::string &namespaceName)
{
    LIBABCKIT_LOG(DEBUG) << "  Found namespace: " << namespaceName << "'\n";
    return std::make_unique<AbckitCoreNamespace>(nullptr, AbckitArktsNamespace(&record));
}

template <typename AbckitCoreType, typename AbckitArktsType, typename AbckitCoreTypeField,
          typename AbckitArktsTypeField>
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
        for (const auto &recordField : record.fieldList) {
            LIBABCKIT_LOG(DEBUG) << "Found instance Field: " << recordField.name << "\n";
            auto field = CreateField<AbckitCoreType, AbckitCoreTypeField, AbckitArktsTypeField>(
                instanceModule.get(), instance.get(), recordField);
            instance->fields.emplace_back(std::move(field));
        }
    }
    return instance;
}

static void AssignOwningModuleOfNamespace(
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
    std::vector<std::unique_ptr<AbckitCoreNamespace>> &namespaces)
{
    for (auto &ns : namespaces) {
        auto fullName = ns->GetArkTSImpl()->impl.GetStaticClass()->name;
        auto [moduleName, className] = ClassGetNames(fullName);
        ns->owningModule = nameToModule[moduleName].get();
    }
}

template <typename AbckitCoreType>
static void AssignInstance(std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
                           std::vector<std::unique_ptr<AbckitCoreType>> &instances)
{
    for (auto &instance : instances) {
        auto fullName = instance->GetArkTSImpl()->impl.GetStaticClass()->name;
        auto [moduleName, instanceName] = ClassGetNames(fullName);
        LIBABCKIT_LOG(DEBUG) << "moduleName, className, fullName: " << moduleName << ", " << instanceName << ", "
                             << fullName << '\n';
        auto &instanceModule = nameToModule[moduleName];
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

static void CreateAndAssignInterfaceField(std::unique_ptr<AbckitCoreInterface> &interface,
                                          const std::string &functionName)
{
    std::smatch match;
    if (IsInterfaceGetFunction(functionName, match)) {
        const auto &fieldName = match[1].str();
        LIBABCKIT_LOG(DEBUG) << "Found interface Field: " << fieldName << "\n";
        interface->fields.emplace(fieldName, CreateInterfaceField(interface, fieldName));
    } else if (IsInterfaceSetFunction(functionName, match)) {
        const auto &fieldName = match[1].str();
        interface->fields[fieldName]->flag = ACC_PUBLIC;
    }
}

using HandlerFunc = std::function<void(AbckitCoreModule *, std::unique_ptr<AbckitCoreFunction> &&, const std::string &,
                                       const std::string &)>;

struct HandlerInfo {
    // handler to the function if condition is true
    HandlerFunc handler;
    // Tell if function belong to some vector
    std::function<bool(AbckitCoreModule *, const std::string &)> condition;
};

static void HandleGlobalFunction(AbckitCoreModule *module, std::unique_ptr<AbckitCoreFunction> &&function,
                                 const std::string &className, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Module Function. module: '" << className << "' function: '" << functionName
                         << "'\n";
    module->functions.emplace_back(std::move(function));
}

static void HandleClassFunction(AbckitCoreModule *module, std::unique_ptr<AbckitCoreFunction> &&function,
                                const std::string &className, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Class Function. class: '" << className << "' function: '" << functionName << "'\n";
    auto &klass = module->ct[className];
    function->parentClass = klass.get();
    klass->methods.emplace_back(std::move(function));
}

static void HandleNamespaceFunction(AbckitCoreModule *module, std::unique_ptr<AbckitCoreFunction> &&function,
                                    const std::string &namespaceName, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Namespace Function. namespace: '" << namespaceName << "' function: '"
                         << functionName << "'\n";
    module->nt[namespaceName]->functions.emplace_back(std::move(function));
}

static void HandleInterfaceFunction(AbckitCoreModule *module, std::unique_ptr<AbckitCoreFunction> &&function,
                                    const std::string &interfaceName, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Interface Function. interface: '" << interfaceName << "' function: '"
                         << functionName << "'\n";
    auto &interface = module->it[interfaceName];
    CreateAndAssignInterfaceField(interface, functionName);
    interface->methods.emplace_back(std::move(function));
}

static void HandleEnumFunction(AbckitCoreModule *module, std::unique_ptr<AbckitCoreFunction> &&function,
                               const std::string &enumName, const std::string &functionName)
{
    LIBABCKIT_LOG(DEBUG) << "Assign Enum Function. enum: '" << enumName << "' function: '" << functionName << "'\n";
    module->et[enumName]->methods.emplace_back(std::move(function));
}

static void ProcessFunction(AbckitCoreModule *module, std::unique_ptr<AbckitCoreFunction> &&function,
                            const std::string &className, const std::string &functionName)
{
    std::vector<HandlerInfo> handlers = {
        {HandleGlobalFunction,
         [](AbckitCoreModule *, const std::string &className) { return IsModuleName(className); }},
        {HandleNamespaceFunction,
         [](AbckitCoreModule *module, const std::string &className) {
             return module->nt.find(className) != module->nt.end();
         }},
        {HandleClassFunction,
         [](AbckitCoreModule *module, const std::string &className) {
             return module->ct.find(className) != module->ct.end();
         }},
        {HandleInterfaceFunction,
         [](AbckitCoreModule *module, const std::string &className) {
             return module->it.find(className) != module->it.end();
         }},
        {HandleEnumFunction, [](AbckitCoreModule *module, const std::string &className) {
             return module->et.find(className) != module->et.end();
         }}};

    for (const auto &handlerInfo : handlers) {
        if (handlerInfo.condition(module, className)) {
            handlerInfo.handler(module, std::move(function), className, functionName);
            return;
        }
    }
}

static void AssignFunctions(std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
                            std::vector<std::unique_ptr<AbckitCoreFunction>> &functions)
{
    for (auto &function : functions) {
        std::string functionName = FunctionGetImpl(function.get())->name;
        auto [moduleName, className] = FuncGetNames(functionName);
        auto &functionModule = nameToModule[moduleName];
        ProcessFunction(functionModule.get(), std::move(function), className, functionName);
    }
}

static bool IsNamespace(const std::string &className, const pandasm::Record &record)
{
    if (IsModuleName(className)) {
        return false;
    }
    const auto &attributes = record.metadata->GetAttributeValues(ETS_ANNOTATION_CLASS.data());
    return std::any_of(attributes.begin(), attributes.end(),
                       [](const std::string &name) { return name == ETS_ANNOTATION_MODULE; });
}

static bool IsInterface(const pandasm::Record &record)
{
    return (record.metadata->GetAccessFlags() & ACC_INTERFACE) != 0;
}

static bool IsEnum(const pandasm::Record &record)
{
    return record.metadata->GetBase() == ENUM_BASE;
}

static const std::string LAMBDA_RECORD_KEY = "LambdaObject";
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
        return nameToInterface.find(className) != nameToInterface.end();
    }

    return true;
}

static std::vector<std::unique_ptr<AbckitCoreFunction>> CollectAllFunctions(
    pandasm::Program *prog, AbckitFile *file,
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &nameToModule,
    const std::unordered_map<std::string, AbckitCoreInterface *> &nameToInterface)
{
    std::vector<std::unique_ptr<AbckitCoreFunction>> functions;
    for (auto &[functionName, functionImpl] : prog->functionStaticTable) {
        auto [moduleName, className] = FuncGetNames(functionName);

        if (ShouldCreateFuncWrapper(functionImpl, className, functionName, nameToInterface)) {
            functions.emplace_back(CollectFunction(file, nameToModule, functionName, functionImpl));
        }
    }
    for (auto &[functionName, functionImpl] : prog->functionInstanceTable) {
        auto [moduleName, className] = FuncGetNames(functionName);

        if (ShouldCreateFuncWrapper(functionImpl, className, functionName, nameToInterface)) {
            functions.emplace_back(CollectFunction(file, nameToModule, functionName, functionImpl));
        }
    }
    return functions;
}

// Container of vectors and maps of some kinds of instances
// Including modules, classes, namespaces, interfaces, enums and functions
struct Container {
    std::vector<std::unique_ptr<AbckitCoreClass>> classes;
    std::vector<std::unique_ptr<AbckitCoreInterface>> interfaces;
    std::vector<std::unique_ptr<AbckitCoreNamespace>> namespaces;
    std::vector<std::unique_ptr<AbckitCoreEnum>> enums;
    std::vector<std::unique_ptr<AbckitCoreFunction>> functions;
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> nameToModule;
    std::unordered_map<std::string, AbckitCoreNamespace *> nameToNamespace;
    std::unordered_map<std::string, AbckitCoreClass *> nameToClass;
    std::unordered_map<std::string, AbckitCoreInterface *> nameToInterface;
};

static void CollectAllInstances(pandasm::Program *prog, Container &container)
{
    for (auto &[recordName, record] : prog->recordTable) {
        if (record.metadata->IsForeign() || (recordName.find(LAMBDA_RECORD_KEY) != std::string::npos)) {
            // NOTE: find and fill AbckitCoreImportDescriptor
            continue;
        }

        auto [moduleName, className] = ClassGetNames(recordName);
        if (IsModuleName(className)) {
            continue;
        }
        if (IsNamespace(className, record)) {
            continue;
        }

        auto &nameToModule = container.nameToModule;
        if (IsInterface(record)) {
            container.interfaces.emplace_back(
                CreateInstance<AbckitCoreInterface, AbckitArktsInterface, AbckitCoreInterfaceField,
                               AbckitArktsInterfaceField>(nameToModule, moduleName, className, record));
            container.nameToInterface[className] = container.interfaces.back().get();
            continue;
        }
        if (IsEnum(record)) {
            container.enums.emplace_back(
                CreateInstance<AbckitCoreEnum, AbckitArktsEnum, AbckitCoreEnumField, AbckitArktsEnumField>(
                    nameToModule, moduleName, className, record));
            continue;
        }
        container.classes.emplace_back(
            CreateInstance<AbckitCoreClass, AbckitArktsClass, AbckitCoreClassField, AbckitArktsClassField>(
                nameToModule, moduleName, className, record));
        container.nameToClass[className] = container.classes.back().get();
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
            container.namespaces.emplace_back(CreateNamespace(record, namespaceName));
            container.nameToNamespace.emplace(namespaceName, container.namespaces.back().get());
        }
    }

    // Collect classes, interfaces and enums
    CollectAllInstances(prog, container);

    // Functions
    container.functions = CollectAllFunctions(prog, file, container.nameToModule, container.nameToInterface);

    auto &nameToModule = container.nameToModule;
    AssignOwningModuleOfNamespace(nameToModule, container.namespaces);
    AssignInstance(nameToModule, container.classes);
    AssignInstance(nameToModule, container.interfaces);
    AssignInstance(nameToModule, container.enums);
    AssignInstance(nameToModule, container.namespaces);
    AssignFunctions(nameToModule, container.functions);

    // NOTE: AbckitCoreExportDescriptor
    // NOTE: AbckitModulePayload

    for (auto &[moduleName, module] : nameToModule) {
        file->localModules.insert({moduleName, std::move(module)});
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
