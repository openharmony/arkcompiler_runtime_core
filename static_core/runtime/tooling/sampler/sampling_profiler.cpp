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

#include <sys/syscall.h>

#include "libpandabase/macros.h"
#include "os/thread.h"
#include "runtime/tooling/sampler/sampling_profiler.h"
#include "runtime/include/managed_thread.h"
#include "runtime/thread_manager.h"
#include "runtime/tooling/sampler/stack_walker_base.h"
#include "runtime/tooling/pt_thread_info.h"
#include "runtime/signal_handler.h"
#include "runtime/coroutines/coroutine.h"

namespace panda::tooling::sampler {

static std::atomic<int> S_CURRENT_HANDLERS_COUNTER = 0;

/* static */
Sampler *Sampler::instance_ = nullptr;

static std::atomic<size_t> S_LOST_SAMPLES = 0;
static std::atomic<size_t> S_LOST_SEGV_SAMPLES = 0;
static std::atomic<size_t> S_LOST_INVALID_SAMPLES = 0;
static std::atomic<size_t> S_LOST_NOT_FIND_SAMPLES = 0;
static std::atomic<size_t> S_TOTAL_SAMPLES = 0;

class ScopedThreadSampling {
public:
    explicit ScopedThreadSampling(ThreadSamplingInfo *sampling_info) : sampling_info_(sampling_info)
    {
        ASSERT(sampling_info_ != nullptr);
        ASSERT(sampling_info_->IsThreadSampling() == false);
        sampling_info_->SetThreadSampling(true);
    }

    ~ScopedThreadSampling()
    {
        ASSERT(sampling_info_->IsThreadSampling() == true);
        sampling_info_->SetThreadSampling(false);
    }

private:
    ThreadSamplingInfo *sampling_info_;

    NO_COPY_SEMANTIC(ScopedThreadSampling);
    NO_MOVE_SEMANTIC(ScopedThreadSampling);
};

class ScopedHandlersCounting {
public:
    explicit ScopedHandlersCounting()
    {
        ++S_CURRENT_HANDLERS_COUNTER;
    }

    ~ScopedHandlersCounting()
    {
        --S_CURRENT_HANDLERS_COUNTER;
    }

