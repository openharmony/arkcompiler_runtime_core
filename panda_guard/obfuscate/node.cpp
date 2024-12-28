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

#include "node.h"

#include "utils/logger.h"

#include "configs/guard_context.h"
#include "graph_analyzer.h"
#include "program.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace {
using OpcodeList = std::vector<panda::pandasm::Opcode>;

constexpr std::string_view TAG = "[Node]";
constexpr std::string_view ENTRY_FUNC_NAME = ".func_main_0";
constexpr std::string_view NORMALIZED_OHM_DELIMITER = "&";
constexpr std::string_view PATH_DELIMITER = "/";
constexpr std::string_view PACKAGE_MODULES_PREFIX = "pkg_modules";
constexpr std::string_view PKG_NAME_PREFIX = "pkgName@";
constexpr std::string_view SCOPE_NAMES_FIELD = "scopeNames";
constexpr std::string_view JSON_FILE_FIELD = "jsonFileContent";

const OpcodeList METHOD_NAME_DIRECT_LIST = {
    panda::pandasm::Opcode::STOWNBYNAME,
    panda::pandasm::Opcode::STOWNBYNAMEWITHNAMESET,
};

const OpcodeList METHOD_NAME_INDIRECT_LIST = {
    panda::pandasm::Opcode::DEFINEGETTERSETTERBYVALUE,
    panda::pandasm::Opcode::STOWNBYVALUE,
    panda::pandasm::Opcode::STOWNBYVALUEWITHNAMESET,
};

bool InOpcodeList(const panda::guard::InstructionInfo &info, const OpcodeList &list)
{
    return std::any_of(list.begin(), list.end(), [&](const auto &elem) { return elem == info.ins_->opcode; });
}

void UpdateScopeNamesLiteralArray(panda::pandasm::LiteralArray &literalArray)
{
    for (auto &literal : literalArray.literals_) {
        if (!literal.IsStringValue()) {
            continue;
        }

        const auto &value = std::get<std::string>(literal.value_);
        literal.value_ = panda::guard::GuardContext::GetInstance()->GetNameMapping()->GetName(value);
    }
}

}  // namespace

void panda::guard::Node::Build()
{
    LOG(INFO, PANDAGUARD) << TAG << "node create for " << this->name_ << " start";

    this->sourceName_ = GuardContext::GetInstance()->GetGuardOptions()->GetSourceName(this->name_);
    this->obfSourceName_ = this->sourceName_;
    LOG(INFO, PANDAGUARD) << TAG << "node sourceName_ " << this->sourceName_;

    this->isNormalizedOhmUrl_ = GuardContext::GetInstance()->GetGuardOptions()->IsUseNormalizedOhmUrl();
    CreateFilePath();
    this->filepath_.obfName = this->filepath_.name;
    LOG(INFO, PANDAGUARD) << TAG << "pre part: " << this->filepath_.prePart;
    LOG(INFO, PANDAGUARD) << TAG << "file path: " << this->filepath_.name;
    LOG(INFO, PANDAGUARD) << TAG << "post part: " << this->filepath_.postPart;

    moduleRecord_.Create();
    if (!Node::IsJsonFile(this->program_->prog_->record_table.at(this->name_))) {
        Function entryFunc(this->program_, this->name_ + ENTRY_FUNC_NAME.data(), false);
        entryFunc.Init();
        entryFunc.Create();

        const auto &function = entryFunc.GetOriginFunction();
        this->sourceFile_ = function.source_file;
        this->obfSourceFile_ = function.source_file;

        this->functionTable_.emplace(entryFunc.idx_, entryFunc);

        entryFunc.ForEachIns([&](const InstructionInfo &info) -> void { ForEachIns(info, TOP_LEVEL); });
    } else {
        LOG(INFO, PANDAGUARD) << "json file record:" << this->name_;
    }

    this->ExtractNames();

    LOG(INFO, PANDAGUARD) << TAG << "node create for " << this->name_ << " end";
}

