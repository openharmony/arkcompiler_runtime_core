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

#include "compiler/tools/aptool/common/profile_loader.h"

namespace ark::aptool::common {

bool ProfileData::Load(const std::string &filePath, std::string &errorMessage)
{
    PandaString pandaPath(filePath);
    PandaString classCtx;
    PandaVector<PandaString> pandaFiles;

    auto result = ark::pgo::AotPgoFile::Load(pandaPath, classCtx, pandaFiles);
    if (!result) {
        errorMessage = result.Error().c_str();
        return false;
    }

    classContext_ = std::move(classCtx);
    pandaFiles_ = std::move(pandaFiles);
    profilingData_ = std::move(*result);
    methodCount_ = 0;
    for (auto &entry : profilingData_.GetAllMethods()) {
        methodCount_ += entry.second.size();
    }
    return true;
}

}  // namespace ark::aptool::common
