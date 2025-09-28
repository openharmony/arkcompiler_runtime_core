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

#include "configuration.h"

#include <algorithm>
#include <regex>

#include "error.h"
#include "logger.h"
#include "util/assert_util.h"

namespace {
bool IsReserved(const std::vector<std::string> &reservedNames, const std::vector<std::string> &universalReservedNames,
                const std::string &str)
{
    if (std::any_of(reservedNames.begin(), reservedNames.end(), [str](const auto &field) { return field == str; })) {
        return true;
    }

    return std::any_of(universalReservedNames.begin(), universalReservedNames.end(), [str](const auto &field) {
        std::regex pattern(field);
        return std::regex_search(str, pattern);
    });
}
}  // namespace

bool ark::guard::Configuration::IsValid() const
{
    LOG_D << "abcPath:" << abcPath;
    LOG_D << "obfAbcPath:" << obfAbcPath;
    LOG_D << "defaultNameCachePath:" << defaultNameCachePath;
    LOG_D << "disableObfuscation:" << obfuscationRules.disableObfuscation;
    LOG_D << "printNameCache:" << obfuscationRules.printNameCache;
    LOG_D << "applyNameCache:" << obfuscationRules.applyNameCache;
    LOG_D << "removeLog:" << obfuscationRules.removeLog;
    LOG_D << "printSeeds enable:" << obfuscationRules.printSeedsOption.enable;
    LOG_D << "printSeeds filePath:" << obfuscationRules.printSeedsOption.filePath;
    LOG_D << "fileNameObfuscation enable:" << obfuscationRules.fileNameOption.enable;

    ARK_GUARD_ASSERT(abcPath.empty(), ErrorCode::CONFIGURATION_FILE_FORMAT_ERROR,
                     "the value of field abcPath in the configuration file is invalid");
    ARK_GUARD_ASSERT(obfAbcPath.empty(), ErrorCode::CONFIGURATION_FILE_FORMAT_ERROR,
                     "the value of field obfAbcPath in the configuration file is invalid");
    ARK_GUARD_ASSERT(defaultNameCachePath.empty(), ErrorCode::CONFIGURATION_FILE_FORMAT_ERROR,
                     "the value of field defaultNameCachePath in the configuration file is invalid");

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

bool ark::guard::Configuration::IsKeepPath(const std::string &str) const
{
    const auto pathOption = &obfuscationRules.keepOption.pathOption;
    if (pathOption->reservedPaths.empty() && pathOption->universalReservedPaths.empty()) {
        return false;
    }
    return IsReserved(pathOption->reservedPaths, pathOption->universalReservedPaths, str);
}

std::vector<ark::guard::ClassSpecification> ark::guard::Configuration::GetClassSpecifications() const
{
    return obfuscationRules.keepOption.classSpecifications;
}

bool ark::guard::Configuration::HasKeepConfiguration() const
{
    const auto &fileNameOption = obfuscationRules.fileNameOption;
    const bool hasFileNameKeep =
        !fileNameOption.reservedFileNames.empty() || !fileNameOption.universalReservedFileNames.empty();

    const auto &keepPathOption = obfuscationRules.keepOption.pathOption;
    const bool hasPathKeep = !keepPathOption.reservedPaths.empty() || !keepPathOption.universalReservedPaths.empty();

    const bool hasClassSpecs = !GetClassSpecifications().empty();

    return hasFileNameKeep || hasPathKeep || hasClassSpecs;
}