void panda::guard::Node::ForEachIns(const panda::guard::InstructionInfo &info, panda::guard::Scope scope)
{
    CreateFunction(info, scope);
    CreateClass(info, scope);
    CreateOuterMethod(info);
    CreateObject(info, scope);
    CreateObjectOuterProperty(info);
    CreateOuterProperty(info);
    FindStLexVarName(info);
}

void panda::guard::Node::CreateFunction(const InstructionInfo &info, const Scope scope)
{
    if (info.ins_->opcode != pandasm::Opcode::DEFINEFUNC) {
        return;
    }

    const std::string idx = info.ins_->ids[0];
    if (this->functionTable_.find(idx) != this->functionTable_.end()) {
        this->functionTable_.at(idx).defineInsList_.push_back(info);
        return;
    }

    Function function(this->program_, idx);
    function.scope_ = scope;
    function.defineInsList_.push_back(info);

    function.Init();

    function.export_ = this->moduleRecord_.IsExportVar(function.name_);
    function.Create();

    function.ForEachIns([&](const InstructionInfo &insInfo) -> void { ForEachIns(insInfo, FUNCTION); });

    this->functionTable_.emplace(function.idx_, function);
}

void panda::guard::Node::CreateClass(const InstructionInfo &info, const Scope scope)
{
    if ((info.ins_->opcode != pandasm::Opcode::DEFINECLASSWITHBUFFER) &&
        (info.ins_->opcode != pandasm::Opcode::CALLRUNTIME_DEFINESENDABLECLASS)) {
        return;
    }

    const std::string idx = info.ins_->ids[1];
    if (this->classTable_.find(idx) != this->classTable_.end()) {
        this->classTable_.at(idx).defineInsList_.push_back(info);
        return;
    }

    Class clazz(this->program_, info.ins_->ids[0]);
    clazz.moduleRecord_ = &this->moduleRecord_;
    clazz.literalArrayIdx_ = idx;
    clazz.defineInsList_.push_back(info);
    clazz.component = GraphAnalyzer::IsComponentClass(info);
    clazz.scope_ = scope;
    if (info.ins_->opcode == pandasm::Opcode::CALLRUNTIME_DEFINESENDABLECLASS) {
        clazz.callRunTimeInst_ = true;
    }
    clazz.Create();

    clazz.ForEachMethodIns([&](const InstructionInfo &insInfo) -> void { ForEachIns(insInfo, FUNCTION); });

    this->classTable_.emplace(clazz.literalArrayIdx_, clazz);
}

void panda::guard::Node::CreateOuterMethod(const InstructionInfo &info)
{
    if (info.ins_->opcode != pandasm::Opcode::DEFINEMETHOD) {
        return;
    }

    InstructionInfo defineInsInfo;
    InstructionInfo nameInsInfo;
    GraphAnalyzer::HandleDefineMethod(info, defineInsInfo, nameInsInfo);

    const std::string methodIdx = info.ins_->ids[0];
    std::string literalArrayIdx;
    if ((defineInsInfo.ins_->opcode == pandasm::Opcode::DEFINECLASSWITHBUFFER) ||
        (defineInsInfo.ins_->opcode == pandasm::Opcode::CALLRUNTIME_DEFINESENDABLECLASS)) {
        literalArrayIdx = defineInsInfo.ins_->ids[1];
    } else {  // createobjectwithbuffer
        literalArrayIdx = defineInsInfo.ins_->ids[0];
    }

    OuterMethod outerMethod(this->program_, methodIdx);
    outerMethod.defineInsList_.push_back(info);
    GetMethodNameInfo(nameInsInfo, outerMethod.nameInfo_);
    outerMethod.Init();

    if (this->classTable_.find(literalArrayIdx) != this->classTable_.end()) {
        auto &clazz = this->classTable_.at(literalArrayIdx);
        outerMethod.className_ = clazz.name_;
        outerMethod.export_ = clazz.export_;
        outerMethod.scope_ = clazz.scope_;

        outerMethod.Create();

        LOG(INFO, PANDAGUARD) << TAG << "found method:" << methodIdx << " for class:" << clazz.constructor_.name_;
        clazz.outerMethods_.push_back(outerMethod);
    } else if (this->objectTable_.find(literalArrayIdx) != this->objectTable_.end()) {
        auto &obj = this->objectTable_.at(literalArrayIdx);
        outerMethod.export_ = obj.export_;
        outerMethod.scope_ = obj.scope_;

        outerMethod.Create();

        LOG(INFO, PANDAGUARD) << TAG << "found method:" << methodIdx << " for obj:" << obj.literalArrayIdx_;
        obj.outerMethods_.push_back(outerMethod);
    } else {
        PANDA_GUARD_ABORT_PRINT(TAG, ErrorCode::GENERIC_ERROR, "unexpect outer method for:" << literalArrayIdx);
    }

    outerMethod.ForEachIns([&](const InstructionInfo &insInfo) -> void { ForEachIns(insInfo, FUNCTION); });
}

