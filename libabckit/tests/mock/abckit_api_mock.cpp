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

#include "mock/abckit_mock.h"
#include "mock/mock_values.h"

#include "include/c/metadata_core.h"
#include "include/c/statuses.h"
#include "include/c/abckit.h"

#include <gtest/gtest.h>

namespace libabckit::mock {

// NOLINTBEGIN(readability-identifier-naming)

inline AbckitStatus getLastError()
{
    return ABCKIT_STATUS_NO_ERROR;
}

inline AbckitFile *openAbc(const char *path)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(strncmp(path, DEFAULT_PATH, sizeof(DEFAULT_PATH)) == 0);
    return DEFAULT_FILE;
}

inline void writeAbc(AbckitFile *file, const char *path)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(strncmp(path, DEFAULT_PATH, sizeof(DEFAULT_PATH)) == 0);
    EXPECT_TRUE(file == DEFAULT_FILE);
}

inline void closeFile(AbckitFile *file)
{
    g_calledFuncs.push(__func__);
    EXPECT_TRUE(file == DEFAULT_FILE);
}

inline void destroyGraph(AbckitGraph *graph)
{
    g_calledFuncs.push(__func__);
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

    // ========================================
    // IR API entrypoints
    // ========================================

    destroyGraph,
};

}  // namespace libabckit::mock

AbckitApi const *AbckitGetMockApiImpl([[maybe_unused]] AbckitApiVersion version)
{
    return &libabckit::mock::g_impl;
}
