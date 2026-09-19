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

#include "plugins/ets/runtime/types/ets_abc_package.h"

#include <cstdint>

#include "libziparchive/extractortool/extractor.h"

namespace ark::ets {

bool EndsWith(std::string_view path, std::string_view suffix)
{
    return path.length() >= suffix.length() &&
           path.compare(path.length() - suffix.length(), suffix.length(), suffix.data()) == 0;
}

std::string GetAbcPathFromPackagePath(std::string_view packagePath, std::string_view suffix)
{
    std::string derived(packagePath.substr(0, packagePath.length() - suffix.length()));
    derived.append(PACKAGE_ABC_PATH.data(), PACKAGE_ABC_PATH.length());
    return derived;
}

PackageReadResult ReadPackageAbc(const std::string &packagePath, std::string_view suffix,
                                 std::unique_ptr<const panda_file::File> &outPf)
{
    auto extractor = std::make_shared<ark::extractor::Extractor>(packagePath);
    if (extractor == nullptr || !extractor->Init()) {
        return PackageReadResult::OPEN_FAILED;
    }

    const std::string abcPath = GetAbcPathFromPackagePath(packagePath, suffix);
    // A `.hsp` is looked up by its derived-path entry without an integrity check; every other
    // package by the fixed entry name with the check. The asymmetry is the format's own.
    auto safeData = (suffix == HSP_SUFFIX) ? extractor->GetSafeDataForHsp(abcPath)
                                           : extractor->GetSafeData(std::string(PACKAGE_ABC_ENTRY));
    if (safeData == nullptr) {
        return PackageReadResult::NO_ABC_ENTRY;
    }

    outPf = panda_file::OpenPandaFileFromSecureMemory(safeData->GetDataPtr(), safeData->GetDataLen(), abcPath);
    return outPf == nullptr ? PackageReadResult::OPEN_FAILED : PackageReadResult::OK;
}

}  // namespace ark::ets
