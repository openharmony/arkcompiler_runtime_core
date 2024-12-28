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

#include "class.h"

#include "utils/logger.h"

#include "configs/guard_context.h"
#include "program.h"
#include "util/assert_util.h"

namespace {
constexpr std::string_view TAG = "[Class]";
constexpr size_t LITERAL_ITEM_LEN = 2;               // 每两组元素表示一条数据tag-value
constexpr size_t LITERAL_METHOD_GROUP_LEN = 6;       // 每6组元素表示1个方法
constexpr size_t CALL_RUNTIME_LITERAL_ITEM_LEN = 2;  // 1组2个元素 记录运行信息
}  // namespace

void panda::guard::Class::Build()
{
    LOG(INFO, PANDAGUARD) << TAG << "class build for " << this->constructor_.idx_ << " start";

    LOG(INFO, PANDAGUARD) << TAG << "isComponent:" << (this->component ? "true" : "false");

    this->constructor_.defineInsList_ = this->defineInsList_;
    this->constructor_.scope_ = this->scope_;
    this->constructor_.Init();

    // init class base info
    this->name_ = this->constructor_.name_;
    this->SetNameCacheScope(this->name_);
    this->constructor_.export_ = this->moduleRecord_->IsExportVar(this->constructor_.name_);
    this->export_ = this->constructor_.export_;

    this->constructor_.Create();

    auto &literalArrayTable = program_->prog_->literalarray_table;
    auto it = literalArrayTable.find(this->literalArrayIdx_);
    PANDA_GUARD_ASSERT_PRINT(it == literalArrayTable.end(), TAG, ErrorCode::GENERIC_ERROR,
                             "get bad literalArrayIdx:" << literalArrayIdx_);
    CreateMethods(it->second);

    LOG(INFO, PANDAGUARD) << TAG << "class build for " << this->constructor_.idx_ << " end";
}

size_t panda::guard::Class::GetMethodCnt() const
{
    return this->methods_.size() + this->outerMethods_.size();
}

size_t panda::guard::Class::GetPropertyCnt() const
{
    size_t cnt = 0;
    for (const auto &method : this->methods_) {
        cnt += method.properties_.size();
    }

    for (const auto &method : this->outerMethods_) {
        cnt += method.properties_.size();
    }

    return cnt;
}

void panda::guard::Class::ForEachMethodIns(const std::function<InsTraver> &callback)
{
    this->ForEachFunction([&callback](Function &func) { func.ForEachIns(callback); });
}

void panda::guard::Class::ForEachFunction(const std::function<FunctionTraver> &callback)
{
    callback(this->constructor_);
    for (auto &method : this->methods_) {
        callback(method);
    }

    for (auto &method : this->outerMethods_) {
        callback(method);
    }
}

void panda::guard::Class::ExtractNames(std::set<std::string> &strings) const
{
    this->constructor_.ExtractNames(strings);

    for (const auto &method : this->methods_) {
        method.ExtractNames(strings);
    }

    for (const auto &method : this->outerMethods_) {
        method.ExtractNames(strings);
    }
}

void panda::guard::Class::CreateMethods(const pandasm::LiteralArray &literalArray)
{
    LOG(INFO, PANDAGUARD) << TAG << "literalArray size:" << literalArray.literals_.size();
    size_t staticMethodIndexLiteralIndex = literalArray.literals_.size() - 1;
    size_t methodItemCount = literalArray.literals_.size() - LITERAL_ITEM_LEN;
    if (callRunTimeInst_) {
        staticMethodIndexLiteralIndex -= CALL_RUNTIME_LITERAL_ITEM_LEN;
        methodItemCount -= CALL_RUNTIME_LITERAL_ITEM_LEN;
    }

    const auto &staticMethodIndexLiteral = literalArray.literals_[staticMethodIndexLiteralIndex];
    PANDA_GUARD_ASSERT_PRINT(staticMethodIndexLiteral.tag_ != panda::panda_file::LiteralTag::INTEGER, TAG,
                             ErrorCode::GENERIC_ERROR, "bad tag for staticMethodIndex");

    const uint32_t staticMethodIndex = std::get<uint32_t>(staticMethodIndexLiteral.value_);
    const size_t methodCount = methodItemCount / LITERAL_METHOD_GROUP_LEN;
    LOG(INFO, PANDAGUARD) << TAG << "methodCount:" << methodCount;

    for (size_t i = 0; i < methodCount; i++) {
        CreateMethod(literalArray, i * LITERAL_METHOD_GROUP_LEN, i >= staticMethodIndex);
    }
}

