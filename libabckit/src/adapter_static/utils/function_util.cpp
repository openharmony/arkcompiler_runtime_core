/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "function_util.h"
#include "libabckit/c/statuses.h"
#include "libabckit/src/macros.h"
#include "libabckit/src/adapter_static/metadata_inspect_static.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "static_core/assembler/assembly-program.h"
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

namespace abckit::util {

std::string ReplaceFunctionModuleName(const std::string &oldModuleName, const std::string &newName)
{
    size_t pos = oldModuleName.rfind('.');
    if (pos == std::string::npos) {
        libabckit::statuses::SetLastError(ABCKIT_STATUS_INTERNAL_ERROR);
        return newName;
    }
    return oldModuleName.substr(0, pos + 1) + newName;
}

std::string GenerateFunctionMangleName(const std::string &moduleName, const ark::pandasm::Function &func)
{
    std::ostringstream ss;
    ss << moduleName << ":";

    for (const auto &param : func.params) {
        ss << param.type.GetName() << ";";
    }
    ss << func.returnType.GetName() << ";";
    return ss.str();
}

bool UpdateFunctionTableKey(ark::pandasm::Program *prog, ark::pandasm::Function *impl, const std::string &newName,
                            std::string &oldMangleName, std::string &newMangleName)
{
    if (newMangleName.empty()) {
        std::string newModuleName = ReplaceFunctionModuleName(impl->name, newName);
        newMangleName = GenerateFunctionMangleName(newModuleName, *impl);
    }

    auto staticIt = prog->functionStaticTable.find(oldMangleName);
    if (staticIt != prog->functionStaticTable.end()) {
        auto staticNode = prog->functionStaticTable.extract(staticIt);
        staticNode.key() = newMangleName;
        prog->functionStaticTable.insert(std::move(staticNode));
    }

    auto instanceIt = prog->functionInstanceTable.find(oldMangleName);
    if (instanceIt != prog->functionInstanceTable.end()) {
        auto instanceNode = prog->functionInstanceTable.extract(instanceIt);
        instanceNode.key() = newMangleName;
        prog->functionInstanceTable.insert(std::move(instanceNode));
    }

    if (staticIt == prog->functionStaticTable.end() && instanceIt == prog->functionInstanceTable.end()) {
        LIBABCKIT_LOG(FATAL) << "Function table key not found: " << oldMangleName << '\n';
        return false;
    }

    return true;
}

static void ReplaceIdsInInsList(std::vector<ark::pandasm::Ins> &insList, const std::string &oldId,
                                const std::string &newId)
{
    for (auto &insn : insList) {
        for (std::size_t i = 0; i < insn.IDSize(); ++i) {
            if (insn.GetID(i) == oldId) {
                insn.SetID(i, newId);
            }
        }
    }
}

static void ReplaceIdsInFunctionTable(std::map<std::string, ark::pandasm::Function> &functionTable,
                                      const std::string &oldId, const std::string &newId)
{
    for (auto &[key, func] : functionTable) {
        ReplaceIdsInInsList(func.ins, oldId, newId);
    }
}

void ReplaceInstructionIds(ark::pandasm::Program *prog, ark::pandasm::Function *impl, const std::string &oldId,
                           const std::string &newId)
{
    ReplaceIdsInInsList(impl->ins, oldId, newId);
    ReplaceIdsInFunctionTable(prog->functionStaticTable, oldId, newId);
    ReplaceIdsInFunctionTable(prog->functionInstanceTable, oldId, newId);
}

static std::string GetReferenceTypeName(AbckitCoreFunction *coreFunc, const AbckitType *type)
{
    AbckitString *classNameStr = libabckit::ClassGetNameStatic(type->GetClass());
    if (classNameStr == nullptr) {
        LIBABCKIT_LOG(ERROR) << "[Error] Class name is null" << std::endl;
        return {};
    }
    std::string className(classNameStr->impl);
    std::string moduleName(coreFunc->owningModule->moduleName->impl);
    return moduleName + "." + className;
}

static std::string GetPrimitiveTypeName(AbckitTypeId typeId, bool useComponentFormat)
{
    static const std::map<AbckitTypeId, std::pair<std::string, std::string>> TYPE_MAP = {
        {ABCKIT_TYPE_ID_STRING, {"std.core.String", "std.core.String"}},
        {ABCKIT_TYPE_ID_U1, {"std.core.u1", "u1"}},
        {ABCKIT_TYPE_ID_U8, {"std.core.u8", "u8"}},
        {ABCKIT_TYPE_ID_I8, {"std.core.i8", "i8"}},
        {ABCKIT_TYPE_ID_U16, {"std.core.u16", "u16"}},
        {ABCKIT_TYPE_ID_I16, {"std.core.i16", "i16"}},
        {ABCKIT_TYPE_ID_U32, {"std.core.u32", "u32"}},
        {ABCKIT_TYPE_ID_I32, {"std.core.i32", "i32"}},
        {ABCKIT_TYPE_ID_U64, {"std.core.u64", "u64"}},
        {ABCKIT_TYPE_ID_I64, {"std.core.i64", "i64"}},
        {ABCKIT_TYPE_ID_F32, {"std.core.f32", "f32"}},
        {ABCKIT_TYPE_ID_F64, {"std.core.f64", "f64"}},
        {ABCKIT_TYPE_ID_ANY, {"std.core.any", "any"}},
        {ABCKIT_TYPE_ID_VOID, {"void", "void"}}};

    auto it = TYPE_MAP.find(typeId);
    if (it != TYPE_MAP.end()) {
        return useComponentFormat ? it->second.first : it->second.second;
    }
    LIBABCKIT_LOG(ERROR) << "[Error] Invalid type id: " << typeId << std::endl;
    return "invalid";
}

std::string GetTypeName(AbckitCoreFunction *coreFunc, const AbckitType *type, bool useComponentFormat)
{
    if (type->id == ABCKIT_TYPE_ID_REFERENCE) {
        return GetReferenceTypeName(coreFunc, type);
    }
    return GetPrimitiveTypeName(type->id, useComponentFormat);
}

void AddFunctionParameterImpl(AbckitCoreFunction *coreFunc, ark::pandasm::Function *funcImpl,
                              const AbckitCoreFunctionParam *paramCore, const std::string &componentName)
{
    ark::pandasm::Type type(componentName, paramCore->type->rank);
    ark::pandasm::Function::Parameter pandaParam(type, ark::panda_file::SourceLang::ETS);
    funcImpl->params.push_back(std::move(pandaParam));

    auto paramHolder = std::make_unique<AbckitCoreFunctionParam>();
    paramHolder->function = coreFunc;
    paramHolder->type = paramCore->type;
    paramHolder->name = paramCore->name;

    auto arktsParam = std::make_unique<AbckitArktsFunctionParam>();
    arktsParam->core = paramHolder.get();
    paramHolder->impl = std::move(arktsParam);

    coreFunc->parameters.emplace_back(std::move(paramHolder));
}

bool RemoveFunctionParameterByIndexImpl([[maybe_unused]] AbckitCoreFunction *coreFunc, ark::pandasm::Function *impl,
                                        size_t index)
{
    if (index >= impl->params.size()) {
        LIBABCKIT_LOG(ERROR) << "Index " << index << " exceeds impl->params.size(): " << impl->params.size()
                             << std::endl;
        return false;
    }

    impl->params.erase(impl->params.begin() + index);
    return true;
}

bool UpdateFileMap(AbckitFile *file, const std::string &oldMangleName, const std::string &newMangleName)
{
    if (file == nullptr) {
        LIBABCKIT_LOG(ERROR) << "File is null" << std::endl;
        return false;
    }
    bool found = false;

    auto staticIt = file->nameToFunctionStatic.find(oldMangleName);
    if (staticIt != file->nameToFunctionStatic.end()) {
        auto staticNode = file->nameToFunctionStatic.extract(staticIt);
        staticNode.key() = newMangleName;
        file->nameToFunctionStatic.insert(std::move(staticNode));
        found = true;
    }

    auto instanceIt = file->nameToFunctionInstance.find(oldMangleName);
    if (instanceIt != file->nameToFunctionInstance.end()) {
        auto instanceNode = file->nameToFunctionInstance.extract(instanceIt);
        instanceNode.key() = newMangleName;
        file->nameToFunctionInstance.insert(std::move(instanceNode));
        found = true;
    }

    if (!found) {
        LIBABCKIT_LOG(ERROR) << "Function mangle name not found in file maps: " << oldMangleName << std::endl;
        return false;
    }

    return true;
}
}  // namespace abckit::util