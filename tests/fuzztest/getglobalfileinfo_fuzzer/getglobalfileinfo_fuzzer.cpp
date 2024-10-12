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

#include "getglobalfileinfo_fuzzer.h"
#include "libpandafile/file.h"
#include "libziparchive/zip_archive.h"

namespace OHOS {
    void GetGlobalFileInfoFuzzTest(const uint8_t* data, size_t size)
    {
        {
            // handle is nullptr
            panda::ZipArchiveHandle handle = nullptr;
            panda::GlobalStat gi = panda::GlobalStat();
            if (panda::GetGlobalFileInfo(handle, &gi) != 0) {
                return;
            }
        }

        {
            // handle is not nullptr
            // Create zip file
            const char* filename = "classes1.abc";
            const char* zipname = "__OpenPandaFileFuzzTest.zip";
            int ret =
                panda::CreateOrAddFileIntoZip(zipname, filename, data, size, APPEND_STATUS_CREATE,
                                              Z_BEST_COMPRESSION);
            if (ret != 0) {
                return;
            }
            // Quick Check
            panda::ZipArchiveHandle handle = nullptr;
            if (panda::OpenArchive(handle, zipname) != 0) {
                return;
            }
            panda::GlobalStat gi = panda::GlobalStat();
            if (panda::GetGlobalFileInfo(handle, &gi) != 0) {
                return;
            }
            panda::CloseArchive(handle);
            (void)remove(zipname);
        }
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::GetGlobalFileInfoFuzzTest(data, size);
    return 0;
}