/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "readwritelockholder_fuzzer.h"
#include "libpandabase/os/mutex.h"

namespace OHOS {
    void ReadWriteLockHolderFuzzTest([[maybe_unused]] const uint8_t* data, [[maybe_unused]] size_t size)
    {
        panda::os::memory::DummyLock lock;
        {
            // test constructor and destructor function of LockHolder
            panda::os::memory::LockHolder<panda::os::memory::DummyLock> lock_holder(lock);
        }
        {
            // test constructor and destructor function of ReadLockHolder
            panda::os::memory::ReadLockHolder<panda::os::memory::DummyLock> read_lock_holder(lock);
        }
        {
            // test constructor and destructor function of WriteLockHolder
            panda::os::memory::WriteLockHolder<panda::os::memory::DummyLock> write_lock_holder(lock);
        }
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::ReadWriteLockHolderFuzzTest(data, size);
    return 0;
}