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
#ifndef PANDA_TARGET_UNIX
#define PANDA_TARGET_UNIX
#endif
#include "releasepages_fuzzer.h"
#include "libpandabase/os/mem.h"

namespace OHOS {
    void ReleasePagesFuzzTest([[maybe_unused]] const uint8_t* data, [[maybe_unused]] size_t size)
    {
        // page_start and page_end should be multiple of PAGE_SIZE(4KB)
        uintptr_t page_start = 0x1000UL;
        uintptr_t page_end = 0x2000UL;
        panda::os::mem::ReleasePages(page_start, page_end);
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::ReleasePagesFuzzTest(data, size);
    return 0;
}