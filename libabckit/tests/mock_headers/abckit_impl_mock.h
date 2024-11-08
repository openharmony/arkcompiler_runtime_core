/*
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

#ifndef ABCKIT_IMPL_MOCK
#define ABCKIT_IMPL_MOCK

#include "ir_impl.h"
#include "libabckit/include/c/metadata_core.h"
#include "libabckit/include/c/statuses.h"
#include "logger.h"
#include "metadata_inspect_impl.h"

#include <cstring>

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFAULT_PATH "abckit.abc"
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAUIT_FILE ((AbckitFile *)0xdeadffff)
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage, cppcoreguidelines-pro-type-cstyle-cast)
#define DEFAUIT_GRAPH ((AbckitGraph *)0xdeadeeee)

namespace libabckit {

inline AbckitStatus GetLastErrorMock()
{
    LIBABCKIT_IMPLEMENTED;
    return libabckit::statuses::GetLastError();
}

inline AbckitFile *OpenAbcMock(const char *path)
{
    LIBABCKIT_LOG(DEBUG) << "Tested " << LIBABCKIT_FUNC_NAME << "\n";
    ASSERT(strncmp(path, DEFAULT_PATH, sizeof(DEFAULT_PATH)) == 0);
    (void)path;
    return DEFAUIT_FILE;
}

inline void WriteAbcMock(AbckitFile *file, const char *path)
{
    LIBABCKIT_LOG(DEBUG) << "Tested " << LIBABCKIT_FUNC_NAME << "\n";
    ASSERT(strncmp(path, DEFAULT_PATH, sizeof(DEFAULT_PATH)) == 0);
    ASSERT(file == DEFAUIT_FILE);
    (void)file;
    (void)path;
}

inline void CloseFileMock(AbckitFile *file)
{
    LIBABCKIT_LOG(DEBUG) << "Tested " << LIBABCKIT_FUNC_NAME << "\n";
    ASSERT(file == DEFAUIT_FILE);
    (void)file;
}

inline void DestroyGraphMock(AbckitGraph *graph)
{
    LIBABCKIT_LOG(DEBUG) << "Tested " << LIBABCKIT_FUNC_NAME << "\n";
    ASSERT(graph == DEFAUIT_GRAPH);
    (void)graph;
}

static AbckitApi g_impl = {
    // ========================================
    // Common API
    // ========================================

    ABCKIT_VERSION_RELEASE_1_0_0,
    GetLastErrorMock,

    // ========================================
    // Inspection API entrypoints
    // ========================================

    OpenAbcMock,
    WriteAbcMock,
    CloseFileMock,

    // // ========================================
    // // IR API entrypoints
    // // ========================================

    DestroyGraphMock,
};

}  // namespace libabckit

inline extern AbckitApi const *AbckitGetMockApiImpl(AbckitApiVersion version)
{
    switch (version) {
        case ABCKIT_VERSION_RELEASE_1_0_0:
            return &libabckit::g_impl;
        default:
            libabckit::statuses::SetLastError(ABCKIT_STATUS_UNKNOWN_API_VERSION);
            return nullptr;
    }
}

#endif
