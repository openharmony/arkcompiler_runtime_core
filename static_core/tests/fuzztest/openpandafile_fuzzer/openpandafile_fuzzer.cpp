/**
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "openpandafile_fuzzer.h"
#include "libpandafile/file.h"
#include "libziparchive/zip_archive.h"

namespace OHOS {
void OpenPandaFileFuzzTest(const uint8_t *data, size_t size)
{
    // Create zip file
    const char *filename1 = panda::panda_file::ARCHIVE_FILENAME;
    const char *filename2 = "classes1.abc";

    const char *zipFilename1 = "__OpenPandaFileFuzzTest.zip";
    int ret1 =
        panda::CreateOrAddFileIntoZip(zipFilename1, filename1, data, size, APPEND_STATUS_CREATE, Z_BEST_COMPRESSION);
    int ret2 =
        panda::CreateOrAddFileIntoZip(zipFilename1, filename2, data, size, APPEND_STATUS_ADDINZIP, Z_BEST_COMPRESSION);
    if (ret1 != 0 || ret2 != 0) {
        (void)remove(zipFilename1);
        return;
    }

    const char *zipFilename2 = "__OpenPandaFileFromZipNameAnonMem.zip";
    int ret3 =
        panda::CreateOrAddFileIntoZip(zipFilename2, filename1, data, size, APPEND_STATUS_CREATE, Z_BEST_COMPRESSION);
    if (ret3 != 0) {
        (void)remove(zipFilename2);
        return;
    }

    // Call OpenPandaFile
    {
        panda::panda_file::OpenPandaFile(zipFilename1);
        panda::panda_file::OpenPandaFile(zipFilename2);
    }
    (void)remove(zipFilename1);
    (void)remove(zipFilename2);
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::OpenPandaFileFuzzTest(data, size);
    return 0;
}
