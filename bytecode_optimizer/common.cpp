/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "common.h"
#include "compiler/optimizer/ir/basicblock.h"
#include "compiler/optimizer/ir/graph.h"
#include "libpandafile/file.h"
#include "libpandafile/file-inl.h"

namespace panda::bytecodeopt {

uint8_t AccReadIndex(const compiler::Inst *inst)
{
    if (inst->IsIntrinsic() && inst->IsAccRead()) {
        ASSERT(inst->GetBasicBlock()->GetGraph()->IsDynamicMethod());
        ASSERT(inst->GetInputsCount() >= 2U);
        return inst->GetInputsCount() - 2U;
    }
    return 0;
}

std::string GetStringFromPandaFile(const panda_file::File &pfile, uint32_t offset)
{
    const auto sd = pfile.GetStringData(panda_file::File::EntityId(offset));
    std::string str(utf::Mutf8AsCString(sd.data));
    return str;
}
}  // namespace panda::bytecodeopt
