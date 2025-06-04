/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "extractor.h"

#include <regex>

namespace ark::extractor {
constexpr char EXT_NAME_ABC[] = ".abc";  // NOLINT(modernize-avoid-c-arrays)
constexpr const char *LOCAL_CODE_PATH = "/data/storage/el1/bundle";
constexpr const char *FILE_SEPARATOR = "/";

static bool StringEndWith(const std::string &str, const char *endStr, size_t endStrLen)
{
    size_t len = str.length();
    return ((len >= endStrLen) && (str.compare(len - endStrLen, endStrLen, endStr) == 0));
}

static std::string GetRelativePath(const std::string &srcPath)
{
    if (srcPath.empty() || srcPath[0] != '/') {
        return srcPath;
    }
    std::regex srcPattern(LOCAL_CODE_PATH);
    std::string relativePath = std::regex_replace(srcPath, srcPattern, "");
    if (relativePath.find(FILE_SEPARATOR) == 0) {
        relativePath = relativePath.substr(1);
        relativePath = relativePath.substr(relativePath.find(std::string(FILE_SEPARATOR)) + 1);
    }
    return relativePath;
}

Extractor::Extractor(const std::string &source) : zipFile_(source)
{
    filePath_ = source;
}

bool Extractor::Init()
{
    if (!zipFile_.Open()) {
        LOG(ERROR, ZIPARCHIVE) << "open zip file failed";
        return false;
    }
    return true;
}

std::shared_ptr<FileMapper> Extractor::GetSafeData(const std::string &fileName)
{
    std::string relativePath = GetRelativePath(fileName);
    if (!StringEndWith(relativePath, EXT_NAME_ABC, sizeof(EXT_NAME_ABC) - 1)) {
        return nullptr;
    }

    return zipFile_.CreateFileMapper(relativePath, FileMapperType::SAFE_ABC);
}
}  // namespace ark::extractor
