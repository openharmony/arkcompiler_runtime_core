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

#include "plugins/ets/runtime/types/ets_abc_runtime_linker.h"

#include "plugins/ets/runtime/ets_class_linker_context.h"
#include "plugins/ets/runtime/ets_execution_context.h"
#include "plugins/ets/runtime/ets_handle.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_abc_file.h"
#include "plugins/ets/runtime/types/ets_array.h"

namespace ark::ets {

bool EtsAbcRuntimeLinker::PrependAbcFile(EtsExecutionContext *executionCtx, EtsAbcFile *abcFile)
{
    EtsHandleScope scope(executionCtx);
    EtsHandle<EtsAbcRuntimeLinker> linkerHandle(executionCtx, this);
    EtsHandle<EtsAbcFile> abcFileHandle(executionCtx, abcFile);

    auto *ctx = EtsClassLinkerContext::FromCoreType(linkerHandle->GetClassLinkerContext());
    os::memory::LockHolder lock(ctx->GetAbcFilesMutex());

    EtsHandle currentAbcFilesHandle(executionCtx, linkerHandle->GetAbcFiles());
    auto currentLength = currentAbcFilesHandle->GetLength();
    EtsHandle resultAbcFilesHandle(
        executionCtx, EtsObjectArray::Create(PlatformTypes(executionCtx)->arkruntimeAbcFile, currentLength + 1U));
    if (UNLIKELY(resultAbcFilesHandle.GetPtr() == nullptr)) {
        return false;
    }
    resultAbcFilesHandle->Set(0, abcFileHandle.GetPtr());
    for (size_t i = 0; i < currentLength; ++i) {
        resultAbcFilesHandle->Set(i + 1U, currentAbcFilesHandle->Get(i));
    }
    linkerHandle->SetAbcFiles(resultAbcFilesHandle.GetPtr());
    return true;
}

}  // namespace ark::ets
