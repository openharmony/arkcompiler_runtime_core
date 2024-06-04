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

#include "hash_fuzzer.h"

#include "utils/hash.h"

namespace OHOS {
    void HashFuzzTest(const uint8_t* data, size_t size)
    {
        panda::GetHash32(data, size);

        std::string str(data, data + size);
        uint8_t* string_data = reinterpret_cast<uint8_t*>(const_cast<char*>(str.c_str()));
        panda::GetHash32String(string_data);

        panda::merge_hashes(size, size);

        uint32_t lhash_uint32_t = static_cast<uint32_t>(size);
        uint32_t rhash_uint32_t = static_cast<uint32_t>(size);
        panda::merge_hashes(lhash_uint32_t, rhash_uint32_t);
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::HashFuzzTest(data, size);
    return 0;
}

