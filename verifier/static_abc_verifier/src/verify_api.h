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

#ifndef STATIC_ABC_VERIFIER_VERIFY_API_H
#define STATIC_ABC_VERIFIER_VERIFY_API_H

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#ifdef _WIN32
#define SAV_EXPORT __declspec(dllexport)
#else
#define SAV_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Verify a static ABC file from a filesystem path.
 *
 * @param filePath     Null-terminated path to the .abc file.
 * @param errorBuf     Buffer to receive a null-terminated error description.
 *                     Can be NULL if no error details are needed.
 * @param errorBufLen  Size of errorBuf in bytes.
 * @return 0 on success (file is valid), non-zero error code on failure.
 */
SAV_EXPORT int StaticAbcVerifyFile(const char *filePath,
                                   char *errorBuf,
                                   int errorBufLen);

/**
 * Verify a static ABC file from an in-memory buffer.
 *
 * @param data         Pointer to the mapped file contents.
 * @param size         Length of the buffer in bytes.
 * @param errorBuf     Buffer to receive a null-terminated error description.
 * @param errorBufLen  Size of errorBuf in bytes.
 * @return 0 on success; non-zero error code on failure.
 */
SAV_EXPORT int StaticAbcVerifyMemory(const unsigned char *data,
                                     size_t size,
                                     char *errorBuf,
                                     int errorBufLen);

#ifdef __cplusplus
}
#endif

#endif  // STATIC_ABC_VERIFIER_VERIFY_API_H
