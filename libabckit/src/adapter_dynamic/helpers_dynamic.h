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

#ifndef LIBABCKIT_SRC_ADAPTER_DYNAMIC_HELPERS_DYNAMIC_H
#define LIBABCKIT_SRC_ADAPTER_DYNAMIC_HELPERS_DYNAMIC_H

#include <string>

#include "libabckit/include/c/abckit.h"
#include "libabckit/src/metadata_inspect_impl.h"
#include "assembler/assembly-emitter.h"
#include "assembler/assembly-literals.h"

namespace libabckit {

struct ModuleUpdateData {
    bool isRegularImportsChange = false;
    bool isLocalExportsChange = false;
    std::vector<panda::pandasm::LiteralArray::Literal> newLiterals {};
    std::unordered_map<uint32_t, uint32_t> regularImportsIdxMap {};
    std::unordered_map<uint32_t, uint32_t> localExportsIdxMap {};
};

struct ModuleIterateData {
    AbckitCoreModule *m = nullptr;
    AbckitModulePayloadDyn *payload = nullptr;
    panda::pandasm::LiteralArray *moduleLitArr = nullptr;
    std::pair<bool, bool> isRegularImport {};  // {isImport, isRegular}
    AbckitDynamicExportKind kind = ABCKIT_DYNAMIC_EXPORT_KIND_LOCAL_EXPORT;
    libabckit::ModuleUpdateData updateData {};
};

const panda::panda_file::File *EmitDynamicProgram(AbckitFile *file, panda::pandasm::Program *program,
                                                  panda::pandasm::AsmEmitter::PandaFileToPandaAsmMaps *mapsp,
                                                  bool getFile, const char *path = nullptr);
bool IterateModuleSections(
    ModuleIterateData &data, const std::function<size_t(ModuleIterateData *)> &requestIdxSectionModifier,
    const std::function<std::pair<size_t, size_t>(ModuleIterateData *, size_t, size_t)> &sectionModifier,
    const std::function<bool(ModuleIterateData *, std::pair<size_t, size_t>)> &postAction);
void DumpModuleArray(const panda::pandasm::LiteralArray *moduleLitArr, std::string_view name);

std::string Relative(const std::string &src, const std::string &base);

bool IsServiceRecord(const std::string &name);
bool IsAnnotationInterfaceRecord(const panda::pandasm::Record &rec);
bool IsExternalRecord(const panda::pandasm::Record &rec);

bool IsMain(const std::string &funcName);
bool IsFunction(const std::string &funcName);
bool IsCtor(const std::string &funcName);
bool IsStatic(const std::string &funcName);
bool IsAnonymous(const std::string &funcName);

panda::pandasm::Function *GetDynFunction(AbckitCoreFunction *function);
panda::pandasm::Function *GetDynFunction(AbckitCoreClass *klass);
AbckitModulePayloadDyn *GetDynModulePayload(AbckitCoreModule *mod);
AbckitDynamicImportDescriptorPayload *GetDynImportDescriptorPayload(AbckitCoreImportDescriptor *id);
AbckitDynamicExportDescriptorPayload *GetDynExportDescriptorPayload(AbckitCoreExportDescriptor *ed);

std::string GetClassNameFromCtor(const std::string &ctorName, const AbckitLiteralArray *scopeNames);

AbckitLiteral *GetOrCreateLiteralBoolDynamic(AbckitFile *file, bool value);
AbckitLiteral *GetOrCreateLiteralU8Dynamic(AbckitFile *file, uint8_t value);
AbckitLiteral *GetOrCreateLiteralU16Dynamic(AbckitFile *file, uint16_t value);
AbckitLiteral *GetOrCreateLiteralMethodAffiliateDynamic(AbckitFile *file, uint16_t value);
AbckitLiteral *GetOrCreateLiteralU32Dynamic(AbckitFile *file, uint32_t value);
AbckitLiteral *GetOrCreateLiteralU64Dynamic(AbckitFile *file, uint64_t value);
AbckitLiteral *GetOrCreateLiteralFloatDynamic(AbckitFile *file, float value);
AbckitLiteral *GetOrCreateLiteralDoubleDynamic(AbckitFile *file, double value);
AbckitLiteral *GetOrCreateLiteralLiteralArrayDynamic(AbckitFile *file, const std::string &value);
AbckitLiteral *GetOrCreateLiteralStringDynamic(AbckitFile *file, const std::string &value);
AbckitLiteral *GetOrCreateLiteralMethodDynamic(AbckitFile *file, const std::string &value);
AbckitLiteral *GetOrCreateLiteralDynamic(AbckitFile *file, const panda::pandasm::LiteralArray::Literal &value);

}  // namespace libabckit

#endif