void panda::guard::Node::CreateObject(const InstructionInfo &info, const Scope scope)
{
    if (info.ins_->opcode != pandasm::Opcode::CREATEOBJECTWITHBUFFER) {
        return;
    }

    const std::string idx = info.ins_->ids[0];
    if (this->objectTable_.find(idx) != this->objectTable_.end()) {
        this->objectTable_.at(idx).defineInsList_.push_back(info);
        return;
    }

    Object object(this->program_, idx);
    LOG(INFO, PANDAGUARD) << TAG << "found record object:" << object.literalArrayIdx_;
    object.defineInsList_.push_back(info);
    object.scope_ = scope;
    object.moduleRecord = &this->moduleRecord_;
    object.Create();

    object.ForEachMethod([&](Function &function) -> void {
        function.ForEachIns([&](const InstructionInfo &insInfo) -> void { ForEachIns(insInfo, FUNCTION); });
    });

    this->objectTable_.emplace(object.literalArrayIdx_, object);
}

void panda::guard::Node::CreateObjectOuterProperty(const panda::guard::InstructionInfo &info)
{
    if (info.ins_->opcode != pandasm::Opcode::DEFINEPROPERTYBYNAME) {
        return;
    }

    InstructionInfo defineIns;
    GraphAnalyzer::HandleDefineProperty(info, defineIns);
    if (!defineIns.IsValid()) {
        return;
    }

    PANDA_GUARD_ASSERT_PRINT(defineIns.ins_->opcode != pandasm::Opcode::CREATEOBJECTWITHBUFFER, TAG,
                             ErrorCode::GENERIC_ERROR, "unexpect related define ins");

    const std::string literalArrayIdx = defineIns.ins_->ids[0];
    PANDA_GUARD_ASSERT_PRINT(this->objectTable_.find(literalArrayIdx) == this->objectTable_.end(), TAG,
                             ErrorCode::GENERIC_ERROR, "no record object for literalArrayIdx:" << literalArrayIdx);

    auto &object = this->objectTable_.at(literalArrayIdx);
    Property property(this->program_, info.ins_->ids[0]);
    property.defineInsList_.push_back(info);
    property.nameInfo_ = info;
    property.export_ = object.export_;
    property.scope_ = object.scope_;

    property.Create();

    object.outerProperties_.push_back(property);
    LOG(INFO, PANDAGUARD) << TAG << "found object outer property:" << property.name_;
}

void panda::guard::Node::CreateOuterProperty(const InstructionInfo &info)
{
    if ((info.ins_->opcode != pandasm::Opcode::STOBJBYNAME) && (info.ins_->opcode != pandasm::Opcode::STOBJBYVALUE)) {
        return;
    }

    InstructionInfo nameInfo;
    Property::GetPropertyNameInfo(info, nameInfo);
    if (!nameInfo.IsValid()) {
        LOG(INFO, PANDAGUARD) << TAG << "invalid nameInfo:" << info.index_ << " " << info.ins_->ToString();
        return;
    }

    Property property(this->program_, nameInfo.ins_->ids[0]);
    property.defineInsList_.push_back(info);
    property.nameInfo_ = nameInfo;

    property.Create();

    this->outerProperties_.push_back(property);
    LOG(INFO, PANDAGUARD) << TAG << "found outer property:" << property.name_;
}

