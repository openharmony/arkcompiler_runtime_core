/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

#include "utils/workerQueue.h"

namespace panda::test {

class WorkerQueueTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Concrete implementation of WorkerQueue for testing
class TestWorkerQueue : public WorkerQueue {
public:
    explicit TestWorkerQueue(size_t threadCount) : WorkerQueue(threadCount) {}
    void Schedule() override {}

    // Hold m_ so the push + jobsCount_ update cannot race with worker threads,
    // which read jobsCount_/jobs_ inside WorkerQueue::Consume.
    void AddJob(WorkerJob *job)
    {
        std::lock_guard<std::mutex> lock(m_);
        jobs_.push_back(job);
        jobsCount_++;
    }

    // Batch-add all jobs under a single lock so that jobsCount_ and jobs_.size()
    // stay in sync.  This prevents a worker thread from consuming a job (and
    // decrementing jobsCount_) while more jobs are still being added, which
    // would cause jobsCount_ to diverge from jobs_.size() and make Consume()
    // skip the last-added job.
    void AddJobs(std::vector<WorkerJob *> &newJobs)
    {
        std::lock_guard<std::mutex> lock(m_);
        for (auto *job : newJobs) {
            jobs_.push_back(job);
            jobsCount_++;
        }
    }
};

// Test job that counts executions
class CountingJob : public WorkerJob {
public:
    explicit CountingJob(std::atomic<int>& counter) : counter_(counter) {}
    bool Run() override
    {
        counter_++;
        return true;
    }
private:
    std::atomic<int>& counter_;
};

// Test job that waits for signal
class WaitingJob : public WorkerJob {
public:
    explicit WaitingJob() {}
    bool Run() override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return true;
    }
};

HWTEST_F(WorkerQueueTest, BasicJobExecution, testing::ext::TestSize.Level0)
{
    std::atomic<int> counter(0);
    {
        TestWorkerQueue *queue = new TestWorkerQueue(2);
        auto* job = new CountingJob(counter);
        queue->AddJob(job);
        queue->Consume();
        queue->Wait();
    }
    EXPECT_EQ(counter, 1);
}

HWTEST_F(WorkerQueueTest, MultipleJobs, testing::ext::TestSize.Level0)
{
    std::atomic<int> counter(0);
    {
        TestWorkerQueue *queue = new TestWorkerQueue(2);
        for (int i = 0; i < 5; i++) {
            queue->AddJob(new CountingJob(counter));
        }
        queue->Consume();
        queue->Wait();
        delete queue;
        queue = nullptr;
    }
    EXPECT_EQ(counter, 5);
}

HWTEST_F(WorkerQueueTest, JobDependencies, testing::ext::TestSize.Level0)
{
    std::atomic<int> counter(0);
    {
        TestWorkerQueue *queue = new TestWorkerQueue(2);
        
        auto* job1 = new CountingJob(counter);
        auto* job2 = new CountingJob(counter);
        job2->DependsOn(job1);
        
        queue->AddJob(job1);
        queue->AddJob(job2);
        queue->Consume();
        queue->Wait();
        delete queue;
        queue = nullptr;
    }
    EXPECT_EQ(counter, 2);
}

HWTEST_F(WorkerQueueTest, JobSignal, testing::ext::TestSize.Level0)
{
    std::atomic<int> counter(0);
    {
        TestWorkerQueue *queue = new TestWorkerQueue(1);
    
        auto* job1 = new CountingJob(counter);
        auto* job2 = new CountingJob(counter);
        job2->DependsOn(job1);
        
        queue->AddJob(job1);
        queue->AddJob(job2);
        
        job1->Signal();
        queue->Consume();
        queue->Wait();
        delete queue;
        queue = nullptr;
    }
    EXPECT_EQ(counter, 2);
}

HWTEST_F(WorkerQueueTest, ConcurrentJobs, testing::ext::TestSize.Level0)
{
    std::atomic<int> counter(0);
    {
        TestWorkerQueue *queue = new TestWorkerQueue(8);

        // Batch-add all jobs under a single lock so that a worker thread
        // cannot consume a job before all jobs are added (which would
        // desync jobsCount_ from jobs_.size() and skip the last job).
        std::vector<WorkerJob *> jobs;
        jobs.push_back(new WaitingJob());
        for (int i = 0; i < 30; i++) {
            jobs.push_back(new CountingJob(counter));
        }
        queue->AddJobs(jobs);

        queue->Consume();
        queue->Wait();
        delete queue;
        queue = nullptr;
    }
    EXPECT_EQ(counter, 30);
}
}  // namespace panda::test