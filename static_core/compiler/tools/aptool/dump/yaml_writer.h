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

#ifndef PANDA_COMPILER_TOOLS_APTOOL_DUMP_YAML_WRITER_H
#define PANDA_COMPILER_TOOLS_APTOOL_DUMP_YAML_WRITER_H

#include <ostream>
#include <string>

#include "compiler/tools/aptool/common/abc_metadata.h"
#include "compiler/tools/aptool/common/profile_loader.h"
#include "compiler/tools/aptool/dump/filter.h"

namespace ark::aptool::dump {

using common::AbcMetadataProvider;
using common::PandaString;
using common::PandaVector;
using common::ProfileData;

class YamlWriter {
public:
    explicit YamlWriter(std::ostream &out) : out_(out) {}

    void WriteLine(const std::string &line);

    void IncreaseIndent();

    void DecreaseIndent();

private:
    std::ostream &out_;
    uint32_t indent_ {0};
};

void DumpProfileToYaml(std::ostream &out, const ProfileData &profile, const AbcMetadataProvider *metadata,
                       const DumpFilterOptions &filters);

}  // namespace ark::aptool::dump

#endif  // PANDA_COMPILER_TOOLS_APTOOL_DUMP_YAML_WRITER_H
