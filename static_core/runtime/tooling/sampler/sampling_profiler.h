/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLING_PROFILER_H_
#define PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLING_PROFILER_H_

#include <csignal>

#include "libpandabase/macros.h"
#include "platforms/unix/libpandabase/pipe.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/mem/panda_containers.h"

#include "runtime/tooling/sampler/sample_info.h"
#include "runtime/tooling/sampler/sample_writer.h"
#include "runtime/tooling/sampler/thread_communicator.h"
#include "runtime/tooling/sampler/lock_free_queue.h"

namespace panda::tooling::sampler {

namespace test {
class SamplerTest;
}  // namespace test

// Panda sampling profiler
class Sampler final : public RuntimeListener {
public:
    ~Sampler() override = default;

    PANDA_PUBLIC_API static Sampler *Create();
    PANDA_PUBLIC_API static void Destroy(Sampler *sampler);

    // Need to get comunicator inside the signal handler
    static const ThreadCommunicator &GetSampleCommunicator()
    {
        ASSERT(instance_ != nullptr);
        return instance_->GetCommunicator();
    }

    const ThreadCommunicator &GetCommunicator() const
    {
        return communicator_;
    }

    static const LockFreeQueue &GetSampleQueuePF()
    {
        ASSERT(instance_ != nullptr);
        return instance_->GetQueuePF();
    }

    const LockFreeQueue &GetQueuePF()
    {
        return loaded_pfs_queue_;
    }

    void SetSampleInterval(uint32_t us)
    {
        ASSERT(is_active_ == false);
        sample_interval_ = static_cast<std::chrono::microseconds>(us);
    }

    void SetSegvHandlerStatus(bool segv_handler_status)
    {
        is_segv_handler_enable_ = segv_handler_status;
    }

    bool IsSegvHandlerEnable() const
    {
        return is_segv_handler_enable_;
    }

    PANDA_PUBLIC_API bool Start(const char *filename);
    PANDA_PUBLIC_API void Stop();

    // Events: Notify profiler that managed thread created or finished
    void ThreadStart(ManagedThread *managed_thread) override;
    void ThreadEnd(ManagedThread *managed_thread) override;
    void LoadModule(std::string_view name) override;

    static constexpr uint32_t DEFAULT_SAMPLE_INTERVAL_US = 500;

private:
    Sampler();

    void SamplerThreadEntry();
    void ListenerThreadEntry(std::string output_file);

    void AddThreadHandle(ManagedThread *thread);
    void EraseThreadHandle(ManagedThread *thread);

    void CollectThreads();
    void CollectModules();

    void WriteLoadedPandaFiles(StreamWriter *writer_ptr);

    void ClearManagedThreadSet()
    {
        os::memory::LockHolder holder(managed_threads_lock_);
        managed_threads_.clear();
    }

    void ClearLoadedPfs()
    {
        os::memory::LockHolder holder(loaded_pfs_lock_);
        loaded_pfs_.clear();
    }

    static Sampler *instance_;

    Runtime *runtime_ {nullptr};
    // Remember agent thread id for security
    os::thread::NativeHandleType listener_tid_ {0};
    os::thread::NativeHandleType sampler_tid_ {0};
    std::unique_ptr<std::thread> sampler_thread_ {nullptr};
    std::unique_ptr<std::thread> listener_thread_ {nullptr};
    ThreadCommunicator communicator_;

    std::atomic<bool> is_active_ {false};
    bool is_segv_handler_enable_ {true};

    PandaSet<os::thread::ThreadId> managed_threads_ GUARDED_BY(managed_threads_lock_);
    os::memory::Mutex managed_threads_lock_;

    LockFreeQueue loaded_pfs_queue_;

    PandaVector<FileInfo> loaded_pfs_ GUARDED_BY(loaded_pfs_lock_);
    os::memory::Mutex loaded_pfs_lock_;

    std::chrono::microseconds sample_interval_;

    friend class test::SamplerTest;

    NO_COPY_SEMANTIC(Sampler);
    NO_MOVE_SEMANTIC(Sampler);
};

}  // namespace panda::tooling::sampler

#endif  // PANDA_RUNTIME_TOOLING_SAMPLER_SAMPLING_PROFILER_H_
