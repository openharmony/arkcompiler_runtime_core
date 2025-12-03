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

#ifndef ARK_GUARD_TESTS_UTIL_ASSERT_H
#define ARK_GUARD_TESTS_UTIL_ASSERT_H

#define ASSERT_ARRAY_EQUAL(vec1_, vec2_)                                             \
    do {                                                                             \
        ASSERT_EQ((vec1_).size(), (vec2_).size()) << "vectors have different sizes"; \
        for (size_t i = 0; i < (vec1_).size(); i++) {                                \
            ASSERT_EQ((vec1_)[i], (vec2_)[i]) << "mismatch at index" << i;           \
        }                                                                            \
    } while (false)

#define ASSERT_ABCKIT_WRAPPER_SUCCESS(rc) ASSERT_EQ(rc, AbckitWrapperErrorCode::ERR_SUCCESS)
#define ASSERT_ARK_GUARD_SUCCESS(rc) ASSERT_EQ(rc, ark::guard::ErrorCode::ERR_SUCCESS)

#endif  // ARK_GUARD_TESTS_UTIL_ASSERT_H
