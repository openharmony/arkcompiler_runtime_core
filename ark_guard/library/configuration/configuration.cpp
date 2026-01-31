/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "configuration.h"

#include <algorithm>

#include "error.h"
#include "logger.h"
#include "util/assert_util.h"
#include "util/string_util.h"

namespace {
bool IsReserved(const std::vector<std::string> &reservedNames, const std::vector<std::string> &universalReservedNames,
                const std::string &str)
{
    if (std::any_of(reservedNames.begin(), reservedNames.end(), [str](const auto &field) { return field == str; })) {
        return true;
    }

    return std::any_of(universalReservedNames.begin(), universalReservedNames.end(),
                       [&str](const auto &field) { return ark::guard::StringUtil::RegexMatch(str, field); });
}

std::string FindLongestReservedPath(const std::vector<std::string> &reservedNames,
                                    const std::vector<std::string> &universalReservedNames, const std::string &str)
{
    size_t maxLength = std::max(ark::guard::StringUtil::FindLongestMatchedPrefix(reservedNames, str),
                                ark::guard::StringUtil::FindLongestMatchedPrefixReg(universalReservedNames, str));
    if (maxLength == 0) {
        return "";
    }
    return str.substr(0, maxLength);
}

}  // namespace

bool ark::guard::Configuration::IsValid() const
{
    LOG_I << "abcPath:" << abcPath;
    LOG_I << "obfAbcPath:" << obfAbcPath;
    LOG_I << "defaultNameCachePath:" << defaultNameCachePath;
    LOG_I << "disableObfuscation:" << obfuscationRules.disableObfuscation;
    LOG_I << "printNameCache:" << obfuscationRules.printNameCache;
    LOG_I << "applyNameCache:" << obfuscationRules.applyNameCache;
    LOG_I << "removeLog:" << obfuscationRules.removeLog;
    LOG_I << "printSeeds enable:" << obfuscationRules.printSeedsOption.enable;
    LOG_I << "printSeeds filePath:" << obfuscationRules.printSeedsOption.filePath;
    LOG_I << "fileNameObfuscation enable:" << obfuscationRules.fileNameOption.enable;

    ARK_GUARD_ASSERT(abcPath.empty(), ErrorCode::CONFIGURATION_FILE_FORMAT_ERROR,
                     "Configuration parsing failed: the value of field abcPath in the configuration file is invalid");
    ARK_GUARD_ASSERT(
        obfAbcPath.empty(), ErrorCode::CONFIGURATION_FILE_FORMAT_ERROR,
        "Configuration parsing failed: the value of field obfAbcPath in the configuration file is invalid");
    ARK_GUARD_ASSERT(
        defaultNameCachePath.empty(), ErrorCode::CONFIGURATION_FILE_FORMAT_ERROR,
        "Configuration parsing failed: the value of field defaultNameCachePath in the configuration file is invalid");

    return true;
}
const std::string &ark::guard::Configuration::GetAbcPath() const
{
    return abcPath;
}

const std::string &ark::guard::Configuration::GetObfAbcPath() const
{
    return obfAbcPath;
}

const std::string &ark::guard::Configuration::GetDefaultNameCachePath() const
{
    return defaultNameCachePath;
}

const std::string &ark::guard::Configuration::GetPrintNameCache() const
{
    return obfuscationRules.printNameCache;
}

const std::string &ark::guard::Configuration::GetApplyNameCache() const
{
    return obfuscationRules.applyNameCache;
}

bool ark::guard::Configuration::DisableObfuscation() const
{
    return obfuscationRules.disableObfuscation;
}

bool ark::guard::Configuration::IsRemoveLogObfEnabled() const
{
    return obfuscationRules.removeLog;
}

ark::guard::PrintSeedsOption ark::guard::Configuration::GetPrintSeedsOption() const
{
    return obfuscationRules.printSeedsOption;
}

bool ark::guard::Configuration::IsFileNameObfEnabled() const
{
    return obfuscationRules.fileNameOption.enable;
}

bool ark::guard::Configuration::IsKeepFileName(const std::string &str) const
{
    const auto fileNameOption = &obfuscationRules.fileNameOption;
    return IsReserved(fileNameOption->reservedFileNames, fileNameOption->universalReservedFileNames, str);
}

bool ark::guard::Configuration::IsKeepPath(const std::string &str, std::string &keptPath) const
{
    const auto pathOption = &obfuscationRules.keepOptions.pathOption;
    if (pathOption->reservedPaths.empty() && pathOption->universalReservedPaths.empty()) {
        return false;
    }

    keptPath = FindLongestReservedPath(pathOption->reservedPaths, pathOption->universalReservedPaths, str);
    return !keptPath.empty();
}

std::vector<ark::guard::ClassSpecification> ark::guard::Configuration::GetClassSpecifications() const
{
    return obfuscationRules.keepOptions.classSpecifications;
}

bool ark::guard::Configuration::HasKeepConfiguration() const
{
    const auto &fileNameOption = obfuscationRules.fileNameOption;
    const bool hasFileNameKeep =
        !fileNameOption.reservedFileNames.empty() || !fileNameOption.universalReservedFileNames.empty();

    const auto &keepPathOption = obfuscationRules.keepOptions.pathOption;
    const bool hasPathKeep = !keepPathOption.reservedPaths.empty() || !keepPathOption.universalReservedPaths.empty();

    const bool hasClassSpecs = !GetClassSpecifications().empty();

    return hasFileNameKeep || hasPathKeep || hasClassSpecs;
}

void ark::guard::Configuration::WarmupRegexCache() const
{
    std::vector<std::string> patternsForIsMatch;
    std::vector<std::string> patternsRaw;

    const auto &fileNameOption = obfuscationRules.fileNameOption;
    patternsRaw.insert(patternsRaw.end(), fileNameOption.universalReservedFileNames.begin(),
                       fileNameOption.universalReservedFileNames.end());

    const auto &pathOption = obfuscationRules.keepOptions.pathOption;
    patternsRaw.insert(patternsRaw.end(), pathOption.universalReservedPaths.begin(),
                       pathOption.universalReservedPaths.end());

    for (const auto &spec : obfuscationRules.keepOptions.classSpecifications) {
        for (const auto *p : {&spec.className_, &spec.extensionClassName_, &spec.annotationName_,
                              &spec.extensionAnnotationName_}) {
            if (!p->empty()) {
                patternsForIsMatch.push_back(*p);
            }
        }
        for (const auto &field : spec.fieldSpecifications_) {
            if (!field.memberName_.empty()) {
                patternsForIsMatch.push_back(field.memberName_);
            }
            if (!field.memberType_.empty()) {
                patternsForIsMatch.push_back(field.memberType_);
            }
            if (!field.memberValue_.empty()) {
                patternsForIsMatch.push_back(field.memberValue_);
            }
        }
        for (const auto &method : spec.methodSpecifications_) {
            if (!method.memberName_.empty()) {
                patternsForIsMatch.push_back(method.memberName_);
            }
            if (!method.memberType_.empty()) {
                patternsForIsMatch.push_back(method.memberType_);
            }
            if (!method.memberValue_.empty()) {
                patternsForIsMatch.push_back(method.memberValue_);
            }
        }
    }

    StringUtil::WarmupRegexCache(patternsRaw);
    StringUtil::WarmupRegexCacheForIsMatch(patternsForIsMatch);
}
