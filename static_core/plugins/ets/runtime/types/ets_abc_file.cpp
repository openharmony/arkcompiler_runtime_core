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

#include "plugins/ets/runtime/types/ets_abc_file.h"

#include <memory>

#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "runtime/include/runtime.h"

namespace ark::ets {

EtsAbcFile *EtsAbcFile::CreateAbcFile(EtsExecutionContext *executionCtx, ClassLinkerContext *ctx,
                                      std::unique_ptr<const panda_file::File> &&pf)
{
    auto *abcFile =
        EtsAbcFile::FromEtsObject(EtsObject::Create(executionCtx, PlatformTypes(executionCtx)->arkruntimeAbcFile));
    if (UNLIKELY(abcFile == nullptr)) {
        return nullptr;
    }
    abcFile->SetPandaFile(pf.get());
    Runtime::GetCurrent()->GetClassLinker()->AddPandaFile(std::move(pf), ctx);
    return abcFile;
}

}  // namespace ark::ets
