/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "libabckit/include/c/metadata_core.h"
#include "libabckit/src/adapter_static/helpers_static.h"
#include "libabckit/include/c/ir_core.h"
#include "libabckit/src/statuses_impl.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "libabckit/src/adapter_static/runtime_adapter_static.h"
#include "libabckit/src/logger.h"
#include "libpandafile/file.h"

#include "src/adapter_static/metadata_modify_static.h"

#include "static_core/abc2program/abc2program_driver.h"

#include "static_core/assembler/assembly-emitter.h"

#include <cstdint>
#include <memory>
#include <unordered_map>

// CC-OFFNXT(WordsTool.95 Google) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace ark;

namespace libabckit {

[[maybe_unused]] static bool IsAbstract(pandasm::ItemMetadata *meta)
{
    uint32_t flags = meta->GetAccessFlags();
    return (flags & ACC_ABSTRACT) != 0U;
}

AbckitString *CreateNameString(AbckitFile *file, std::string &name)
{
    return CreateStringStatic(file, name.data());
}

void CollectFunction(AbckitFile *file, std::unordered_set<std::string> &globalClassNames,
                     std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &modulesMap,
                     const std::string &functionName, ark::pandasm::Function &functionImpl)
{
    auto [moduleName, className] = FuncGetNames(functionName);
    LIBABCKIT_LOG(DEBUG) << "  Found function. module: '" << moduleName << "' class: '" << className << "' function: '"
                         << functionName << "'\n";
    ASSERT(modulesMap.find(moduleName) != modulesMap.end());
    auto &functionModule = modulesMap[moduleName];
    auto function = std::make_unique<AbckitCoreFunction>();
    function->m = functionModule.get();
    function->impl = std::make_unique<AbckitArktsFunction>();
    function->GetArkTSImpl()->impl = &functionImpl;
    function->GetArkTSImpl()->core = function.get();
    auto name = GetMangleFuncName(function.get());
    ASSERT(file->nameToFunction.count(name) == 0);
    file->nameToFunction.insert({name, function.get()});

    for (auto &annoImpl : functionImpl.metadata->GetAnnotations()) {
        auto anno = std::make_unique<AbckitCoreAnnotation>();
        anno->impl = std::make_unique<AbckitArktsAnnotation>();
        anno->GetArkTSImpl()->core = anno.get();

        for (auto &annoElemImpl : annoImpl.GetElements()) {
            auto annoElem = std::make_unique<AbckitCoreAnnotationElement>();
            annoElem->ann = anno.get();
            annoElem->name = CreateStringStatic(file, annoElemImpl.GetName().data());
            annoElem->impl = std::make_unique<AbckitArktsAnnotationElement>();
            annoElem->GetArkTSImpl()->core = annoElem.get();
            auto value = std::make_unique<AbckitValue>(file, annoElemImpl.GetValue());
            annoElem->value = std::move(value);

            anno->elements.emplace_back(std::move(annoElem));
        }
        function->annotations.emplace_back(std::move(anno));
    }

    if (globalClassNames.find(className) != globalClassNames.end()) {
        functionModule->functions.emplace_back(std::move(function));
    } else {
        ASSERT(functionModule->ct.find(className) != functionModule->ct.end());
        auto &klass = functionModule->ct[className];
        function->klass = klass.get();
        klass->methods.emplace_back(std::move(function));
    }
}

static void CollectFunctions(pandasm::Program *prog, AbckitFile *file,
                             std::unordered_set<std::string> &globalClassNames,
                             std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &modulesMap)
{
    static const std::string LAMBDA_RECORD_KEY = "LambdaObject";
    static const std::string INIT_FUNC_NAME = "_$init$_";
    static const std::string TRIGGER_CCTOR_FUNC_NAME = "_$trigger_cctor$_";

    for (auto &[functionName, functionImpl] : prog->functionTable) {
        auto [moduleName, className] = FuncGetNames(functionName);

        if (functionImpl.metadata->IsForeign() ||
            (className.substr(0, LAMBDA_RECORD_KEY.size()) == LAMBDA_RECORD_KEY) ||
            (functionName.find(INIT_FUNC_NAME, 0) != std::string::npos) ||
            (functionName.find(TRIGGER_CCTOR_FUNC_NAME, 0) != std::string::npos)) {
            // NOTE: find and fill AbckitCoreImportDescriptor
            continue;
        }

        if (IsAbstract(functionImpl.metadata.get())) {
            // NOTE: ?
            continue;
        }

        CollectFunction(file, globalClassNames, modulesMap, functionName, functionImpl);
    }
}

static void CreateModule(AbckitFile *file, std::unordered_set<std::string> &globalClassNames,
                         std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &modulesMap,
                         const std::string &recordName)
{
    auto [moduleName, className] = ClassGetNames(recordName);
    if (globalClassNames.find(className) != globalClassNames.end()) {
        if (modulesMap.find(moduleName) != modulesMap.end()) {
            LIBABCKIT_LOG(FATAL) << "Duplicated ETSGLOBAL for module: " << moduleName << '\n';
        }

        LIBABCKIT_LOG(DEBUG) << "Found module: '" << moduleName << "'\n";
        auto m = std::make_unique<AbckitCoreModule>();
        m->file = file;
        m->target = ABCKIT_TARGET_ARK_TS_V2;
        m->moduleName = CreateStringStatic(file, moduleName.data());
        m->impl = std::make_unique<AbckitArktsModule>();
        m->GetArkTSImpl()->core = m.get();
        modulesMap.insert({moduleName, std::move(m)});
    }
}

void CreateClass(std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> &modulesMap,
                 const std::string &moduleName, const std::string &className, ark::pandasm::Record &record)
{
    LIBABCKIT_LOG(DEBUG) << "  Found class. module: '" << moduleName << "' class: '" << className << "'\n";
    ASSERT(modulesMap.find(moduleName) != modulesMap.end());
    auto &classModule = modulesMap[moduleName];
    auto abckitRecord = &record;
    auto klass = std::make_unique<AbckitCoreClass>(classModule.get(), AbckitArktsClass(abckitRecord));
    classModule->InsertClass(className, std::move(klass));
}

// CC-OFFNXT(G.FUD.05) huge function
static void CreateWrappers(pandasm::Program *prog, AbckitFile *file)
{
    std::unordered_set<std::string> globalClassNames = {"ETSGLOBAL", "_GLOBAL"};
    static const std::string LAMBDA_RECORD_KEY = "LambdaObject";
    static const std::string INIT_FUNC_NAME = "_$init$_";
    static const std::string TRIGGER_CCTOR_FUNC_NAME = "_$trigger_cctor$_";
    file->program = prog;

    // Collect modules
    std::unordered_map<std::string, std::unique_ptr<AbckitCoreModule>> modulesMap;
    for (auto &[recordName, record] : prog->recordTable) {
        if (record.metadata->IsForeign()) {
            // NOTE: Create AbckitCoreImportDescriptor and AbckitCoreModuleDescriptor
            continue;
        }
        CreateModule(file, globalClassNames, modulesMap, recordName);
    }

    // Collect classes
    for (auto &[recordName, record] : prog->recordTable) {
        if (record.metadata->IsForeign() || (recordName.find(LAMBDA_RECORD_KEY) != std::string::npos)) {
            // NOTE: find and fill AbckitCoreImportDescriptor
            continue;
        }

        // NOTE: AbckitCoreAnnotationInterface

        auto [moduleName, className] = ClassGetNames(recordName);
        if (globalClassNames.find(className) != globalClassNames.end()) {
            continue;
        }
        CreateClass(modulesMap, moduleName, className, record);
    }

    // Functions
    CollectFunctions(prog, file, globalClassNames, modulesMap);

    // NOTE: AbckitCoreExportDescriptor
    // NOTE: AbckitModulePayload

    for (auto &[moduleName, module] : modulesMap) {
        file->localModules.insert({moduleName, std::move(module)});
    }

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

AbckitFile *OpenAbcStatic(const char *path)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << path << '\n';
    auto *abc2program = new ark::abc2program::Abc2ProgramDriver();
    if (!abc2program->Compile(path)) {
        LIBABCKIT_LOG(DEBUG) << "Failed to open " << path << "\n";
        delete abc2program;
        return nullptr;
    }
    pandasm::Program &prog = abc2program->GetProgram();
    auto file = new AbckitFile();
    file->frontend = Mode::STATIC;
    CreateWrappers(&prog, file);

    auto pf = panda_file::File::Open(path);
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

    for (auto &val : file->values) {
        delete val->GetStaticImpl();
    }

    auto *ctxIInternal = reinterpret_cast<struct CtxIInternal *>(file->internal);
    delete ctxIInternal->driver;
    delete ctxIInternal;
    delete[] file->version;
    delete file;
}

void WriteAbcStatic(AbckitFile *file, const char *path)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << path << '\n';

    auto program = file->GetStaticProgram();

    auto emitDebugInfo = false;
    std::map<std::string, size_t> *statp = nullptr;
    ark::pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp = nullptr;

    if (!pandasm::AsmEmitter::Emit(path, *program, statp, mapsp, emitDebugInfo)) {
        LIBABCKIT_LOG(DEBUG) << "FAILURE\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_TODO);
        return;
    }
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