    NO_COPY_SEMANTIC(ScopedHandlersCounting);
    NO_MOVE_SEMANTIC(ScopedHandlersCounting);
};

/* static */
Sampler *Sampler::Create()
{
    /*
     * Sampler can be created only once and managed by one thread
     * Runtime::Tools owns it ptr after it's created
     */
    ASSERT(instance_ == nullptr);
    instance_ = new Sampler;

    /**
     * As soon as the sampler is created, we subscribe to the events
     * This is done so that start and stop do not depend on the runtime
     * Internal issue #13780
     */
    ASSERT(Runtime::GetCurrent() != nullptr);

    Runtime::GetCurrent()->GetNotificationManager()->AddListener(instance_,
                                                                 RuntimeNotificationManager::Event::THREAD_EVENTS);
    Runtime::GetCurrent()->GetNotificationManager()->AddListener(instance_,
                                                                 RuntimeNotificationManager::Event::LOAD_MODULE);
    /**
     * Collect threads and modules which were created before sampler start
     * If we collect them before add listeners then new thread can be created (or new module can be loaded)
     * so we will lose this thread (or module)
     */
    instance_->CollectThreads();
    instance_->CollectModules();

    return Sampler::instance_;
}

/* static */
void Sampler::Destroy(Sampler *sampler)
{
    ASSERT(instance_ != nullptr);
    ASSERT(instance_ == sampler);
    ASSERT(!sampler->is_active_);

    LOG(INFO, PROFILER) << "Total samples: " << S_TOTAL_SAMPLES << "\nLost samples: " << S_LOST_SAMPLES;
    LOG(INFO, PROFILER) << "Lost samples(Invalid method ptr): " << S_LOST_INVALID_SAMPLES
                        << "\nLost samples(Invalid pf ptr): " << S_LOST_NOT_FIND_SAMPLES;
    LOG(INFO, PROFILER) << "Lost samples(SIGSEGV occured): " << S_LOST_SEGV_SAMPLES;

    Runtime::GetCurrent()->GetNotificationManager()->RemoveListener(instance_,
                                                                    RuntimeNotificationManager::Event::THREAD_EVENTS);
    Runtime::GetCurrent()->GetNotificationManager()->RemoveListener(instance_,
                                                                    RuntimeNotificationManager::Event::LOAD_MODULE);

    instance_->ClearManagedThreadSet();
    instance_->ClearLoadedPfs();

    delete sampler;
    instance_ = nullptr;
}

Sampler::Sampler() : runtime_(Runtime::GetCurrent()), sample_interval_(DEFAULT_SAMPLE_INTERVAL_US)
{
    ASSERT_NATIVE_CODE();
}

void Sampler::AddThreadHandle(ManagedThread *thread)
{
    os::memory::LockHolder holder(managed_threads_lock_);
    managed_threads_.insert(thread->GetId());
}

void Sampler::EraseThreadHandle(ManagedThread *thread)
{
    os::memory::LockHolder holder(managed_threads_lock_);
    managed_threads_.erase(thread->GetId());
}

void Sampler::ThreadStart(ManagedThread *managed_thread)
{
    AddThreadHandle(managed_thread);
}

void Sampler::ThreadEnd(ManagedThread *managed_thread)
{
    EraseThreadHandle(managed_thread);
}

void Sampler::LoadModule(std::string_view name)
{
    auto callback = [this, name](const panda_file::File &pf) {
        if (pf.GetFilename() == name) {
            auto ptr_id = reinterpret_cast<uintptr_t>(&pf);
            FileInfo pf_module;
            pf_module.ptr = ptr_id;
            pf_module.pathname = pf.GetFullFileName();
            pf_module.checksum = pf.GetHeader()->checksum;
            if (!loaded_pfs_queue_.FindValue(ptr_id)) {
                loaded_pfs_queue_.Push(pf_module);
            }
            os::memory::LockHolder holder(loaded_pfs_lock_);
            this->loaded_pfs_.push_back(pf_module);
            return false;
        }
        return true;
    };
    runtime_->GetClassLinker()->EnumeratePandaFiles(callback, false);
}

bool Sampler::Start(const char *filename)
{
    if (is_active_) {
        LOG(ERROR, PROFILER) << "Attemp to start sampling profiler while it's already started";
        return false;
    }

    if (UNLIKELY(!communicator_.Init())) {
        LOG(ERROR, PROFILER) << "Failed to create pipes for sampling listener. Profiler cannot be started";
        return false;
    }

    is_active_ = true;
    // Creating std::string instead of sending pointer to avoid UB stack-use-after-scope
    listener_thread_ = std::make_unique<std::thread>(&Sampler::ListenerThreadEntry, this, std::string(filename));
    listener_tid_ = listener_thread_->native_handle();

    // All prepairing actions should be done before this thread is started
    sampler_thread_ = std::make_unique<std::thread>(&Sampler::SamplerThreadEntry, this);
    sampler_tid_ = sampler_thread_->native_handle();

    return true;
}

void Sampler::Stop()
{
    if (!is_active_) {
        LOG(ERROR, PROFILER) << "Attemp to stop sampling profiler, but it was not started";
        return;
    }
    if (!sampler_thread_->joinable()) {
        LOG(FATAL, PROFILER) << "Sampling profiler thread unexpectedly disappeared";
        UNREACHABLE();
    }
    if (!listener_thread_->joinable()) {
        LOG(FATAL, PROFILER) << "Listener profiler thread unexpectedly disappeared";
        UNREACHABLE();
    }

    is_active_ = false;
    sampler_thread_->join();
    listener_thread_->join();

    // After threads are stopped we can clear all sampler info
    sampler_thread_.reset();
    listener_thread_.reset();
    sampler_tid_ = 0;
    listener_tid_ = 0;
}

void Sampler::WriteLoadedPandaFiles(StreamWriter *writer_ptr)
{
    os::memory::LockHolder holder(loaded_pfs_lock_);
    if (LIKELY(loaded_pfs_.empty())) {
        return;
    }
    for (const auto &module : loaded_pfs_) {
        if (!writer_ptr->IsModuleWritten(module)) {
            writer_ptr->WriteModule(module);
        }
    }
    loaded_pfs_.clear();
}

void Sampler::CollectThreads()
{
    auto t_manager = runtime_->GetPandaVM()->GetThreadManager();
    if (UNLIKELY(t_manager == nullptr)) {
        // NOTE(m.strizhak): make it for languages without thread_manager
        LOG(FATAL, PROFILER) << "Thread manager is nullptr";
        UNREACHABLE();
    }

    t_manager->EnumerateThreads(
        [this](ManagedThread *thread) {
            AddThreadHandle(thread);
            return true;
        },
        static_cast<unsigned int>(EnumerationFlag::ALL), static_cast<unsigned int>(EnumerationFlag::VM_THREAD));
}

void Sampler::CollectModules()
{
    auto callback = [this](const panda_file::File &pf) {
        auto ptr_id = reinterpret_cast<uintptr_t>(&pf);
        FileInfo pf_module;

        pf_module.ptr = ptr_id;
        pf_module.pathname = pf.GetFullFileName();
        pf_module.checksum = pf.GetHeader()->checksum;

        if (!loaded_pfs_queue_.FindValue(ptr_id)) {
            loaded_pfs_queue_.Push(pf_module);
        }

        os::memory::LockHolder holder(loaded_pfs_lock_);
        this->loaded_pfs_.push_back(pf_module);

        return true;
    };
    runtime_->GetClassLinker()->EnumeratePandaFiles(callback, false);
}

void SigProfSamplingProfilerHandler([[maybe_unused]] int signum, [[maybe_unused]] siginfo_t *siginfo,
                                    [[maybe_unused]] void *ptr)
{
    if (S_CURRENT_HANDLERS_COUNTER == 0) {
        // Sampling ended if S_CURRENT_HANDLERS_COUNTER is 0. Thread started executing handler for signal
        // that was sent before end, so thread is late now and we should return from handler
        return;
    }
    auto scoped_handlers_counting = ScopedHandlersCounting();

    ManagedThread *mthread = ManagedThread::GetCurrent();
    ASSERT(mthread != nullptr);

    // Checking that code is being executed
    auto frame_ptr = reinterpret_cast<CFrame::SlotType *>(mthread->GetCurrentFrame());
    if (frame_ptr == nullptr) {
        return;
    }

    auto frame = mthread->GetCurrentFrame();
    bool is_compiled = mthread->IsCurrentFrameCompiled();
    auto thread_status = mthread->GetStatus();

    bool is_coroutine_running = false;
    if (Coroutine::ThreadIsCoroutine(mthread)) {
        is_coroutine_running = Coroutine::CastFromThread(mthread)->GetCoroutineStatus() == Coroutine::Status::RUNNING;
    }

    Method *top_method = frame->GetMethod();

    S_TOTAL_SAMPLES++;

    const LockFreeQueue &pfs_queue = Sampler::GetSampleQueuePF();

    SampleInfo sample {};
    // Volatile because we don't need to optimize this variable to be able to use setjmp without clobbering
    // Optimized variables may end up with incorrect value as a consequence of a longjmp() operation
    volatile size_t stack_counter = 0;

    ScopedThreadSampling scoped_thread_sampling(mthread->GetPtThreadInfo()->GetSamplingInfo());

    // NOLINTNEXTLINE(cert-err52-cpp)
    if (setjmp(mthread->GetPtThreadInfo()->GetSamplingInfo()->GetSigSegvJmpEnv()) != 0) {
        // This code executed after longjmp()
        // In case of SIGSEGV we lose the sample
        S_LOST_SAMPLES++;
        S_LOST_SEGV_SAMPLES++;
        return;
    }

    if (thread_status == ThreadStatus::RUNNING) {
        sample.thread_info.thread_status = SampleInfo::ThreadStatus::RUNNING;
    } else if (thread_status == ThreadStatus::NATIVE && is_coroutine_running) {
        sample.thread_info.thread_status = SampleInfo::ThreadStatus::RUNNING;
    } else {
        sample.thread_info.thread_status = SampleInfo::ThreadStatus::SUSPENDED;
    }

    if (StackWalkerBase::IsMethodInBoundaryFrame(top_method)) {
        bool is_frame_boundary = true;
        while (is_frame_boundary) {
            Method *method = frame->GetMethod();
            Frame *prev = frame->GetPrevFrame();

            if (StackWalkerBase::IsMethodInI2CFrame(method)) {
                sample.stack_info.managed_stack[stack_counter].panda_file_ptr =
                    helpers::ToUnderlying(FrameKind::BRIDGE);
                sample.stack_info.managed_stack[stack_counter].file_id = helpers::ToUnderlying(FrameKind::BRIDGE);
                ++stack_counter;

                frame = prev;
                is_compiled = false;
            } else if (StackWalkerBase::IsMethodInC2IFrame(method)) {
                sample.stack_info.managed_stack[stack_counter].panda_file_ptr =
                    helpers::ToUnderlying(FrameKind::BRIDGE);
                sample.stack_info.managed_stack[stack_counter].file_id = helpers::ToUnderlying(FrameKind::BRIDGE);
                ++stack_counter;

                frame = prev;
                is_compiled = true;
            } else if (StackWalkerBase::IsMethodInBPFrame(method)) {
                S_LOST_SAMPLES++;
                return;
            } else {
                is_frame_boundary = false;
            }
        }
    } else if (is_compiled) {
        auto signal_context = SignalContext(ptr);
        auto pc = signal_context.GetPC();
        auto fp = signal_context.GetFP();
        bool pc_in_compiled = InAllocatedCodeRange(pc);
        CFrame cframe(frame);
        bool is_native = cframe.IsNative();
        if (!is_native && fp == nullptr) {
            sample.stack_info.managed_stack[stack_counter].panda_file_ptr = helpers::ToUnderlying(FrameKind::BRIDGE);
            sample.stack_info.managed_stack[stack_counter].file_id = helpers::ToUnderlying(FrameKind::BRIDGE);
            ++stack_counter;

            // fp is not set yet, so cframe not finished, currently in bridge, previous frame iframe
            is_compiled = false;
        } else if (!is_native && fp != nullptr) {
            auto pf_id = reinterpret_cast<uintptr_t>(frame->GetMethod()->GetPandaFile());
            if (pc_in_compiled) {
                // Currently in compiled method so get it from fp
                frame = reinterpret_cast<Frame *>(fp);
            } else if (!pc_in_compiled && pfs_queue.FindValue(pf_id)) {
                sample.stack_info.managed_stack[stack_counter].panda_file_ptr =
                    helpers::ToUnderlying(FrameKind::BRIDGE);
                sample.stack_info.managed_stack[stack_counter].file_id = helpers::ToUnderlying(FrameKind::BRIDGE);
                ++stack_counter;

                // pc not in jitted code, so fp is not up-to-date, currently not in cfame
                is_compiled = false;
            }
        }
    }

    auto stack_walker = StackWalkerBase(frame, is_compiled);

    while (stack_walker.HasFrame()) {
        auto method = stack_walker.GetMethod();

        if (method == nullptr || IsInvalidPointer(reinterpret_cast<uintptr_t>(method))) {
            S_LOST_SAMPLES++;
            S_LOST_INVALID_SAMPLES++;
            return;
        }

        auto pf = method->GetPandaFile();
        auto pf_id = reinterpret_cast<uintptr_t>(pf);
        if (!pfs_queue.FindValue(pf_id)) {
            S_LOST_SAMPLES++;
            S_LOST_NOT_FIND_SAMPLES++;
            return;
        }

        sample.stack_info.managed_stack[stack_counter].panda_file_ptr = pf_id;
        sample.stack_info.managed_stack[stack_counter].file_id = method->GetFileId().GetOffset();

        ++stack_counter;
        stack_walker.NextFrame();

        if (stack_counter == SampleInfo::StackInfo::MAX_STACK_DEPTH) {
            // According to the limitations we should drop all frames that is higher than MAX_STACK_DEPTH
            break;
        }
    }
    if (stack_counter == 0) {
        return;
    }
    sample.stack_info.managed_stack_size = stack_counter;
    sample.thread_info.thread_id = os::thread::GetCurrentThreadId();

    const ThreadCommunicator &communicator = Sampler::GetSampleCommunicator();
    communicator.SendSample(sample);
}

void Sampler::SamplerThreadEntry()
{
    struct sigaction action {};
    action.sa_sigaction = &SigProfSamplingProfilerHandler;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    // Clear signal set
    sigemptyset(&action.sa_mask);
    // Ignore incoming sigprof if handler isn't completed
    sigaddset(&action.sa_mask, SIGPROF);

    struct sigaction old_action {};

    if (sigaction(SIGPROF, &action, &old_action) == -1) {
        LOG(FATAL, PROFILER) << "Sigaction failed, can't start profiling";
        UNREACHABLE();
    }

    // We keep handler assigned to SigProfSamplingProfilerHandler after sampling end because
    // otherwice deadlock can happen if signal will be slow and reach thread after handler resignation
    if (old_action.sa_sigaction != nullptr && old_action.sa_sigaction != SigProfSamplingProfilerHandler) {
        LOG(FATAL, PROFILER) << "SIGPROF signal handler was overriden in sampling profiler";
        UNREACHABLE();
    }
    ++S_CURRENT_HANDLERS_COUNTER;

    auto pid = getpid();
    // Atomic with relaxed order reason: data race with is_active_
    while (is_active_.load(std::memory_order_relaxed)) {
        {
            os::memory::LockHolder holder(managed_threads_lock_);
            for (const auto &thread_id : managed_threads_) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                if (syscall(SYS_tgkill, pid, thread_id, SIGPROF) != 0) {
                    LOG(ERROR, PROFILER) << "Can't send signal to thread";
                }
            }
        }
        os::thread::NativeSleepUS(sample_interval_);
    }

