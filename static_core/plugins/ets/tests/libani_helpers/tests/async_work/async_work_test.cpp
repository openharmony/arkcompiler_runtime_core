/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"
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

#include "ani_gtest.h"
#include "libani_helpers/concurrency_helpers.h"

namespace ark::ets::ani_helpers::testing {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace arkts::concurrency_helpers;

static void OnExecStub([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {}
static void OnCompleteStub([[maybe_unused]] ani_env *env,
                           [[maybe_unused]] arkts::concurrency_helpers::WorkStatus status, [[maybe_unused]] void *data)
{
}

class AsyncWorkTest : public ark::ets::ani::testing::AniTest {};

TEST_F(AsyncWorkTest, CreateAsyncWorkInvalidArgs)
{
    AsyncWork *work = nullptr;
    ASSERT_EQ(CreateAsyncWork(nullptr, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(CreateAsyncWork(env_, nullptr, OnCompleteStub, nullptr, &work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, nullptr, nullptr, &work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, nullptr), WorkStatus::INVALID_ARGS);
}

TEST_F(AsyncWorkTest, QueueAsyncWorkInvalidArgs)
{
    AsyncWork *work = nullptr;
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::OK);

    ASSERT_EQ(QueueAsyncWork(nullptr, work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(QueueAsyncWork(env_, nullptr), WorkStatus::INVALID_ARGS);
}

TEST_F(AsyncWorkTest, CancelAsyncWorkInvalidArgs)
{
    AsyncWork *work = nullptr;
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::OK);

    ASSERT_EQ(CancelAsyncWork(nullptr, work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(CancelAsyncWork(env_, nullptr), WorkStatus::INVALID_ARGS);
}

TEST_F(AsyncWorkTest, DeleteAsyncWorkInvalidArgs)
{
    AsyncWork *work = nullptr;
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::OK);

    ASSERT_EQ(DeleteAsyncWork(nullptr, work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(DeleteAsyncWork(env_, nullptr), WorkStatus::INVALID_ARGS);
}

}  // namespace ark::ets::ani_helpers::testing
