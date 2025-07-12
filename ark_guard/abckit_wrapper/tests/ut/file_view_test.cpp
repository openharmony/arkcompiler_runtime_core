/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "abckit_wrapper/file_view.h"

using namespace testing::ext;

/**
 * @tc.name: fileView_namespace_001
 * @tc.desc: test fileView init failed
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST(TestFileView, fileView_namespace_001, TestSize.Level0)
{
    abckit_wrapper::FileView fileView;
    ASSERT_NE(fileView.Init(""), 0);
}
