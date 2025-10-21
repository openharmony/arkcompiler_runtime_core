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

#include "seeds_dumper.h"

#include <fstream>
#include <iostream>

#include "logger.h"
#include "obfuscator_data_manager.h"

namespace {
constexpr std::string_view INDENT = "  ";
}

void ark::guard::SeedsDumper::Flush() const
{
    if (seedsOption_.filePath.empty()) {
        std::cout << stream_.str();
        return;
    }

    std::ofstream ofs;
    ofs.open(seedsOption_.filePath, std::ios::trunc | std::ios::out);
    if (!ofs.is_open()) {
        std::cout << stream_.str();
        return;
    }
    ofs << stream_.str();
    ofs.close();
}

bool ark::guard::SeedsDumper::Dump(abckit_wrapper::FileView &fileView)
{
    if (!seedsOption_.enable) {
        return true;
    }

    if (!fileView.ModulesAccept(this->Wrap<abckit_wrapper::LocalModuleFilter>())) {
        return false;
    }

    Flush();
    return true;
}

bool ark::guard::SeedsDumper::Visit(abckit_wrapper::Module *module)
{
    ObfuscatorDataManager manager(module);
    auto obfuscateData = manager.GetData();
    if (obfuscateData == nullptr) {
        LOG_E << "Get module obfuscate data failed, module:" << module->GetFullyQualifiedName();
        return false;
    }
    if (obfuscateData->IsKept()) {
        stream_ << module->GetFullyQualifiedName() << std::endl;
    }

    return module->ChildrenAccept(*this);
}

bool ark::guard::SeedsDumper::VisitNamespace(abckit_wrapper::Namespace *ns)
{
    ObfuscatorDataManager manager(ns);
    auto obfuscateData = manager.GetData();
    auto parentObfuscateData = manager.GetParentData();
    if (obfuscateData == nullptr || parentObfuscateData == nullptr) {
        LOG_E << "Get namespace obfuscate data failed, namespace:" << ns->GetFullyQualifiedName();
        return false;
    }
    const auto indent = parentObfuscateData->GetIndent() + INDENT.data();
    obfuscateData->SetIndent(indent);
    if (obfuscateData->IsKept()) {
        stream_ << indent << ns->GetName() << std::endl;
    }
    return ns->ChildrenAccept(*this);
}

bool ark::guard::SeedsDumper::VisitMethod(abckit_wrapper::Method *method)
{
    if (method->IsConstructor()) {
        return true;
    }
    ObfuscatorDataManager manager(method);
    auto obfuscateData = manager.GetData();
    auto parentObfuscateData = manager.GetParentData();
    if (obfuscateData == nullptr || parentObfuscateData == nullptr) {
        LOG_E << "Get method obfuscate data failed, method:" << method->GetFullyQualifiedName();
        return false;
    }
    const auto indent = parentObfuscateData->GetIndent() + INDENT.data();
    if (obfuscateData->IsKept()) {
        stream_ << indent << method->GetSignature() << std::endl;
    }
    return true;
}

bool ark::guard::SeedsDumper::VisitField(abckit_wrapper::Field *field)
{
    ObfuscatorDataManager manager(field);
    auto obfuscateData = manager.GetData();
    auto parentObfuscateData = manager.GetParentData();
    if (obfuscateData == nullptr || parentObfuscateData == nullptr) {
        LOG_E << "Get field obfuscate data failed, field:" << field->GetFullyQualifiedName();
        return false;
    }
    const auto indent = parentObfuscateData->GetIndent() + INDENT.data();
    if (obfuscateData->IsKept()) {
        stream_ << indent << field->GetSignature() << std::endl;
    }
    return true;
}

bool ark::guard::SeedsDumper::VisitClass(abckit_wrapper::Class *clazz)
{
    ObfuscatorDataManager manager(clazz);
    auto obfuscateData = manager.GetData();
    auto parentObfuscateData = manager.GetParentData();
    if (obfuscateData == nullptr || parentObfuscateData == nullptr) {
        LOG_E << "Get class obfuscate data failed, class:" << clazz->GetFullyQualifiedName();
        return false;
    }
    const auto indent = parentObfuscateData->GetIndent() + INDENT.data();
    obfuscateData->SetIndent(indent);
    if (obfuscateData->IsKept()) {
        stream_ << indent << clazz->GetName() << std::endl;
    }
    return clazz->ChildrenAccept(*this);
}

bool ark::guard::SeedsDumper::VisitAnnotationInterface(abckit_wrapper::AnnotationInterface *ai)
{
    ObfuscatorDataManager manager(ai);
    auto obfuscateData = manager.GetData();
    auto parentObfuscateData = manager.GetParentData();
    if (obfuscateData == nullptr || parentObfuscateData == nullptr) {
        LOG_E << "Get annotation interface obfuscate data failed, annotation interface:" << ai->GetFullyQualifiedName();
        return false;
    }
    const auto indent = parentObfuscateData->GetIndent() + INDENT.data();
    if (obfuscateData->IsKept()) {
        stream_ << indent << ai->GetName() << std::endl;
    }
    return true;
}