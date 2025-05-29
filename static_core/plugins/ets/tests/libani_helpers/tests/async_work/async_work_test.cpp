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
#include "include/thread_scopes.h"
#include "coroutines/coroutine_manager.h"

namespace ark::ets::ani_helpers::testing {

// NOLINTNEXTLINE(google-build-using-namespace)
using namespace arkts::concurrency_helpers;

namespace {

void OnExecStub([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {}
void OnCompleteStub([[maybe_unused]] ani_env *env, [[maybe_unused]] arkts::concurrency_helpers::WorkStatus status,
                    [[maybe_unused]] void *data)
{
}

class AsyncWorkTest : public ark::ets::ani::testing::AniTest {};

class MainWorkerAsyncWorkTest : public ark::ets::ani::testing::AniTest {
    std::vector<ani_option> GetExtraAniOptions() override
    {
        return {ani_option {"--ext:ets.coroutine-workers-count=1", nullptr}};
    }
};

class AsyncWorkEvent {
public:
    AsyncWorkEvent() : event_(Coroutine::GetCurrent()->GetManager()) {}

    void Wait()
    {
        event_.Lock();
        Coroutine::GetCurrent()->GetManager()->Await(&event_);
    }

    void Fire()
    {
        event_.Happen();
    }

private:
    GenericEvent event_;
};

}  // namespace

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
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(nullptr, work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(QueueAsyncWork(env_, nullptr), WorkStatus::INVALID_ARGS);

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, CancelAsyncWorkInvalidArgs)
{
    AsyncWork *work = nullptr;
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(CancelAsyncWork(nullptr, work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(CancelAsyncWork(env_, nullptr), WorkStatus::INVALID_ARGS);

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, DeleteAsyncWorkInvalidArgs)
{
    AsyncWork *work = nullptr;
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(DeleteAsyncWork(nullptr, work), WorkStatus::INVALID_ARGS);
    ASSERT_EQ(DeleteAsyncWork(env_, nullptr), WorkStatus::INVALID_ARGS);

    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, QueueAsyncWorkTwiceError)
{
    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    using TestData = std::tuple<AsyncWorkEvent &, AsyncWork **>;
    auto testData = TestData {event, &work};
    auto status = CreateAsyncWork(
        env_, []([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {},
        []([[maybe_unused]] ani_env *env, WorkStatus s, void *data) {
            auto &[e, w] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(s, WorkStatus::OK);
            ASSERT_EQ(DeleteAsyncWork(env, *w), WorkStatus::OK);
            e.Fire();
        },
        (void *)&testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::ERROR);

    event.Wait();
}

TEST_F(AsyncWorkTest, QueueAsyncWork)
{
    auto event = AsyncWorkEvent();
    int testCounter = 0;
    AsyncWork *work = nullptr;
    using TestData = std::tuple<AsyncWorkEvent &, AsyncWork **, int *>;
    auto testData = TestData {event, &work, &testCounter};
    auto status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, void *data) {
            auto &[_, w, counter] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(*counter, 0);
            (*counter)++;
            ASSERT_EQ(*counter, 1);
        },
        []([[maybe_unused]] ani_env *env, WorkStatus s, void *data) {
            auto &[e, w, counter] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(s, WorkStatus::OK);
            ASSERT_EQ(*counter, 1);
            (*counter)++;
            ASSERT_EQ(DeleteAsyncWork(env, *w), WorkStatus::OK);
            e.Fire();
        },
        (void *)&testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    event.Wait();
    ASSERT_EQ(testCounter, 2U);
}

TEST_F(AsyncWorkTest, CancelAsyncWorkTwiceError)
{
    AsyncWork *work = nullptr;
    ASSERT_EQ(CreateAsyncWork(env_, OnExecStub, OnCompleteStub, nullptr, &work), WorkStatus::OK);
    ASSERT_NE(work, nullptr);
    ASSERT_EQ(CancelAsyncWork(env_, work), WorkStatus::OK);
    ASSERT_EQ(CancelAsyncWork(env_, work), WorkStatus::ERROR);
    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

TEST_F(AsyncWorkTest, CancelAsyncWorkBeforeQueued)
{
    AsyncWork *work = nullptr;
    auto status = CreateAsyncWork(
        env_, []([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) { ASSERT_TRUE(false); },
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] WorkStatus s, [[maybe_unused]] void *data) {
            ASSERT_TRUE(false);
        },
        nullptr, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(CancelAsyncWork(env_, work), WorkStatus::OK);
    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::ERROR);
    ASSERT_EQ(DeleteAsyncWork(env_, work), WorkStatus::OK);
}

// This test is launched with single main worker to cancel work after it was queued, but before it was started
TEST_F(MainWorkerAsyncWorkTest, CancelAsyncAfterQueued)
{
    auto event = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    using TestData = std::tuple<AsyncWorkEvent &, AsyncWork **>;
    auto testData = TestData {event, &work};
    auto status = CreateAsyncWork(
        env_, []([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {},
        []([[maybe_unused]] ani_env *env, WorkStatus s, void *data) {
            auto &[e, w] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(s, WorkStatus::CANCELED);
            ASSERT_EQ(DeleteAsyncWork(env, *w), WorkStatus::OK);
            e.Fire();
        },
        (void *)&testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    ASSERT_EQ(CancelAsyncWork(env_, work), WorkStatus::OK);

    event.Wait();
}

TEST_F(AsyncWorkTest, CancelAsyncAfterRunning)
{
    auto eventRun = AsyncWorkEvent();
    auto eventCompleted = AsyncWorkEvent();
    AsyncWork *work = nullptr;
    using TestData = std::tuple<AsyncWorkEvent &, AsyncWorkEvent &, AsyncWork **>;
    auto testData = TestData {eventRun, eventCompleted, &work};
    auto status = CreateAsyncWork(
        env_,
        []([[maybe_unused]] ani_env *env, [[maybe_unused]] void *data) {
            auto &[eR, eC, w] = *reinterpret_cast<TestData *>(data);
            eR.Fire();
        },
        []([[maybe_unused]] ani_env *env, WorkStatus s, void *data) {
            auto &[eR, eC, w] = *reinterpret_cast<TestData *>(data);
            ASSERT_EQ(s, WorkStatus::OK);
            ASSERT_EQ(DeleteAsyncWork(env, *w), WorkStatus::OK);
            eC.Fire();
        },
        (void *)&testData, &work);
    ASSERT_EQ(status, WorkStatus::OK);
    ASSERT_NE(work, nullptr);

    ASSERT_EQ(QueueAsyncWork(env_, work), WorkStatus::OK);
    eventRun.Wait();
    ASSERT_EQ(CancelAsyncWork(env_, work), WorkStatus::ERROR);
    eventCompleted.Wait();
}

}  // namespace ark::ets::ani_helpers::testing
