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

#include "libabckit/include/c/metadata_core.h"
#include "libabckit/include/c/extensions/arkts/metadata_arkts.h"
#include "libabckit/include/c/extensions/js/metadata_js.h"

#include "libabckit/src/adapter_dynamic/metadata_modify_dynamic.h"
#include "libabckit/src/adapter_dynamic/metadata_inspect_dynamic.h"
#include "libabckit/src/wrappers/graph_wrapper/graph_wrapper.h"
#include "libabckit/src/logger.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "assembler/annotation.h"
#include "assembler/assembly-literals.h"
#include "assembler/assembly-program.h"
#include "assembler/assembly-function.h"
#include "assembler/assembly-record.h"
#include "src/adapter_dynamic/helpers_dynamic.h"
#include "src/statuses_impl.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <tuple>

namespace libabckit {

// CC-OFFNXT(WordsTool.95 Google) sensitive word conflict
// NOLINTNEXTLINE(google-build-using-namespace)
using namespace panda;

AbckitString *CreateStringDynamic(AbckitFile *file, const char *value)
{
    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << "\"" << value << "\"" << '\n';
    auto *prog = file->GetDynamicProgram();
    const auto &[progValueIter, _] = prog->strings.insert(value);
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

void FunctionSetGraphDynamic(AbckitCoreFunction *function, AbckitGraph *graph)
{
    LIBABCKIT_LOG_FUNC;

    auto func = GetDynFunction(function);
    LIBABCKIT_LOG(DEBUG) << "============================================ BEFORE CODEGEN: " << func->name << '\n';
    LIBABCKIT_LOG_DUMP(func->DebugDump(), DEBUG);

    auto res = GraphWrapper::BuildCodeDynamic(graph, func->name);
    auto status = std::get<1>(res);
    if (status != AbckitStatus::ABCKIT_STATUS_NO_ERROR) {
        GraphWrapper::DestroyGraphDynamic(graph);
        statuses::SetLastError(status);
        return;
    }
    void *fw = std::get<0>(res);

    LIBABCKIT_LOG_FUNC;
    LIBABCKIT_LOG(DEBUG) << "============================================ AFTER CODEGEN: " << func->name << '\n';
    LIBABCKIT_LOG_DUMP((reinterpret_cast<pandasm::Function *>(fw))->DebugDump(), DEBUG);

    GraphWrapper::DestroyGraphDynamic(graph);

    auto *newFunc = reinterpret_cast<pandasm::Function *>(fw);
    func->ins = newFunc->ins;
    func->catch_blocks = newFunc->catch_blocks;
    func->regs_num = newFunc->regs_num;

    // update IC slot number info in annotation
    func->slots_num = newFunc->slots_num;
    static const std::string SLOT_NUMBER = "_ESSlotNumberAnnotation";
    static const std::string ELEMENT_NAME = "SlotNumber";
    size_t slotNumAnnoIndex = 0;
    for (const auto &an : func->metadata->GetAnnotations()) {
        if (an.GetName() == SLOT_NUMBER) {
            pandasm::AnnotationElement ele(ELEMENT_NAME, std::make_unique<pandasm::ScalarValue>(
                                                             pandasm::ScalarValue::Create<pandasm::Value::Type::U32>(
                                                                 static_cast<uint32_t>(func->slots_num))));
            func->metadata->SetOrAddAnnotationElementByIndex(slotNumAnnoIndex, 0, std::move(ele));
            break;
        }
        slotNumAnnoIndex++;
    }

    delete newFunc;
}

AbckitJsModule *FileAddExternalModuleDynamic(AbckitFile *file, const struct AbckitJsExternalModuleCreateParams *params)
{
    auto m = std::make_unique<AbckitCoreModule>();
    m->impl = std::make_unique<AbckitJsModule>();
    m->file = file;
    m->target = ABCKIT_TARGET_JS;

    auto *prog = file->GetDynamicProgram();

    prog->strings.insert(params->name);
    auto &strings = file->strings;
    if (strings.find(params->name) != strings.end()) {
        m->moduleName = strings.at(params->name).get();
    } else {
        auto s = std::make_unique<AbckitString>();
        s->impl = params->name;
        strings.insert({params->name, std::move(s)});
        m->moduleName = strings[params->name].get();
    }

    m->isExternal = true;
    auto modulePayloadDyn = AbckitModulePayloadDyn();
    modulePayloadDyn.absPaths = false;
    m->GetJSImpl()->impl = modulePayloadDyn;
    file->externalModules.insert({params->name, std::move(m)});
    return file->externalModules[params->name]->GetJSImpl();
}

AbckitArktsModule *FileAddExternalModuleDynamic(AbckitFile *file,
                                                const struct AbckitArktsExternalModuleCreateParams *params)
{
    auto m = std::make_unique<AbckitCoreModule>();
    m->impl = std::make_unique<AbckitArktsModule>();
    m->file = file;
    m->target = ABCKIT_TARGET_ARK_TS_V1;

    auto *prog = file->GetDynamicProgram();

    prog->strings.insert(params->name);
    auto &strings = file->strings;
    if (strings.find(params->name) != strings.end()) {
        m->moduleName = strings.at(params->name).get();
    } else {
        auto s = std::make_unique<AbckitString>();
        s->impl = params->name;
        strings.insert({params->name, std::move(s)});
        m->moduleName = strings[params->name].get();
    }

    m->isExternal = true;
    auto modulePayloadDyn = AbckitModulePayloadDyn();
    modulePayloadDyn.absPaths = false;
    m->GetArkTSImpl()->impl.GetDynModule() = modulePayloadDyn;
    file->externalModules.insert({params->name, std::move(m)});
    return file->externalModules[params->name]->GetArkTSImpl();
}

void ModuleRemoveImportDynamic(AbckitCoreModule *m, AbckitArktsImportDescriptor *i)
{
    LIBABCKIT_LOG_FUNC;
    auto found = std::find_if(m->id.begin(), m->id.end(),
                              [&](std::unique_ptr<AbckitCoreImportDescriptor> const &d) { return d.get() == i->core; });
    if (found == m->id.end()) {
        LIBABCKIT_LOG(DEBUG) << "Can not find the import to delete\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }

    auto mPayload = GetDynModulePayload(m);
    auto idxOffset =
        i->payload.GetDynId().isRegularImport ? mPayload->regularImportsOffset : mPayload->namespaceImportsOffset;
    auto gap = i->payload.GetDynId().isRegularImport ? 3 : 2;
    (mPayload->moduleLiteralArray->GetDynamicImpl())
        ->literals_[idxOffset + i->payload.GetDynId().moduleRecordIndexOff * gap]
        .tag_ = panda_file::LiteralTag::NULLVALUE;

    m->id.erase(found);
}

void ModuleRemoveImportDynamic(AbckitCoreModule *m, AbckitJsImportDescriptor *i)
{
    LIBABCKIT_LOG_FUNC;
    auto found = std::find_if(m->id.begin(), m->id.end(),
                              [&](std::unique_ptr<AbckitCoreImportDescriptor> const &d) { return d.get() == i->core; });
    if (found == m->id.end()) {
        LIBABCKIT_LOG(DEBUG) << "Can not find the import to delete\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }

    auto mPayload = GetDynModulePayload(m);
    auto idxOffset =
        i->payload.GetDynId().isRegularImport ? mPayload->regularImportsOffset : mPayload->namespaceImportsOffset;
    auto gap = i->payload.GetDynId().isRegularImport ? 3 : 2;
    (mPayload->moduleLiteralArray->GetDynamicImpl())
        ->literals_[idxOffset + i->payload.GetDynId().moduleRecordIndexOff * gap]
        .tag_ = panda_file::LiteralTag::NULLVALUE;

    m->id.erase(found);
}

void AddNewModuleRequest(AbckitCoreModule *m, AbckitCoreModule *newModule)
{
    auto mPayload = GetDynModulePayload(m);
    auto *moduleLitArr = mPayload->moduleLiteralArray->GetDynamicImpl();
    m->md.push_back(newModule);
    auto requestsIdxNum = std::get<uint32_t>(moduleLitArr->literals_[mPayload->moduleRequestsOffset - 1].value_);
    moduleLitArr->literals_[mPayload->moduleRequestsOffset - 1].value_ = requestsIdxNum + 1;
    auto literalModuleRequest = pandasm::LiteralArray::Literal {panda_file::LiteralTag::STRING,
                                                                "./" + std::string(newModule->moduleName->impl)};
    moduleLitArr->literals_.insert(moduleLitArr->literals_.begin() + m->md.size(), std::move(literalModuleRequest));

    mPayload->regularImportsOffset = mPayload->regularImportsOffset + 1;
    mPayload->namespaceImportsOffset = mPayload->namespaceImportsOffset + 1;
    mPayload->localExportsOffset = mPayload->localExportsOffset + 1;
    mPayload->indirectExportsOffset = mPayload->indirectExportsOffset + 1;
    mPayload->starExportsOffset = mPayload->starExportsOffset + 1;
}

template <class T>
size_t AddNamespaceImportToModuleLiteralArray(AbckitCoreModule *importing, AbckitCoreModule *imported, const T *params)
{
    auto mPayload = GetDynModulePayload(importing);
    auto literalLocalName = pandasm::LiteralArray::Literal {panda_file::LiteralTag::STRING, std::string(params->alias)};
    auto *moduleLitArr = mPayload->moduleLiteralArray->GetDynamicImpl();

    ASSERT((imported->isExternal && importing->file->externalModules.find(imported->moduleName->impl.data()) !=
                                        importing->file->externalModules.end()) ||
           (!imported->isExternal && importing->file->localModules.find(imported->moduleName->impl.data()) !=
                                         importing->file->localModules.end()));

    auto found = std::find(importing->md.begin(), importing->md.end(), imported);
    uint16_t requestIdx;
    if (found != importing->md.end()) {
        requestIdx = std::distance(importing->md.begin(), found);
    } else {
        AddNewModuleRequest(importing, imported);
        requestIdx = importing->md.size() - 1;
    }

    auto literalRequestIdx = pandasm::LiteralArray::Literal {panda_file::LiteralTag::METHODAFFILIATE, requestIdx};
    moduleLitArr->literals_.insert(moduleLitArr->literals_.begin() + mPayload->localExportsOffset - 1,
                                   std::move(literalRequestIdx));
    moduleLitArr->literals_.insert(moduleLitArr->literals_.begin() + mPayload->localExportsOffset - 1,
                                   std::move(literalLocalName));

    auto namespaceImportsNum = std::get<uint32_t>(moduleLitArr->literals_[mPayload->namespaceImportsOffset - 1].value_);
    moduleLitArr->literals_[mPayload->namespaceImportsOffset - 1].value_ = namespaceImportsNum + 1;
    mPayload->localExportsOffset = mPayload->localExportsOffset + 2U;
    mPayload->indirectExportsOffset = mPayload->indirectExportsOffset + 2U;
    mPayload->starExportsOffset = mPayload->starExportsOffset + 2U;

    return namespaceImportsNum;
}

template <class T>
size_t AddRegularImportToModuleLiteralArray(AbckitCoreModule *importing, AbckitCoreModule *imported, const T *params)
{
    auto mPayload = GetDynModulePayload(importing);
    auto literalLocalName = pandasm::LiteralArray::Literal {panda_file::LiteralTag::STRING, std::string(params->alias)};
    auto literalImportName = pandasm::LiteralArray::Literal {panda_file::LiteralTag::STRING, std::string(params->name)};
    auto *moduleLitArr = mPayload->moduleLiteralArray->GetDynamicImpl();

    ASSERT((imported->isExternal && importing->file->externalModules.find(imported->moduleName->impl.data()) !=
                                        importing->file->externalModules.end()) ||
           (!imported->isExternal && importing->file->localModules.find(imported->moduleName->impl.data()) !=
                                         importing->file->localModules.end()));

    auto found = std::find(importing->md.begin(), importing->md.end(), imported);
    uint16_t requestIdx;
    if (found != importing->md.end()) {
        requestIdx = std::distance(importing->md.begin(), found);
    } else {
        AddNewModuleRequest(importing, imported);
        requestIdx = importing->md.size() - 1;
    }

    auto literalRequestIdx = pandasm::LiteralArray::Literal {panda_file::LiteralTag::METHODAFFILIATE, requestIdx};
    moduleLitArr->literals_.insert(moduleLitArr->literals_.begin() + mPayload->namespaceImportsOffset - 1,
                                   std::move(literalRequestIdx));
    moduleLitArr->literals_.insert(moduleLitArr->literals_.begin() + mPayload->namespaceImportsOffset - 1,
                                   std::move(literalImportName));
    moduleLitArr->literals_.insert(moduleLitArr->literals_.begin() + mPayload->namespaceImportsOffset - 1,
                                   std::move(literalLocalName));

    auto regularImportsNum = std::get<uint32_t>(moduleLitArr->literals_[mPayload->regularImportsOffset - 1].value_);
    moduleLitArr->literals_[mPayload->regularImportsOffset - 1].value_ = regularImportsNum + 1;
    const size_t three = 3;
    mPayload->namespaceImportsOffset = mPayload->namespaceImportsOffset + three;
    mPayload->localExportsOffset = mPayload->localExportsOffset + three;
    mPayload->indirectExportsOffset = mPayload->indirectExportsOffset + three;
    mPayload->starExportsOffset = mPayload->starExportsOffset + three;

    return regularImportsNum;
}

template <class T>
size_t AddImportToModuleLiteralArray(AbckitCoreModule *importing, AbckitCoreModule *imported, const T *params)
{
    return (std::strcmp(params->name, "*") == 0) ? AddNamespaceImportToModuleLiteralArray(importing, imported, params)
                                                 : AddRegularImportToModuleLiteralArray(importing, imported, params);
}

AbckitArktsImportDescriptor *ModuleAddImportFromDynamicModuleDynamic(
    AbckitCoreModule *importing, AbckitCoreModule *imported,
    const AbckitArktsImportFromDynamicModuleCreateParams *params)
{
    auto id = std::make_unique<AbckitCoreImportDescriptor>();
    id->importingModule = importing;
    id->importedModule = imported;
    auto payloadDyn = AbckitDynamicImportDescriptorPayload();
    payloadDyn.isRegularImport = std::strcmp(params->name, "*") != 0;
    payloadDyn.moduleRecordIndexOff = AddImportToModuleLiteralArray(importing, imported, params);
    auto payload = AbckitCoreImportDescriptorPayload();
    payload.impl = payloadDyn;
    id->impl = std::make_unique<AbckitArktsImportDescriptor>();
    id->GetArkTSImpl()->kind = AbckitImportExportDescriptorKind::UNTYPED;
    id->GetArkTSImpl()->payload = payload;
    id->GetArkTSImpl()->core = id.get();
    importing->id.push_back(std::move(id));

    return importing->id[importing->id.size() - 1]->GetArkTSImpl();
}

AbckitJsImportDescriptor *ModuleAddImportFromDynamicModuleDynamic(
    AbckitCoreModule *importing, AbckitCoreModule *imported, const AbckitJsImportFromDynamicModuleCreateParams *params)
{
    auto id = std::make_unique<AbckitCoreImportDescriptor>();
    id->importingModule = importing;
    id->importedModule = imported;
    auto payloadDyn = AbckitDynamicImportDescriptorPayload();
    payloadDyn.isRegularImport = std::strcmp(params->name, "*") != 0;
    payloadDyn.moduleRecordIndexOff = AddImportToModuleLiteralArray(importing, imported, params);
    auto payload = AbckitCoreImportDescriptorPayload();
    payload.impl = payloadDyn;
    id->impl = std::make_unique<AbckitJsImportDescriptor>();
    id->GetJSImpl()->payload = payload;
    id->GetJSImpl()->kind = AbckitImportExportDescriptorKind::UNTYPED;
    id->GetJSImpl()->core = id.get();
    importing->id.push_back(std::move(id));

    return importing->id[importing->id.size() - 1]->GetJSImpl();
}

template <class T>
size_t AddLocalExportToModuleLiteralArray(AbckitCoreModule *exporting, AbckitCoreModule * /*exported*/, const T *params)
{
    auto mPayload = GetDynModulePayload(exporting);
    auto literalLocalName = pandasm::LiteralArray::Literal {panda_file::LiteralTag::STRING, std::string(params->alias)};
    auto literalExportName = pandasm::LiteralArray::Literal {panda_file::LiteralTag::STRING, std::string(params->name)};
    auto *moduleLitArr = mPayload->moduleLiteralArray->GetDynamicImpl();

    moduleLitArr->literals_.insert(moduleLitArr->literals_.begin() + mPayload->indirectExportsOffset - 1U,
                                   std::move(literalExportName));
    moduleLitArr->literals_.insert(moduleLitArr->literals_.begin() + mPayload->indirectExportsOffset - 1U,
                                   std::move(literalLocalName));

    auto localExportsNum = std::get<uint32_t>(moduleLitArr->literals_[mPayload->localExportsOffset - 1U].value_);
    moduleLitArr->literals_[mPayload->localExportsOffset - 1U].value_ = localExportsNum + 1U;
    mPayload->starExportsOffset = mPayload->indirectExportsOffset + 2U;
    mPayload->starExportsOffset = mPayload->starExportsOffset + 2U;

    return localExportsNum;
}

template <class T>
size_t AddIndirectExportToModuleLiteralArray(AbckitCoreModule *exporting, AbckitCoreModule *exported, const T *params)
{
    auto mPayload = GetDynModulePayload(exporting);
    auto literalExportName =
        pandasm::LiteralArray::Literal {panda_file::LiteralTag::STRING, std::string(params->alias)};
    auto literalImportName = pandasm::LiteralArray::Literal {panda_file::LiteralTag::STRING, std::string(params->name)};
    auto *moduleLitArr = mPayload->moduleLiteralArray->GetDynamicImpl();

    ASSERT((exported->isExternal && exporting->file->externalModules.find(exported->moduleName->impl.data()) !=
                                        exporting->file->externalModules.end()) ||
           (!exported->isExternal && exporting->file->localModules.find(exported->moduleName->impl.data()) !=
                                         exporting->file->localModules.end()));

    auto found = std::find(exporting->md.begin(), exporting->md.end(), exported);
    uint16_t requestIdx;
    if (found != exporting->md.end()) {
        requestIdx = std::distance(exporting->md.begin(), found);
    } else {
        AddNewModuleRequest(exporting, exported);
        requestIdx = exporting->md.size() - 1;
    }

    auto literalRequestIdx = pandasm::LiteralArray::Literal {panda_file::LiteralTag::METHODAFFILIATE, requestIdx};
    moduleLitArr->literals_.insert(moduleLitArr->literals_.begin() + mPayload->starExportsOffset - 1,
                                   std::move(literalRequestIdx));
    moduleLitArr->literals_.insert(moduleLitArr->literals_.begin() + mPayload->starExportsOffset - 1,
                                   std::move(literalImportName));
    moduleLitArr->literals_.insert(moduleLitArr->literals_.begin() + mPayload->starExportsOffset - 1,
                                   std::move(literalExportName));

    auto indirectExportsNum = std::get<uint32_t>(moduleLitArr->literals_[mPayload->indirectExportsOffset - 1].value_);
    moduleLitArr->literals_[mPayload->indirectExportsOffset - 1].value_ = indirectExportsNum + 1;
    mPayload->starExportsOffset = mPayload->starExportsOffset + 3U;

    return indirectExportsNum;
}

std::string GetServiceExportName(AbckitCoreModule *m)
{
    auto maxNum = -1;
    auto mPayload = GetDynModulePayload(m);
    const auto *moduleLitArr = mPayload->moduleLiteralArray;
    for (auto &ed : m->ed) {
        auto edPayload = GetDynExportDescriptorPayload(ed.get());
        if (edPayload->hasServiceImport) {
            auto exportNameOffset = mPayload->localExportsOffset + edPayload->moduleRecordIndexOff * 2;
            auto name = std::get<std::string>(moduleLitArr->GetDynamicImpl()->literals_[exportNameOffset].value_);
            ASSERT(name.find("=ens") != std::string::npos);
            const int base = 10U;
            auto serviceImportsNum = std::strtol(name.substr(4U).c_str(), nullptr, base);
            if (serviceImportsNum > maxNum) {
                maxNum = serviceImportsNum;
            }
        }
    }

    return "=ens" + std::to_string(maxNum + 1);
}

template <class T>
size_t AddStarExportToModuleLiteralArray(AbckitCoreModule *exporting, AbckitCoreModule *exported, const T *params,
                                         AbckitDynamicExportDescriptorPayload *payload)
{
    auto mPayload = GetDynModulePayload(exporting);
    auto *moduleLitArr = mPayload->moduleLiteralArray->GetDynamicImpl();

    ASSERT((exported->isExternal && exporting->file->externalModules.find(exported->moduleName->impl.data()) !=
                                        exporting->file->externalModules.end()) ||
           (!exported->isExternal && exporting->file->localModules.find(exported->moduleName->impl.data()) !=
                                         exporting->file->localModules.end()));

    auto found = std::find(exporting->md.begin(), exporting->md.end(), exported);
    uint16_t requestIdx;
    if (found != exporting->md.end()) {
        requestIdx = std::distance(exporting->md.begin(), found);
    } else {
        AddNewModuleRequest(exporting, exported);
        requestIdx = exporting->md.size() - 1;
    }

    if (params->alias == nullptr) {
        auto starExportsNum = std::get<uint32_t>(moduleLitArr->literals_[mPayload->starExportsOffset - 1].value_);
        auto literalRequestIdx = pandasm::LiteralArray::Literal {panda_file::LiteralTag::METHODAFFILIATE, requestIdx};
        moduleLitArr->literals_.push_back(std::move(literalRequestIdx));
        moduleLitArr->literals_[mPayload->starExportsOffset - 1].value_ = starExportsNum + 1;
        return starExportsNum;
    }
    auto serviceName = GetServiceExportName(exporting);
    T params1 {};
    params1.name = params->alias;
    params1.alias = serviceName.c_str();
    auto localExpIdx = AddLocalExportToModuleLiteralArray(exporting, nullptr, &params1);
    T params2 {};
    params2.alias = serviceName.c_str();
    auto nsImpIdx = AddNamespaceImportToModuleLiteralArray(exporting, exported, &params2);
    payload->hasServiceImport = true;
    payload->serviceNamespaceImportIdx = nsImpIdx;
    return localExpIdx;
}

template <class T>
size_t AddExportToModuleLiteralArray(AbckitCoreModule *exporting, AbckitCoreModule *exported, const T *params,
                                     AbckitDynamicExportDescriptorPayload *payload, AbckitDynamicExportKind kind)
{
    switch (kind) {
        case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT: {
            return AddStarExportToModuleLiteralArray(exporting, exported, params, payload);
        }
        case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT: {
            return AddLocalExportToModuleLiteralArray(exporting, exported, params);
        }
        case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT: {
            return AddIndirectExportToModuleLiteralArray(exporting, exported, params);
        }
    }
    UNREACHABLE();
}

AbckitArktsExportDescriptor *DynamicModuleAddExportDynamic(AbckitCoreModule *exporting, AbckitCoreModule *exported,
                                                           const AbckitArktsDynamicModuleExportCreateParams *params)
{
    auto ed = std::make_unique<AbckitCoreExportDescriptor>();
    ed->exportingModule = exporting;
    ed->exportedModule = exported;
    auto payloadDyn = AbckitDynamicExportDescriptorPayload();
    if (std::strcmp(params->name, "*") == 0) {
        payloadDyn.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT;
    } else if (exported == nullptr) {
        payloadDyn.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT;
        ed->exportedModule = exporting;
    } else {
        payloadDyn.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT;
    }

    payloadDyn.hasServiceImport = false;
    payloadDyn.moduleRecordIndexOff =
        AddExportToModuleLiteralArray(exporting, exported, params, &payloadDyn, payloadDyn.kind);
    auto payload = AbckitCoreExportDescriptorPayload();
    payload.impl = payloadDyn;
    ed->impl = std::make_unique<AbckitArktsExportDescriptor>();
    ed->GetArkTSImpl()->kind = AbckitImportExportDescriptorKind::UNTYPED;
    ed->GetArkTSImpl()->payload = payload;
    ed->GetArkTSImpl()->core = ed.get();
    exporting->ed.push_back(std::move(ed));

    return exporting->ed[exporting->ed.size() - 1]->GetArkTSImpl();
}

AbckitJsExportDescriptor *DynamicModuleAddExportDynamic(AbckitCoreModule *exporting, AbckitCoreModule *exported,
                                                        const AbckitJsDynamicModuleExportCreateParams *params)
{
    auto ed = std::make_unique<AbckitCoreExportDescriptor>();
    ed->exportingModule = exporting;
    ed->exportedModule = exported;
    auto payloadDyn = AbckitDynamicExportDescriptorPayload();
    if (std::strcmp(params->name, "*") == 0) {
        payloadDyn.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT;
    } else if (exported == nullptr) {
        payloadDyn.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT;
        ed->exportedModule = exporting;
    } else {
        payloadDyn.kind = AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT;
    }

    payloadDyn.hasServiceImport = false;
    payloadDyn.moduleRecordIndexOff =
        AddExportToModuleLiteralArray(exporting, exported, params, &payloadDyn, payloadDyn.kind);
    auto payload = AbckitCoreExportDescriptorPayload();
    payload.impl = payloadDyn;
    ed->impl = std::make_unique<AbckitJsExportDescriptor>();
    ed->GetJSImpl()->payload = payload;
    ed->GetJSImpl()->kind = AbckitImportExportDescriptorKind::UNTYPED;
    ed->GetJSImpl()->core = ed.get();
    exporting->ed.push_back(std::move(ed));
    return exporting->ed[exporting->ed.size() - 1]->GetJSImpl();
}

void ModuleRemoveExportDynamic(AbckitCoreModule *m, AbckitArktsExportDescriptor *i)
{
    LIBABCKIT_LOG_FUNC;
    auto found = std::find_if(m->ed.begin(), m->ed.end(),
                              [&](std::unique_ptr<AbckitCoreExportDescriptor> const &d) { return d.get() == i->core; });
    if (found == m->ed.end()) {
        LIBABCKIT_LOG(DEBUG) << "Can not find the export to delete\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }

    auto idxOffset = 0;
    ;
    auto gap = 0;
    auto mPayload = GetDynModulePayload(m);
    switch (i->payload.GetDynamicPayload().kind) {
        case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT: {
            idxOffset = mPayload->localExportsOffset;
            gap = 2U;
            break;
        }
        case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT: {
            idxOffset = mPayload->indirectExportsOffset;
            gap = 3U;
            break;
        }
        case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT: {
            idxOffset = mPayload->starExportsOffset;
            gap = 1;
        }
    }
    (mPayload->moduleLiteralArray->GetDynamicImpl())
        ->literals_[idxOffset + i->payload.GetDynamicPayload().moduleRecordIndexOff * gap]
        .tag_ = panda_file::LiteralTag::NULLVALUE;

    m->ed.erase(found);
}

void ModuleRemoveExportDynamic(AbckitCoreModule *m, AbckitJsExportDescriptor *i)
{
    LIBABCKIT_LOG_FUNC;
    auto found = std::find_if(m->ed.begin(), m->ed.end(),
                              [&](std::unique_ptr<AbckitCoreExportDescriptor> const &d) { return d.get() == i->core; });
    if (found == m->ed.end()) {
        LIBABCKIT_LOG(DEBUG) << "Can not find the export to delete\n";
        statuses::SetLastError(AbckitStatus::ABCKIT_STATUS_BAD_ARGUMENT);
        return;
    }

    auto idxOffset = 0U;
    ;
    auto gap = 0U;
    auto mPayload = GetDynModulePayload(m);
    switch (i->payload.GetDynamicPayload().kind) {
        case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT: {
            idxOffset = mPayload->localExportsOffset;
            gap = 2U;
            break;
        }
        case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_INDIRECT_EXPORT: {
            idxOffset = mPayload->indirectExportsOffset;
            gap = 3U;
            break;
        }
        case AbckitDynamicExportKind::ABCKIT_DYNAMIC_EXPORT_KIND_STAR_EXPORT: {
            idxOffset = mPayload->starExportsOffset;
            gap = 1U;
        }
    }
    (mPayload->moduleLiteralArray->GetDynamicImpl())
        ->literals_[idxOffset + i->payload.GetDynamicPayload().moduleRecordIndexOff * gap]
        .tag_ = panda_file::LiteralTag::NULLVALUE;

    m->ed.erase(found);
}

AbckitLiteral *CreateLiteralBoolDynamic(AbckitFile *file, bool value)
{
    LIBABCKIT_LOG_FUNC;
    return GetOrCreateLiteralBoolDynamic(file, value);
}

AbckitLiteral *CreateLiteralU8Dynamic(AbckitFile *file, uint8_t value)
{
    LIBABCKIT_LOG_FUNC;
    return GetOrCreateLiteralU8Dynamic(file, value);
}

AbckitLiteral *CreateLiteralU16Dynamic(AbckitFile *file, uint16_t value)
{
    LIBABCKIT_LOG_FUNC;
    return GetOrCreateLiteralU16Dynamic(file, value);
}

AbckitLiteral *CreateLiteralMethodAffiliateDynamic(AbckitFile *file, uint16_t value)
{
    LIBABCKIT_LOG_FUNC;
    return GetOrCreateLiteralMethodAffiliateDynamic(file, value);
}

AbckitLiteral *CreateLiteralU32Dynamic(AbckitFile *file, uint32_t value)
{
    LIBABCKIT_LOG_FUNC;
    return GetOrCreateLiteralU32Dynamic(file, value);
}

AbckitLiteral *CreateLiteralU64Dynamic(AbckitFile *file, uint64_t value)
{
    LIBABCKIT_LOG_FUNC;
    return GetOrCreateLiteralU64Dynamic(file, value);
}

AbckitLiteral *CreateLiteralFloatDynamic(AbckitFile *file, float value)
{
    LIBABCKIT_LOG_FUNC;
    return GetOrCreateLiteralFloatDynamic(file, value);
}

AbckitLiteral *CreateLiteralDoubleDynamic(AbckitFile *file, double value)
{
    LIBABCKIT_LOG_FUNC;
    return GetOrCreateLiteralDoubleDynamic(file, value);
}

AbckitLiteral *CreateLiteralStringDynamic(AbckitFile *file, const char *value)
{
    LIBABCKIT_LOG_FUNC;
    return GetOrCreateLiteralStringDynamic(file, value);
}

AbckitLiteral *CreateLiteralMethodDynamic(AbckitFile *file, AbckitCoreFunction *function)
{
    LIBABCKIT_LOG_FUNC;
    return GetOrCreateLiteralMethodDynamic(file, GetDynFunction(function)->name);
}

AbckitLiteral *CreateLiteralLiteralArrayDynamic(AbckitFile *file, AbckitLiteralArray *litarr)
{
    LIBABCKIT_LOG_FUNC;
    std::string arrName;
    for (auto &item : file->GetDynamicProgram()->literalarray_table) {
        if (&item.second == litarr->GetDynamicImpl()) {
            arrName = item.first;
            break;
        }
    }
    return GetOrCreateLiteralLiteralArrayDynamic(file, arrName);
}

AbckitLiteralArray *CreateLiteralArrayDynamic(AbckitFile *file, AbckitLiteral **value, size_t size)
{
    LIBABCKIT_LOG_FUNC;
    auto *prog = file->GetDynamicProgram();

    std::vector<pandasm::LiteralArray::Literal> stdLitArr;
    for (size_t i = 0; i < size; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto lit = *reinterpret_cast<pandasm::LiteralArray::Literal *>(value[i]->val.get());
        pandasm::LiteralArray::Literal tagLit;
        tagLit.tag_ = panda_file::LiteralTag::TAGVALUE;
        tagLit.value_ = static_cast<uint8_t>(lit.tag_);
        stdLitArr.emplace_back(tagLit);
        stdLitArr.emplace_back(lit);
    }

    // NOLINTNEXTLINE(cert-msc51-cpp)
    uint32_t arrayOffset = 0;
    while (prog->literalarray_table.find(std::to_string(arrayOffset)) != prog->literalarray_table.end()) {
        LIBABCKIT_LOG(DEBUG) << "generating new arrayOffset\n";
        arrayOffset = std::stoi(prog->literalarray_table.rbegin()->first) + 1;
    }
    auto arrayName = std::to_string(arrayOffset);

    prog->literalarray_table.emplace(arrayName, pandasm::LiteralArray());
    pandasm::LiteralArray &arrImpl = prog->literalarray_table[arrayName];
    arrImpl.literals_ = std::move(stdLitArr);

    auto litarr = std::make_unique<AbckitLiteralArray>(file, &arrImpl);
    return file->litarrs.emplace_back(std::move(litarr)).get();
}

AbckitValue *CreateValueU1Dynamic(AbckitFile *file, bool value)
{
    LIBABCKIT_LOG_FUNC;
    auto *pval = new pandasm::ScalarValue(
        pandasm::ScalarValue::Create<panda::pandasm::Value::Type::U1>(static_cast<uint8_t>(value)));
    auto abcval = std::make_unique<AbckitValue>(file, pval);
    file->values.emplace_back(std::move(abcval));
    return file->values.back().get();
}

AbckitValue *CreateValueDoubleDynamic(AbckitFile *file, double value)
{
    LIBABCKIT_LOG_FUNC;
    auto *pval = new pandasm::ScalarValue(pandasm::ScalarValue::Create<panda::pandasm::Value::Type::F64>(value));
    auto abcval = std::make_unique<AbckitValue>(file, pval);
    file->values.emplace_back(std::move(abcval));
    return file->values.back().get();
}

AbckitValue *CreateValueStringDynamic(AbckitFile *file, const char *value)
{
    LIBABCKIT_LOG_FUNC;
    auto *pval =
        new pandasm::ScalarValue(pandasm::ScalarValue::Create<panda::pandasm::Value::Type::STRING>(std::string(value)));
    auto abcval = std::make_unique<AbckitValue>(file, pval);
    file->values.emplace_back(std::move(abcval));
    return file->values.back().get();
}

AbckitValue *CreateLiteralArrayValueDynamic(AbckitFile *file, AbckitValue **value, size_t size)
{
    LIBABCKIT_LOG_FUNC;
    std::vector<AbckitLiteral *> litArr;
    for (size_t i = 0; i < size; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        switch (ValueGetTypeDynamic(value[i])->id) {
            case ABCKIT_TYPE_ID_U1:
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                litArr.emplace_back(CreateLiteralBoolDynamic(file, ValueGetU1Dynamic(value[i])));
                break;
            case ABCKIT_TYPE_ID_F64:
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                litArr.emplace_back(CreateLiteralDoubleDynamic(file, ValueGetDoubleDynamic(value[i])));
                break;
            case ABCKIT_TYPE_ID_STRING:
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                litArr.emplace_back(CreateLiteralStringDynamic(file, ValueGetStringDynamic(value[i])->impl.data()));
                break;
            default:
                break;
        }
    }
    auto *abcArr = CreateLiteralArrayDynamic(file, litArr.data(), litArr.size());
    auto *prog = file->GetDynamicProgram();
    std::string arrName;
    for (auto &item : prog->literalarray_table) {
        if (&item.second == abcArr->GetDynamicImpl()) {
            arrName = item.first;
            break;
        }
    }
    auto *pval =
        new pandasm::ScalarValue(pandasm::ScalarValue::Create<panda::pandasm::Value::Type::LITERALARRAY>(arrName));
    auto abcval = std::make_unique<AbckitValue>(file, pval);
    file->values.emplace_back(std::move(abcval));
    return file->values.back().get();
}

AbckitArktsAnnotationInterface *ModuleAddAnnotationInterfaceDynamic(
    AbckitCoreModule *m, const AbckitArktsAnnotationInterfaceCreateParams *params)
{
    std::string name = params->name;
    auto prog = m->file->GetDynamicProgram();
    auto progRecord = pandasm::Record {name, prog->lang};
    progRecord.metadata->SetAccessFlags(ACC_ANNOTATION);
    prog->record_table.emplace(name, std::move(progRecord));
    pandasm::Record &record = prog->record_table.find(name)->second;
    auto ai = std::make_unique<AbckitCoreAnnotationInterface>();
    ai->m = m;
    ai->impl = std::make_unique<AbckitArktsAnnotationInterface>();
    ai->GetArkTSImpl()->impl = &record;
    ai->GetArkTSImpl()->core = ai.get();
    return m->at.emplace(name, std::move(ai)).first->second->GetArkTSImpl();
}

AbckitArktsAnnotation *ClassAddAnnotationDynamic(AbckitCoreClass *klass,
                                                 const AbckitArktsAnnotationCreateParams *params)
{
    auto ai = params->ai;

    auto progFunc = klass->GetArkTSImpl()->impl.GetDynamicClass();
    auto name = ai->GetDynamicImpl()->name;

    std::vector<pandasm::AnnotationData> vec;
    vec.emplace_back(name);

    progFunc->metadata->AddAnnotations(vec);

    auto anno = std::make_unique<AbckitCoreAnnotation>();
    anno->owner = klass;
    anno->name = AnnotationInterfaceGetNameDynamic(ai->core);
    anno->ai = ai->core;
    anno->impl = std::make_unique<AbckitArktsAnnotation>();
    anno->GetArkTSImpl()->core = anno.get();
    return klass->annotations.emplace_back(std::move(anno))->GetArkTSImpl();
}

void ClassRemoveAnnotationDynamic(AbckitCoreClass *klass, AbckitCoreAnnotation *anno)
{
    auto progFunc = klass->GetArkTSImpl()->impl.GetDynamicClass();

    auto ai = anno->ai;

    auto str = AnnotationInterfaceGetNameDynamic(ai);
    auto name = str->impl;

    progFunc->metadata->DeleteAnnotationByName(name);
    auto &annotations = klass->annotations;
    auto iter = std::find_if(annotations.begin(), annotations.end(),
                             [&name](auto &annoIt) { return name == annoIt.get()->name->impl; });
    annotations.erase(iter);
}

AbckitArktsAnnotation *FunctionAddAnnotationDynamic(AbckitCoreFunction *function,
                                                    const AbckitArktsAnnotationCreateParams *params)
{
    auto ai = params->ai;

    auto progFunc = function->GetArkTSImpl()->GetDynamicImpl();
    auto name = ai->GetDynamicImpl()->name;

    std::vector<pandasm::AnnotationData> vec;
    vec.emplace_back(name);

    progFunc->metadata->AddAnnotations(vec);

    auto anno = std::make_unique<AbckitCoreAnnotation>();
    anno->owner = function;
    anno->name = AnnotationInterfaceGetNameDynamic(ai->core);
    anno->ai = ai->core;
    anno->impl = std::make_unique<AbckitArktsAnnotation>();
    anno->GetArkTSImpl()->core = anno.get();
    return function->annotations.emplace_back(std::move(anno))->GetArkTSImpl();
}

void FunctionRemoveAnnotationDynamic(AbckitCoreFunction *function, AbckitCoreAnnotation *anno)
{
    auto progFunc = function->GetArkTSImpl()->GetDynamicImpl();

    auto ai = anno->ai;

    auto str = AnnotationInterfaceGetNameDynamic(ai);
    auto name = str->impl;

    progFunc->metadata->DeleteAnnotationByName(name);
    auto &annotations = function->annotations;
    auto iter = std::find_if(annotations.begin(), annotations.end(),
                             [&name](auto &annoIt) { return name == annoIt.get()->name->impl; });
    annotations.erase(iter);
}

AbckitArktsAnnotationElement *AnnotationAddAnnotationElementDynamic(AbckitCoreAnnotation *anno,
                                                                    AbckitArktsAnnotationElementCreateParams *params)
{
    auto valuePtr = params->value;
    auto name = params->name;

    auto value = *valuePtr;
    auto pandasmValue = *value.GetDynamicImpl();
    auto abcValue = std::make_unique<AbckitValue>(value);

    pandasm::AnnotationElement progAnnoElem(params->name, std::make_unique<pandasm::Value>(std::move(pandasmValue)));
    abcValue->impl = progAnnoElem.GetValue();

    auto annoElem = std::make_unique<AbckitCoreAnnotationElement>();
    annoElem->ann = anno;
    annoElem->name = CreateStringDynamic(anno->ai->m->file, progAnnoElem.GetName().data());
    annoElem->value = std::move(abcValue);

    if (std::holds_alternative<AbckitCoreFunction *>(anno->owner)) {
        auto *func = std::get<AbckitCoreFunction *>(anno->owner);
        auto progOwner = func->GetArkTSImpl()->GetDynamicImpl();
        progOwner->metadata->AddAnnotationElementByName(name, std::move(progAnnoElem));
        annoElem->value->file = func->m->file;
    } else if (std::holds_alternative<AbckitCoreClass *>(anno->owner)) {
        auto *klass = std::get<AbckitCoreClass *>(anno->owner);
        auto progOwner = klass->GetArkTSImpl()->impl.GetDynamicClass();
        progOwner->metadata->AddAnnotationElementByName(name, std::move(progAnnoElem));
        annoElem->value->file = klass->m->file;
    }

    annoElem->impl = std::make_unique<AbckitArktsAnnotationElement>();
    annoElem->GetArkTSImpl()->core = annoElem.get();

    return anno->elements.emplace_back(std::move(annoElem))->GetArkTSImpl();
}

void AnnotationRemoveAnnotationElementDynamic(AbckitCoreAnnotation *anno, AbckitCoreAnnotationElement *elem)
{
    auto name = elem->name;

    if (std::holds_alternative<AbckitCoreFunction *>(anno->owner)) {
        auto progOwner = std::get<AbckitCoreFunction *>(anno->owner)->GetArkTSImpl()->GetDynamicImpl();
        progOwner->metadata->DeleteAnnotationElementByName(anno->name->impl, name->impl);
    } else if (std::holds_alternative<AbckitCoreClass *>(anno->owner)) {
        auto progOwner = std::get<AbckitCoreClass *>(anno->owner)->GetArkTSImpl()->impl.GetDynamicClass();
        progOwner->metadata->DeleteAnnotationElementByName(anno->name->impl, name->impl);
    }

    auto &annotationElementes = anno->elements;
    auto iter = std::find_if(annotationElementes.begin(), annotationElementes.end(),
                             [&name](auto &annoElemIt) { return name->impl == annoElemIt.get()->name->impl; });
    if (iter == annotationElementes.end()) {
        statuses::SetLastError(ABCKIT_STATUS_TODO);
        return;
    }
    annotationElementes.erase(iter);
}

std::string TypeToName(AbckitType *type)
{
    if (type->id == ABCKIT_TYPE_ID_REFERENCE) {
        auto str = ClassGetNameDynamic(type->klass);
        if (str == nullptr) {
            return "";
        }
        return std::string(str->impl);
    }
    switch (type->id) {
        case ABCKIT_TYPE_ID_VOID:
            return "void";
        case ABCKIT_TYPE_ID_U1:
            return "u1";
        case ABCKIT_TYPE_ID_U8:
            return "u8";
        case ABCKIT_TYPE_ID_I8:
            return "u8";
        case ABCKIT_TYPE_ID_U16:
            return "u16";
        case ABCKIT_TYPE_ID_I16:
            return "i16";
        case ABCKIT_TYPE_ID_U32:
            return "u32";
        case ABCKIT_TYPE_ID_I32:
            return "i32";
        case ABCKIT_TYPE_ID_U64:
            return "u64";
        case ABCKIT_TYPE_ID_I64:
            return "i64";
        case ABCKIT_TYPE_ID_F32:
            return "f32";
        case ABCKIT_TYPE_ID_F64:
            return "f64";
        case ABCKIT_TYPE_ID_ANY:
            return "any";
        case ABCKIT_TYPE_ID_STRING:
            return "panda.String";
        default:
            return "invalid";
    }
}

AbckitArktsAnnotationInterfaceField *AnnotationInterfaceAddFieldDynamic(
    AbckitCoreAnnotationInterface *ai, const AbckitArktsAnnotationInterfaceFieldCreateParams *params)
{
    auto name = params->name;
    auto type = params->type;
    auto value = params->defaultValue;

    auto field = std::make_unique<AbckitCoreAnnotationInterfaceField>();
    field->ai = ai;
    field->name = CreateStringDynamic(ai->m->file, name);
    field->type = type;
    field->value = value;

    field->impl = std::make_unique<AbckitArktsAnnotationInterfaceField>();
    field->GetArkTSImpl()->core = field.get();
    ai->fields.emplace_back(std::move(field));

    auto record = ai->GetArkTSImpl()->GetDynamicImpl();

    auto prog = ai->m->file->GetDynamicProgram();
    auto progField = pandasm::Field(prog->lang);

    progField.name = name;
    std::string typeName;
    progField.type = pandasm::Type(TypeToName(type), type->rank);
    progField.metadata->SetValue(*static_cast<pandasm::ScalarValue *>(value->GetDynamicImpl()));
    record->field_list.emplace_back(std::move(progField));

    return ai->fields.back()->GetArkTSImpl();
}

void AnnotationInterfaceRemoveFieldDynamic(AbckitCoreAnnotationInterface *ai, AbckitCoreAnnotationInterfaceField *field)
{
    auto str = field->name;
    if (str == nullptr) {
        statuses::SetLastError(ABCKIT_STATUS_TODO);
        return;
    }
    auto name = str->impl;

    auto record = ai->GetArkTSImpl()->GetDynamicImpl();

    auto &fields = record->field_list;

    auto fieldsIter = std::find_if(fields.begin(), fields.end(), [&name](auto &field) { return name == field.name; });
    if (fieldsIter == fields.end()) {
        statuses::SetLastError(ABCKIT_STATUS_TODO);
        return;
    }
    fields.erase(fieldsIter);

    auto &aiFields = ai->fields;
    auto iter = std::find_if(aiFields.begin(), aiFields.end(),
                             [&name](auto &field) { return name == field.get()->name->impl; });
    if (iter == aiFields.end()) {
        statuses::SetLastError(ABCKIT_STATUS_TODO);
        return;
    }
    aiFields.erase(iter);
}

}  // namespace libabckit