void panda::guard::Node::FindStLexVarName(const panda::guard::InstructionInfo &info)
{
    if (info.ins_->opcode != pandasm::Opcode::STLEXVAR) {
        return;
    }

    InstructionInfo outInfo;
    GraphAnalyzer::GetLdaStr(info, outInfo);
    if (!outInfo.IsValid()) {
        return;
    }

    LOG(INFO, PANDAGUARD) << TAG << "found stlexvar name:" << outInfo.ins_->ids[0];
    GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(outInfo.ins_->ids[0]);
}

void panda::guard::Node::CreateFilePath()
{
    if (!GuardContext::GetInstance()->GetGuardOptions()->IsFileNameObfEnabled()) {
        this->filepath_.name = this->name_;
        return;
    }

    if (this->isNormalizedOhmUrl_) {
        CreateFilePathForNormalizedMode();
    } else {
        CreateFilePathForDefaultMode();
    }
}

void panda::guard::Node::CreateFilePathForDefaultMode()
{
    bool isRemoteHar = StringUtil::IsPrefixMatched(name_, PACKAGE_MODULES_PREFIX.data());
    if (isRemoteHar) {
        std::string prefix = pkgName_ + PATH_DELIMITER.data();
        PANDA_GUARD_ASSERT_PRINT(!StringUtil::IsPrefixMatched(name_, prefix), TAG, ErrorCode::GENERIC_ERROR,
                                 "invalid remote har prefix");
        filepath_.prePart = std::move(prefix);

        filepath_.name = name_.substr(filepath_.prePart.size(), name_.size() - filepath_.prePart.size());
        return;
    }

    // format: bundleName/hapPkgName@pkgName/filepath
    size_t startPos = name_.find_first_of(PATH_DELIMITER.data(), 0);
    if (startPos == std::string::npos) {
        filepath_.name = name_;
        return;
    }

    std::string toFound = pkgName_ + PATH_DELIMITER.data();
    size_t foundPos = name_.find(toFound, startPos);
    if (foundPos == std::string::npos) {
        filepath_.name = name_;
        return;
    }

    foundPos += toFound.size();
    filepath_.prePart = name_.substr(0, foundPos);
    filepath_.name = name_.substr(foundPos, name_.size() - foundPos);
}

void panda::guard::Node::CreateFilePathForNormalizedMode()
{
    // [<bundle name>?]&<package name>/<file path>&[<version>?]
    size_t startPos = name_.find_first_of(NORMALIZED_OHM_DELIMITER.data(), 0);
    std::string prefix = NORMALIZED_OHM_DELIMITER.data() + pkgName_ + PATH_DELIMITER.data();
    PANDA_GUARD_ASSERT_PRINT(!StringUtil::IsPrefixMatched(name_, prefix, startPos), TAG, ErrorCode::GENERIC_ERROR,
                             "invalid normalizedOhmUrl prefix");
    size_t prefixEnd = startPos + prefix.size();
    filepath_.prePart = name_.substr(0, prefixEnd);

    size_t filePathEnd = name_.find_first_of(NORMALIZED_OHM_DELIMITER.data(), prefixEnd);
    PANDA_GUARD_ASSERT_PRINT(filePathEnd == std::string::npos, TAG, ErrorCode::GENERIC_ERROR,
                             "invalid normalizedOhmUrl format");
    filepath_.name = name_.substr(prefixEnd, filePathEnd - prefixEnd);

    filepath_.postPart = name_.substr(filePathEnd, name_.size() - filePathEnd);
}

