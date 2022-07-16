/*
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

#include "libpandabase/utils/logger.h"

#include "verification/jobs/thread_pool.h"

namespace panda::verifier {
bool Processor::Init()
{
    PandaString thread_name {"verifier#"};
    thread_name += NumToStr(types_.GetThreadNum());
    panda::os::thread::SetThreadName(panda::os::thread::GetNativeHandle(), thread_name.c_str());
    LOG(DEBUG, VERIFIER) << "Thread ID " << panda::os::thread::GetCurrentThreadId() << " is named " << thread_name;
    return true;
}

bool Processor::Process(Task task)
{
    if (task.IsEmpty()) {
        // The return value of this method is currently ignored in panda::ThreadPool::WorkerEntry.
        // If this changes, all return statements should be reviewed.
        return true;
    }

    auto &method = task.GetMethod();
    if (method.IsVerified()) {
        LOG(DEBUG, VERIFIER) << "Method '" << method.GetFullName(true) << "' is already verified";
        return true;
    }

    auto method_name = method.GetFullName();

    auto opt_cache = ThreadPool::GetCache();
    if (!opt_cache.HasRef()) {
        LOG(INFO, VERIFIER) << "Attempt to verify " << method.GetFullName(true)
                            << "after the thread pool started shutdown, ignoring";
        return true;
    }
    auto &cache = opt_cache.Get();
    auto opt_cached_method = cache.FastAPI().GetMethod(method.GetClass()->GetSourceLang(), method.GetUniqId(), true);
    if (!opt_cached_method.HasRef()) {
        LOG(ERROR, VERIFIER) << "Method '" << method.GetFullName(true)
                             << "' not found in verifier cache, cannot create a job.";
        method.SetVerified(false);
        return true;
    }
    const auto &method_options = Runtime::GetCurrent()->GetVerificationOptions().Debug.GetMethodOptions();
    const auto &verif_method_options = method_options[method_name];
    LOG(DEBUG, VERIFIER) << "Verification config for '" << method_name << "': " << verif_method_options.GetName();
    LOG(INFO, VERIFIER) << "Verifier thread #" << types_.GetThreadNum() << " started verification of method '"
                        << method.GetFullName(true) << "'";

    Job job {method, opt_cached_method.Get(), verif_method_options};
    bool result = job.DoChecks(cache, types_);
    method.SetVerified(result);
    LOG(INFO, VERIFIER) << "Verification result for method " << method.GetFullName(true)
                        << (result ? ": OK" : ": FAIL");

    return true;
}

bool Processor::Destroy()
{
    bool show_subtyping =
        Runtime::GetCurrentSync([](auto &instance) { return instance.GetVerificationOptions().Debug.Show.TypeSystem; });
    if (show_subtyping) {
        LOG(DEBUG, VERIFIER) << "Typesystem of verifier thread #" << types_.GetThreadNum();
        types_.DisplayTypeSystem([](const auto &str) { LOG(DEBUG, VERIFIER) << str; });
    }
    return true;
}

bool ThreadPool::Enqueue(Method *method)
{
    Data *data = GetData();
    if (data == nullptr) {
        LOG(DEBUG, VERIFIER) << "Attempted to enqueue a method after shutdown";
        return false;
    }

    bool enqueued = data->thread_pool.PutTask(Task {*method});
    return enqueued;
}

void ThreadPool::Initialize(mem::InternalAllocatorPtr allocator, size_t num_threads)
{
    ASSERT(allocator != nullptr);

    // Atomic with release order reason: data race with shutdown_ with dependecies on writes before the store which
    // should become visible acquire
    shutdown_.store(false, std::memory_order_release);
    if (GetData() != nullptr) {
        return;
    }

    // Atomic with release order reason: data race with next_thread_num_ with dependecies on writes before the store
    // which should become visible acquire
    Processor::next_thread_num_.store(0, std::memory_order_release);
    allocator_ = allocator;
    Data *data = allocator->New<Data>(allocator, num_threads);
    if (data == nullptr) {
        LOG(ERROR, VERIFIER) << "Insufficient memory to initialize verifier thread pool";
        return;
    }
    Data *expected = nullptr;
    if (data_.compare_exchange_strong(expected, data, std::memory_order_seq_cst)) {
        LOG(INFO, VERIFIER) << "Initialized verifier thread pool";
    } else {
        // already initialized by another thread
        allocator->Delete(data);
    }
}

void ThreadPool::Destroy()
{
    // Atomic with seq_cst order reason: initial adding annotations for memory order
    if (shutdown_.exchange(true, std::memory_order_seq_cst)) {
        // the value was true already and shutdown has started
        return;
    }

    Data *data = GetData(true);
    if (data == nullptr) {
        return;
    }

    data->thread_pool.Shutdown(true);

    // Atomic with seq_cst order reason: initial adding annotations for memory order
    Data *data1 = data_.exchange(nullptr, std::memory_order_seq_cst);
    if (data1 != nullptr) {
        // this should happen in at most one thread even if Destroy is called in many
        allocator_->Delete(data1);
        LOG(INFO, VERIFIER) << "Destroyed verifier thread pool";
    }
}

void ThreadPool::SignalMethodVerified()
{
    Data *data = GetData();
    if (data != nullptr) {
        panda::os::memory::LockHolder lck {data->lock};
        data->cond_var.SignalAll();
    }
}

}  // namespace panda::verifier