void panda::guard::Class::CreateMethod(const pandasm::LiteralArray &literalArray, size_t index, bool isStatic)
{
    const size_t methodNameLiteralIndex = index + 1;
    const auto &methodNameLiteral = literalArray.literals_[methodNameLiteralIndex];
    PANDA_GUARD_ASSERT_PRINT(methodNameLiteral.tag_ != panda::panda_file::LiteralTag::STRING, TAG,
                             ErrorCode::GENERIC_ERROR, "bad tag for methodName");

    const std::string methodName = std::get<std::string>(methodNameLiteral.value_);
    LOG(INFO, PANDAGUARD) << TAG << "methodName:" << methodName;

    const size_t methodIdxLiteralIndex = methodNameLiteralIndex + LITERAL_ITEM_LEN;
    PANDA_GUARD_ASSERT_PRINT(methodIdxLiteralIndex >= literalArray.literals_.size(), TAG, ErrorCode::GENERIC_ERROR,
                             "methodIdxLiteralIndex offset overflow");
    const auto &[tag_, value_] = literalArray.literals_[methodIdxLiteralIndex];
    PANDA_GUARD_ASSERT_PRINT(tag_ != panda::panda_file::LiteralTag::METHOD, TAG, ErrorCode::GENERIC_ERROR,
                             "bad tag for methodIdx");

    const std::string methodIdx = std::get<std::string>(value_);
    LOG(INFO, PANDAGUARD) << TAG << "methodIdx:" << methodIdx;

    Method method(this->program_, methodIdx);
    method.literalArrayIdx_ = this->literalArrayIdx_;
    method.className_ = this->name_;
    method.idxIndex_ = methodIdxLiteralIndex;
    method.nameIndex_ = methodNameLiteralIndex;
    method.static_ = isStatic;
    method.scope_ = this->scope_;
    method.export_ = this->export_;

    method.Init();
    method.Create();
    PANDA_GUARD_ASSERT_PRINT(method.name_ != methodName, TAG, ErrorCode::GENERIC_ERROR,
                             "get different name for method:" << method.idx_);

    this->methods_.push_back(method);
}

void panda::guard::Class::Update()
{
    LOG(INFO, PANDAGUARD) << TAG << "class update for " << this->name_ << " start";
    this->constructor_.Obfuscate();
    this->obfName_ = this->constructor_.obfName_;

    for (auto &method : this->methods_) {
        method.Obfuscate();
    }

    for (auto &method : this->outerMethods_) {
        method.Obfuscate();
    }

    LOG(INFO, PANDAGUARD) << TAG << "class update for " << this->name_ << " end";
}

void panda::guard::Class::WriteNameCache(const std::string &filePath)
{
    if (!this->obfuscated) {
        return;
    }
    this->WriteFileCache(filePath);
    this->WritePropertyCache();

    this->constructor_.WriteNameCache(filePath);
    for (auto &property : this->constructor_.properties_) {
        property.WriteNameCache(filePath);
    }

    for (auto &method : this->methods_) {
        method.WriteNameCache(filePath);
    }

    for (auto &method : this->outerMethods_) {
        method.WriteNameCache(filePath);
    }
}

void panda::guard::Class::WriteFileCache(const std::string &filePath)
{
    if (!constructor_.nameNeedUpdate_) {
        return;
    }

    GuardContext::GetInstance()->GetNameCache()->AddObfIdentifierName(filePath, this->GetNameCacheScope(),
                                                                      this->obfName_);
}

void panda::guard::Class::WritePropertyCache()
{
    TopLevelOptionEntity::WritePropertyCache(*this);
}