void panda::guard::Node::ExtractNames()
{
    moduleRecord_.ExtractNames(this->strings_);

    for (const auto &[_, function] : this->functionTable_) {
        function.ExtractNames(this->strings_);
    }

    for (const auto &[_, clazz] : this->classTable_) {
        clazz.ExtractNames(this->strings_);
    }

    for (const auto &[_, object] : this->objectTable_) {
        object.ExtractNames(this->strings_);
    }

    auto parts = StringUtil::Split(filepath_.name, PATH_DELIMITER.data());
    for (const auto &part : parts) {
        this->strings_.emplace(part);
    }

    GuardContext::GetInstance()->GetNameMapping()->AddReservedNames(this->strings_);

    LOG(INFO, PANDAGUARD) << TAG << "strings:";
    for (const auto &str : this->strings_) {
        LOG(INFO, PANDAGUARD) << TAG << str;
    }
}

void panda::guard::Node::RefreshNeedUpdate()
{
    this->fileNameNeedUpdate_ = GuardContext::GetInstance()->GetGuardOptions()->IsFileNameObfEnabled();

    if (GuardContext::GetInstance()->GetGuardOptions()->IsKeepPath(this->name_)) {
        LOG(INFO, PANDAGUARD) << TAG << "found keep rule for:" << this->name_;
        this->contentNeedUpdate_ = false;
        GuardContext::GetInstance()->GetNameMapping()->AddNameMapping(this->strings_);
    }

    for (auto &[_, object] : this->objectTable_) {
        object.SetContentNeedUpdate(this->contentNeedUpdate_);
    }

    this->needUpdate = this->fileNameNeedUpdate_ || this->contentNeedUpdate_;
}

void panda::guard::Node::ForEachFunction(const std::function<FunctionTraver> &callback)
{
    for (auto &[_, function] : this->functionTable_) {
        callback(function);
    }

    for (auto &[_, clazz] : this->classTable_) {
        clazz.ForEachFunction(callback);
    }

    for (auto &[_, object] : this->objectTable_) {
        object.ForEachMethod(callback);
    }
}

void panda::guard::Node::Update()
{
    LOG(INFO, PANDAGUARD) << TAG << "node update for " << this->name_ << " start";

    if (this->fileNameNeedUpdate_) {
        UpdateFileNameDefine();
    }

    // fileNameNeedUpdate_ || contentNeedUpdate_
    for (auto &[_, function] : this->functionTable_) {
        function.Obfuscate();
        function.WriteNameCache(this->sourceName_);
    }

    for (auto &[_, clazz] : this->classTable_) {
        clazz.Obfuscate();
        clazz.WriteNameCache(this->sourceName_);
    }

    for (auto &[_, object] : this->objectTable_) {
        object.Obfuscate();
        object.WriteNameCache(this->sourceName_);
    }

    for (auto &property : this->outerProperties_) {
        property.Obfuscate();
        property.WriteNameCache(this->sourceName_);
    }

    if (this->contentNeedUpdate_) {
        moduleRecord_.Obfuscate();
        moduleRecord_.WriteNameCache(this->sourceName_);
    }

    this->WriteFileCache(this->sourceName_);

    LOG(INFO, PANDAGUARD) << TAG << "node update for " << this->name_ << " end";
}

void panda::guard::Node::WriteFileCache(const std::string &filePath)
{
    GuardContext::GetInstance()->GetNameCache()->AddObfName(filePath, this->obfSourceName_);
    GuardContext::GetInstance()->GetNameCache()->AddSourceFile(filePath, this->sourceFile_, this->obfSourceFile_);
}

void panda::guard::Node::UpdateRecordTable()
{
    auto entry = this->program_->prog_->record_table.extract(this->name_);
    entry.key() = this->obfName_;
    entry.mapped().name = this->obfName_;
    if (!entry.mapped().source_file.empty()) {
        this->UpdateSourceFile(entry.mapped().source_file);
        entry.mapped().source_file = this->obfSourceFile_;
    }
    this->program_->prog_->record_table.insert(std::move(entry));
}

