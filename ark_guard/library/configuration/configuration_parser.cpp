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

#include "configuration_parser.h"

#include <regex>

#include "libarkbase/utils/json_parser.h"

#include "error.h"
#include "keep_option_parser.h"
#include "util/assert_util.h"
#include "util/file_util.h"
#include "util/json_util.h"
#include "util/string_util.h"

namespace {
constexpr std::string_view ABC_PATH = "abcPath";
constexpr std::string_view OBF_ABC_PATH = "obfAbcPath";
constexpr std::string_view DEFAULT_NAME_CACHE_PATH = "defaultNameCachePath";

constexpr std::string_view OBFUSCATION_RULES = "obfuscationRules";
constexpr std::string_view DISABLE_OBFUSCATION = "disableObfuscation";
constexpr std::string_view PRINT_NAME_CACHE = "printNameCache";
constexpr std::string_view APPLY_NAME_CACHE = "applyNameCache";
constexpr std::string_view REMOVE_LOG = "removeLog";

constexpr std::string_view PRINT_SEEDS_OPTION = "printSeedsOption";
constexpr std::string_view ENABLE = "enable";
constexpr std::string_view FILE_PATH = "filePath";

constexpr std::string_view FILE_NAME_OBFUSCATION = "fileNameObfuscation";
constexpr std::string_view RESERVED_FILE_NAMES = "reservedFileNames";
constexpr std::string_view UNIVERSAL_RESERVED_FILE_NAMES = "universalReservedFileNames";

constexpr std::string_view KEEP_OPTION = "keepOption";
constexpr std::string_view KEEP_PATH = "keepPath";
constexpr std::string_view RESERVED_PATH = "reservedPath";
constexpr std::string_view UNIVERSAL_RESERVED_PATH = "universalReservedPath";

constexpr std::string_view KEEP = "keep";
constexpr std::string_view KEEP_CLASS_WITH_MEMBERS = "keepclasswithmembers";
constexpr std::string_view KEEP_CLASS_MEMBERS = "keepclassmembers";
}  // namespace

void ark::guard::ConfigurationParser::Parse(Configuration &configuration)
{
    std::string fileContent = FileUtil::GetFileContent(configPath_);
    ARK_GUARD_ASSERT(fileContent.empty(), ErrorCode::CONFIGURATION_FILE_FORMAT_ERROR,
                     "configuration file is empty:" + configPath_);

    ParseConfigurationFile(fileContent, configuration);
    ARK_GUARD_ASSERT(!configuration.IsValid(), ErrorCode::CONFIGURATION_FILE_FORMAT_ERROR, "configuration is invalid");
}

void ark::guard::ConfigurationParser::ParseConfigurationFile(const std::string &content, Configuration &configuration)
{
    JsonObject object(content);
    ARK_GUARD_ASSERT(!object.IsValid(), ErrorCode::CONFIGURATION_FILE_FORMAT_ERROR,
                     "the configuration file is not a valid json");

    configuration.abcPath = JsonUtil::GetStringValue(&object, ABC_PATH, false);
    configuration.obfAbcPath = JsonUtil::GetStringValue(&object, OBF_ABC_PATH, false);
    configuration.defaultNameCachePath = JsonUtil::GetStringValue(&object, DEFAULT_NAME_CACHE_PATH, false);

    auto obfRulesObject = JsonUtil::GetJsonObject(&object, OBFUSCATION_RULES);
    if (!obfRulesObject) {
        return;
    }
    configuration.obfuscationRules.disableObfuscation = JsonUtil::GetBoolValue(obfRulesObject, DISABLE_OBFUSCATION);
    configuration.obfuscationRules.printNameCache = JsonUtil::GetStringValue(obfRulesObject, PRINT_NAME_CACHE);
    configuration.obfuscationRules.applyNameCache = JsonUtil::GetStringValue(obfRulesObject, APPLY_NAME_CACHE);
    configuration.obfuscationRules.removeLog = JsonUtil::GetBoolValue(obfRulesObject, REMOVE_LOG);

    ParsePrintSeedsOption(obfRulesObject, configuration);
    ParseFileNameObfuscation(obfRulesObject, configuration);
    ParseKeepOption(obfRulesObject, configuration);
}