    // Sending last sample on finish to avoid of deadlock in listener
    SampleInfo last_sample;
    last_sample.stack_info.managed_stack_size = 0;
    communicator_.SendSample(last_sample);

    --S_CURRENT_HANDLERS_COUNTER;

    const unsigned int time_to_sleep_ms = 100;
    do {
        os::thread::NativeSleep(time_to_sleep_ms);
    } while (S_CURRENT_HANDLERS_COUNTER != 0);
}

// Passing std:string copy instead of reference, 'cause another thread owns this object
// NOLINTNEXTLINE(performance-unnecessary-value-param)
void Sampler::ListenerThreadEntry(std::string output_file)
{
    auto writer_ptr = std::make_unique<StreamWriter>(output_file.c_str());
    // Writing panda files that were loaded before sampler was created
    WriteLoadedPandaFiles(writer_ptr.get());

    SampleInfo buffer_sample;
    // Atomic with relaxed order reason: data race with is_active_
    while (is_active_.load(std::memory_order_relaxed)) {
        WriteLoadedPandaFiles(writer_ptr.get());
        communicator_.ReadSample(&buffer_sample);
        if (LIKELY(buffer_sample.stack_info.managed_stack_size != 0)) {
            writer_ptr->WriteSample(buffer_sample);
        }
    }
    // Writing all remaining samples
    while (!communicator_.IsPipeEmpty()) {
        WriteLoadedPandaFiles(writer_ptr.get());
        communicator_.ReadSample(&buffer_sample);
        if (LIKELY(buffer_sample.stack_info.managed_stack_size != 0)) {
            writer_ptr->WriteSample(buffer_sample);
        }
    }
}

}  // namespace panda::tooling::sampler
