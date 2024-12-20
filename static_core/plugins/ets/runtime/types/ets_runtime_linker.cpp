/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/types/ets_runtime_linker.h"

#include "plugins/ets/runtime/ets_vm.h"
#include "plugins/ets/runtime/types/ets_class.h"

namespace ark::ets {

/* static */
EtsRuntimeLinker *EtsRuntimeLinker::Create(ClassLinkerContext *ctx)
{
    ASSERT(ctx->GetRefToLinker() == nullptr);
    auto *klass = PandaEtsVM::GetCurrent()->GetClassLinker()->GetRuntimeLinkerClass();
    auto *etsObject = EtsObject::Create(klass);
    auto *runtimeLinker = EtsRuntimeLinker::FromEtsObject(etsObject);
    runtimeLinker->SetClassLinkerContext(ctx);
    return runtimeLinker;
}

}  // namespace ark::ets
