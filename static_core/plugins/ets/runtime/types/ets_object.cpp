/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/ets_coroutine.h"

namespace ark::ets {

/* static */
EtsObject *EtsObject::Create(EtsCoroutine *etsCoroutine, EtsClass *klass)
{
    ASSERT_HAVE_ACCESS_TO_MANAGED_OBJECTS();
    return static_cast<EtsObject *>(ObjectHeader::Create(etsCoroutine, klass->GetRuntimeClass()));
}

/* static */
EtsObject *EtsObject::Create(EtsClass *klass)
{
    return Create(EtsCoroutine::GetCurrent(), klass);
}

/* static */
EtsObject *EtsObject::CreateNonMovable(EtsClass *klass)
{
    return static_cast<EtsObject *>(ObjectHeader::CreateNonMovable(klass->GetRuntimeClass()));
}

}  // namespace ark::ets
