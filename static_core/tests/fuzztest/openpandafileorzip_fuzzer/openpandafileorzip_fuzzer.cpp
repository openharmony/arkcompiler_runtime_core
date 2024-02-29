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

#include "openpandafileorzip_fuzzer.h"
#include "libpandafile/file.h"
#include "libziparchive/zip_archive.h"

namespace OHOS {
void OpenPandaFileOrZipFuzzTest(const uint8_t *data, size_t size)
{
    const char *filename1 = panda::panda_file::ARCHIVE_FILENAME;
    const char *filename2 = "classes1.abc";
    // Create uncompressed zip file
    const char *uncompressZipFilename = "__OpenPandaFileOrZipFuzzTest_uncompress.zip";
    int ret1 = panda::CreateOrAddFileIntoZip(uncompressZipFilename, filename1, data, size, APPEND_STATUS_CREATE,
                                             Z_NO_COMPRESSION);
    int ret2 = panda::CreateOrAddFileIntoZip(uncompressZipFilename, filename2, data, size, APPEND_STATUS_ADDINZIP,
                                             Z_NO_COMPRESSION);
    if (ret1 != 0 || ret2 != 0) {
        (void)remove(uncompressZipFilename);
        return;
    }
    // Create compressed zip file
    const char *compressedZipFilename = "__OpenPandaFileOrZipFuzzTest_compressed.zip";
    ret1 = panda::CreateOrAddFileIntoZip(uncompressZipFilename, filename1, data, size, APPEND_STATUS_CREATE,
                                         Z_BEST_COMPRESSION);
    ret2 = panda::CreateOrAddFileIntoZip(uncompressZipFilename, filename2, data, size, APPEND_STATUS_ADDINZIP,
                                         Z_BEST_COMPRESSION);
    if (ret1 != 0 || ret2 != 0) {
        (void)remove(compressedZipFilename);
        return;
    }

    {
        panda::panda_file::OpenPandaFileOrZip(uncompressZipFilename);
        panda::panda_file::OpenPandaFileOrZip(compressedZipFilename);
    }
    (void)remove(uncompressZipFilename);
    (void)remove(compressedZipFilename);
}
}  // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::OpenPandaFileOrZipFuzzTest(data, size);
    return 0;
}