void panda::guard::Node::UpdateFileNameDefine()
{
    filepath_.obfName = GuardContext::GetInstance()->GetNameMapping()->GetFilePath(filepath_.name);
    obfName_ = filepath_.prePart + filepath_.obfName + filepath_.postPart;
    UpdateRecordTable();

    /* e.g.
     *  sourceName_: entry/src/main/ets/entryability/EntryAbility.ets
     *  filepath_.name: src/main/ets/entryability/EntryAbility
     *  sourceNamePrePart: entry/
     *  sourceNamePostPart: .ets
     */
    const auto &[sourceNamePrePart, sourceNamePostPart] = StringUtil::RSplitOnce(this->sourceName_, filepath_.name);
    if (sourceNamePrePart.empty() && sourceNamePostPart.empty()) {
        this->obfSourceName_ = obfName_;
    } else {
        this->obfSourceName_ = sourceNamePrePart + filepath_.obfName + sourceNamePostPart;
    }
}

void panda::guard::Node::UpdateFileNameReferences()
{
    moduleRecord_.UpdateFileNameReferences();
}

void panda::guard::Node::UpdateSourceFile(const std::string &file)
{
    PANDA_GUARD_ASSERT_PRINT(file != this->sourceFile_, TAG, ErrorCode::GENERIC_ERROR, "duplicate source file" << file);
    if (this->sourceFileUpdated_) {
        return;
    }

    this->sourceFile_ = file;

    /* e.g.
     *  file: entry|entry|1.0.0|src/main/ets/entryability/EntryAbility.ets
     *  filepath_.name: src/main/ets/entryability/EntryAbility
     *  prefix: entry|entry|1.0.0|
     *  suffix: .ets
     */
    const auto &[prefix, suffix] = StringUtil::RSplitOnce(file, filepath_.name);
    PANDA_GUARD_ASSERT_PRINT(file != filepath_.name && prefix.empty() && suffix.empty(), TAG, ErrorCode::GENERIC_ERROR,
                             "invalid source file" << file << ",record: " << this->name_);
    this->obfSourceFile_ = prefix + filepath_.obfName + suffix;
    this->sourceFileUpdated_ = true;

    LOG(INFO, PANDAGUARD) << TAG << "source_file: " << this->sourceFile_;
}

void panda::guard::Node::UpdateScopeNames()
{
    LOG(INFO, PANDAGUARD) << TAG << "update scopeNames for:" << this->name_;
    auto &record = this->program_->prog_->record_table.at(this->obfName_);
    for (auto &it : record.field_list) {
        if (it.name == SCOPE_NAMES_FIELD) {
            const auto &literalArrayIdx = it.metadata->GetValue()->GetValue<std::string>();
            LOG(INFO, PANDAGUARD) << TAG << "scopeNames literalArrayIdx:" << literalArrayIdx;
            auto &literalArray = this->program_->prog_->literalarray_table.at(literalArrayIdx);
            UpdateScopeNamesLiteralArray(literalArray);
            break;
        }
    }
}

bool panda::guard::Node::FindPkgName(const panda::pandasm::Record &record, std::string &pkgName)
{
    return std::any_of(
        record.field_list.begin(), record.field_list.end(), [&](const panda::pandasm::Field &field) -> bool {
            bool found = field.name.rfind(PKG_NAME_PREFIX, 0) == 0;
            if (found) {
                pkgName = field.name.substr(PKG_NAME_PREFIX.size(), field.name.size() - PKG_NAME_PREFIX.size());
            }
            return found;
        });
}

bool panda::guard::Node::IsJsonFile(const pandasm::Record &record)
{
    return std::any_of(record.field_list.begin(), record.field_list.end(),
                       [](const auto &field) { return field.name == JSON_FILE_FIELD; });
}

void panda::guard::Node::GetMethodNameInfo(const panda::guard::InstructionInfo &info,
                                           panda::guard::InstructionInfo &nameInfo)
{
    if (!info.IsValid()) {
        return;
    }

    if (InOpcodeList(info, METHOD_NAME_DIRECT_LIST)) {
        nameInfo = info;
    } else if (InOpcodeList(info, METHOD_NAME_INDIRECT_LIST)) {
        GraphAnalyzer::GetLdaStr(info, nameInfo);
    }
}
