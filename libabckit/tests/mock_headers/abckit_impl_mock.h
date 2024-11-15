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

#include "mock_global_values.h"

#include "../../src/ir_impl.h"
#include "../../include/c/metadata_core.h"
#include "../../include/c/statuses.h"
#include "../../src/logger.h"
#include "../../src/metadata_inspect_impl.h"

#include "../../include/c/abckit.h"

#include <cstring>
#include <gtest/gtest.h>

namespace libabckit::mock {

// NOLINTBEGIN(readability-identifier-naming)

inline AbckitStatus getLastError()
{
    LIBABCKIT_IMPLEMENTED;
    return libabckit::statuses::GetLastError();
}

inline AbckitFile *openAbc(const char *path)
{
    g_calledFuncs.push(LIBABCKIT_FUNC_NAME);
    EXPECT_TRUE(strncmp(path, DEFAULT_PATH, sizeof(DEFAULT_PATH)) == 0);
    return DEFAULT_FILE;
}

inline void writeAbc(AbckitFile *file, const char *path)
{
    g_calledFuncs.push(LIBABCKIT_FUNC_NAME);
    EXPECT_TRUE(strncmp(path, DEFAULT_PATH, sizeof(DEFAULT_PATH)) == 0);
    EXPECT_TRUE(file == DEFAULT_FILE);
}

inline void closeFile(AbckitFile *file)
{
    g_calledFuncs.push(LIBABCKIT_FUNC_NAME);
    EXPECT_TRUE(file == DEFAULT_FILE);
}

inline void destroyGraph(AbckitGraph *graph)
{
    g_calledFuncs.push(LIBABCKIT_FUNC_NAME);
    EXPECT_TRUE(graph == DEFAULT_GRAPH);
}

// NOLINTEND(readability-identifier-naming)

static AbckitApi g_impl = {
    // ========================================
    // Common API
    // ========================================

    ABCKIT_VERSION_RELEASE_1_0_0,
    getLastError,

    // ========================================
    // Inspection API entrypoints
    // ========================================

    openAbc,
    writeAbc,
    closeFile,

    // // ========================================
    // // IR API entrypoints
    // // ========================================

    destroyGraph,
};

}  // namespace libabckit::mock

inline extern AbckitApi const *AbckitGetMockApiImpl(AbckitApiVersion version)
{
    switch (version) {
        case ABCKIT_VERSION_RELEASE_1_0_0:
            return &libabckit::mock::g_impl;
        default:
            libabckit::statuses::SetLastError(ABCKIT_STATUS_UNKNOWN_API_VERSION);
            return nullptr;
    }
}

#endif
