/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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

#include "runtime/tooling/tools.h"

#include "runtime/deoptimization.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/interpreter/interpreter_impl.h"
#include "runtime/tooling/coverage_listener.h"
#include "runtime/tooling/sampler/sampling_profiler.h"

namespace ark::tooling {

extern "C" PANDA_PUBLIC_API int StartSamplingProfiler(const char *asptFilename, int interva)
{
    auto *runtime = Runtime::GetCurrent();
    if (runtime == nullptr) {
        LOG(DEBUG, PROFILER) << "That runtime is not created.";
        return 1;
    }
    return static_cast<int>(
        !runtime->GetTools().StartSamplingProfiler(std::make_unique<sampler::FileStreamWriter>(asptFilename), interva));
}

extern "C" PANDA_PUBLIC_API void StopSamplingProfiler()
{
    auto *runtime = Runtime::GetCurrent();
    if (runtime == nullptr) {
        LOG(DEBUG, PROFILER) << "That runtime is not created.";
        return;
    }
    runtime->GetTools().StopSamplingProfiler();
}

sampler::Sampler *Tools::GetSamplingProfiler()
{
    // Singleton instance
    return sampler_;
}

void Tools::CreateSamplingProfiler()
{
    ASSERT(sampler_ == nullptr);
    sampler_ = sampler::Sampler::Create();

    const char *samplerSegvOption = std::getenv("ARK_SAMPLER_DISABLE_SEGV_HANDLER");
    if (samplerSegvOption != nullptr) {
        std::string_view option = samplerSegvOption;
        if (option == "1" || option == "true" || option == "ON") {
            // SEGV handler for sampler is enable by default
            sampler_->SetSegvHandlerStatus(false);
        }
    }
}

bool Tools::StartSamplingProfiler(std::unique_ptr<sampler::StreamWriter> streamWriter, uint32_t interval)
{
    ASSERT(sampler_ != nullptr);
    sampler_->SetSampleInterval(interval);
    return sampler_->Start(std::move(streamWriter));
}

void Tools::StopSamplingProfiler()
{
    ASSERT(sampler_ != nullptr);
    sampler_->Stop();
}

void Tools::DestroySamplingProfiler()
{
    ASSERT(sampler_ != nullptr);
    sampler::Sampler::Destroy(sampler_);
    sampler_ = nullptr;
}

bool Tools::IsSamplingProfilerCreate()
{
    return sampler_ != nullptr;
}

void Tools::CreateCoverageListener(const std::string &filePath)
{
    coverageListener_ = new tooling::CoverageListener(filePath);
}

CoverageListener *Tools::GetCoverageListener()
{
    return coverageListener_;
}

void Tools::DestroyCoverageListener()
{
    if (coverageListener_ == nullptr) {
        return;
    }
    delete coverageListener_;
    coverageListener_ = nullptr;
}

bool Tools::IsDebugSessionAttachAllowed() const
{
    return Runtime::GetCurrent()->GetOptions().IsDebuggerAttachAllowed();
}

static void DeoptimizeAllCompiledMethods(Runtime *runtime, ManagedThread *thread)
{
    PandaSet<Method *> compiledMethods;
    runtime->GetClassLinker()->EnumerateClasses([&](Class *klass) {
        ASSERT(klass != nullptr);
        for (auto &method : klass->GetMethodsWithCopied()) {
            // Note special handling for proxy methods
            if (method.HasCompiledCode() && !method.IsIntrinsic() && !method.IsNative() && !method.IsProxy()) {
                compiledMethods.insert(&method);
            }
        }
        return true;
    });
    if (!compiledMethods.empty()) {
        ScopedManagedCodeThread smct(thread);
        InvalidateCompiledEntryPoint(compiledMethods, false);
    }
}

DebugSessionAttachErrorCode Tools::AttachDebugSession()
{
    auto *thread = ManagedThread::GetCurrent();
    ASSERT(thread != nullptr);
    ASSERT(!ManagedThread::IsManagedScope());

    auto *runtime = Runtime::GetCurrent();
    if (UNLIKELY(runtime->IsDebugMode())) {
        return DebugSessionAttachErrorCode::ALREADY_ATTACHED;
    }
    if (UNLIKELY(!IsDebugSessionAttachAllowed())) {
        return DebugSessionAttachErrorCode::NOT_ALLOWED;
    }

    // NOTE(dslynko): support JIT: suspend and disable it during attach
    if (UNLIKELY(runtime->IsJitEnabled())) {
        return DebugSessionAttachErrorCode::NOT_ALLOWED;
    }

    // 1. Enable debug mode globally.
    runtime->SetDebugMode(true);

    // 2. Clear all JIT and AOT compiled methods and trigger deoptimization
    // for compiled frames corresponding to those methods.
    if (LIKELY(runtime->GetClassLinker()->GetAotManager()->HasAotFiles())) {
        DeoptimizeAllCompiledMethods(runtime, thread);
    }

    // 3. Create the debug session.
    const auto session = runtime->StartDebugSession();
    if (UNLIKELY(!session)) {
        return DebugSessionAttachErrorCode::INVALID;
    }

    // 4. Suspend all threads and switch each to use debug dispatch table.
    auto *vm = thread->GetVM();
    ScopedSuspendAllThreads ssat(vm->GetRendezvous());
    vm->GetThreadManager()->EnumerateThreads([](ManagedThread *thread) {
        interpreter::SwitchToDebugMode(thread);
        return true;
    });

    return DebugSessionAttachErrorCode::OK;
}

}  // namespace ark::tooling