void ark::guard::ConfigurationParser::ParsePrintSeedsOption(const ark::JsonObject *object, Configuration &configuration)
{
    auto innerObject = JsonUtil::GetJsonObject(object, PRINT_SEEDS_OPTION);
    if (!innerObject) {
        return;
    }
    auto option = &configuration.obfuscationRules.printSeedsOption;
    option->enable = JsonUtil::GetBoolValue(innerObject, ENABLE);
    option->filePath = JsonUtil::GetStringValue(innerObject, FILE_PATH);
}

void ark::guard::ConfigurationParser::ParseFileNameObfuscation(const ark::JsonObject *object,
                                                               Configuration &configuration)
{
    auto innerObject = JsonUtil::GetJsonObject(object, FILE_NAME_OBFUSCATION);
    if (!innerObject) {
        return;
    }
    auto option = &configuration.obfuscationRules.fileNameOption;
    option->enable = JsonUtil::GetBoolValue(innerObject, ENABLE);
    option->reservedFileNames = JsonUtil::GetArrayStringValue(innerObject, RESERVED_FILE_NAMES);
    option->universalReservedFileNames = JsonUtil::GetArrayStringValue(innerObject, UNIVERSAL_RESERVED_FILE_NAMES);
}

void ark::guard::ConfigurationParser::ParseKeepOption(const ark::JsonObject *object, Configuration &configuration)
{
    auto innerObject = JsonUtil::GetJsonObject(object, KEEP_OPTION);
    if (!innerObject) {
        return;
    }
    auto keepPathObject = JsonUtil::GetJsonObject(innerObject, KEEP_PATH);
    if (keepPathObject) {
        auto pathOption = &configuration.obfuscationRules.keepOption.pathOption;
        pathOption->reservedPaths =
            StringUtil::ReplaceSlashWithDot(JsonUtil::GetArrayStringValue(keepPathObject, RESERVED_PATH));
        pathOption->universalReservedPaths =
            StringUtil::ReplaceSlashWithDot(JsonUtil::GetArrayStringValue(keepPathObject, UNIVERSAL_RESERVED_PATH));
    }

    auto keeps = JsonUtil::GetArrayStringValue(innerObject, KEEP);
    auto keepClassMembers = JsonUtil::GetArrayStringValue(innerObject, KEEP_CLASS_MEMBERS);
    auto keepClassWithMembers = JsonUtil::GetArrayStringValue(innerObject, KEEP_CLASS_WITH_MEMBERS);

    size_t keepOptionSize = keeps.size() + keepClassMembers.size() + keepClassWithMembers.size();

    std::vector<std::string> keepOptionStrList;
    keepOptionStrList.reserve(keepOptionSize);
    keepOptionStrList.insert(keepOptionStrList.end(), std::make_move_iterator(keeps.begin()),
                             std::make_move_iterator(keeps.end()));
    keepOptionStrList.insert(keepOptionStrList.end(), std::make_move_iterator(keepClassMembers.begin()),
                             std::make_move_iterator(keepClassMembers.end()));
    keepOptionStrList.insert(keepOptionStrList.end(), std::make_move_iterator(keepClassWithMembers.begin()),
                             std::make_move_iterator(keepClassWithMembers.end()));

    configuration.obfuscationRules.keepOption.classSpecifications.reserve(keepOptionSize);
    for (const auto &keepOptionStr : keepOptionStrList) {
        KeepOptionParser parser(keepOptionStr);
        auto result = parser.Parse();
        if (result.has_value()) {
            configuration.obfuscationRules.keepOption.classSpecifications.emplace_back(result.value());
        }
    }
}