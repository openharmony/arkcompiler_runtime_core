/**
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "runtime/include/thread_scopes.h"
#include "plugins/ets/runtime/types/ets_shared_memory.h"

#ifndef PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_BUFFER_INL_H_
#define PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_BUFFER_INL_H_

namespace panda::ets {

template <typename F>
std::pair<int8_t, int8_t> EtsSharedMemory::ReadModifyWriteI8(int32_t index, const F &f)
{
    auto coroutine = EtsCoroutine::GetCurrent();
    [[maybe_unused]] EtsHandleScope scope(coroutine);
    EtsHandle<EtsSharedMemory> this_handle(coroutine, this);

    // NOTE(egor-porsev): add LIKELY(std::try_lock) path to prevent ScopedNativeCodeThread creation if no blocking
    // occurs
    ScopedNativeCodeThread n(coroutine);
    os::memory::LockHolder lock(coroutine->GetPandaVM()->GetAtomicsMutex());
    ScopedManagedCodeThread m(coroutine);

    auto old_value = this_handle->GetElement(index);
    auto new_value = f(old_value);
    this_handle->SetElement(index, new_value);

    return std::pair(old_value, new_value);
}

}  // namespace panda::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_BUFFER_INL_H_