/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <algorithm>
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif
#include <string_view>

#include "compiler/tools/aptool/dump/filter.h"

namespace ark::aptool::dump {

bool PandaFileMatches(const PandaString *pandaFileName, const DumpFilterOptions &filters)
{
    if (filters.keepPandaFiles.empty()) {
        return true;
    }
    if (pandaFileName == nullptr) {
        return false;
    }
    std::string_view pandaFileView(pandaFileName->c_str());
    fs::path filePath(pandaFileView);
    auto baseName = filePath.filename().string();
    for (const auto &value : filters.keepPandaFiles) {
        if (pandaFileView == value) {
            return true;
        }
        fs::path filterPath(value);
        auto filterBase = filterPath.filename().string();
        if ((!filterBase.empty() && filterBase == baseName) || value == baseName) {
            return true;
        }
    }
    return false;
}

bool ClassMatches(const std::optional<std::string> &classDescriptor, const DumpFilterOptions &filters)
{
    if (filters.keepClassDescriptors.empty()) {
        return true;
    }
    if (!classDescriptor.has_value()) {
        return false;
    }
    return std::any_of(filters.keepClassDescriptors.begin(), filters.keepClassDescriptors.end(),
                       [&](const std::string &value) { return *classDescriptor == value; });
}

bool MethodMatches(const std::optional<std::string> &methodSignature, const DumpFilterOptions &filters)
{
    if (filters.keepMethodNames.empty()) {
        return true;
    }
    if (!methodSignature.has_value() || methodSignature->empty()) {
        return false;
    }
    auto nameEnd = methodSignature->find('(');
    std::string_view methodName(methodSignature->data(),
                                nameEnd == std::string::npos ? methodSignature->size() : nameEnd);
    for (const auto &value : filters.keepMethodNames) {
        if (methodSignature->find(value) != std::string::npos) {
            return true;
        }
        if (!methodName.empty() && methodName.find(value) != std::string::npos) {
            return true;
        }
    }
    return false;
}

}  // namespace ark::aptool::dump
